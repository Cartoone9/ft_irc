#include <csignal>
#include <cstdlib>   // strtol()
#include <iostream>

#include <fcntl.h>   // fcntl()
#include <poll.h>	// poll()
#include <unistd.h> // write()

#include "colors.hpp"
#include "dictionary.hpp"
#include "Connection.class.hpp"
#include "handlers.hpp"
#include "utils.hpp"

volatile sig_atomic_t g_stop = false;

// --- Helper Functions Declaration ---
int tests(); // tests.cpp

static int			usage();
static int			strToPort(const std::string &str);
static std::string	getMOTD(int argc, char **argv);
static void			handleSignal(int _);

// --- Main File Functions ---
int main(int argc, char **argv)
{
	if (argc == 2 && std::string("--test") == argv[1])
		return tests();

	if (argc != 3 && argc != 4)
		return usage();

	int port = strToPort(argv[1]);
	if (port == ERROR)
		return (NOK);

	std::string password = argv[2];
	if (password.empty())
		return (error("password can't be empty."));

	std::signal(SIGINT, handleSignal);
	// std::signal(SIGQUIT, handleSignal);
	// std::signal(SIGTERM, handleSignal);

	// Prepare application state
	State state = State();
	state.password = password;
	state.motd = getMOTD(argc, argv);
	state.start_time = time(0);
	state.oper_name = OPER_NAME;
	state.oper_pass = OPER_PASS;
	state.clients[BOT_ID] = createBotClient();

	// Setup message routing
	Connection connection(state,
			password == "--test" ? parrot : botRouter);

	// Setup the listening socket
	int	listen_s_fd = initListeningSocket(port);
	if (listen_s_fd == ERROR)
		return (NOK);

	displayBanner(port, state);

	// Start the poll() loop
	if (connection.pollLoop(listen_s_fd) == ERROR)
		return (NOK);

	return (OK);
}

bool isStopped()
{
	if (!g_stop)
		return false;
	std::cout << "\r  \r" REVERSED " stopping signal received " RESET << std::endl;
	return true;
}

// --- Helper Functions ---
static int usage()
{
	std::cout << "Usage: ./ircserv <port> <password> [MOTD]" << std::endl;
	return (OK);
}

static int strToPort(const std::string &str)
{
	// valid ports: 0 - 65535

	if (str.empty())
		return (error("empty port"), ERROR);

	size_t str_len = str.size();

	if (str_len > 5)
		return (error("invalid port"), ERROR);

	for (size_t i = 0; i < str_len; i++)
	{
		if (!std::isdigit(str[i]))
			return (error("invalid port"), ERROR);
	}

	long long port_ret = strtol(str.c_str(), NULL, 10);

	if (port_ret > 65535)
		return (error("invalid port"), ERROR);
	else
		return (static_cast<int>(port_ret));
}

static std::string	getMOTD(int argc, char **argv)
{
	if (argc == 3)
		return ("");
	else
		return (argv[3]);
}

static void handleSignal(int _)
{
	(void)_;
	g_stop = true;

	const char msg[] = "\r  \r" REVERSED " stopping signal received " RESET;
	write(1, msg, sizeof(msg) - 1);
}
