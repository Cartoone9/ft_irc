#ifndef CONNECTION_CLASS_HPP
#define CONNECTION_CLASS_HPP

#include <cerrno>       // errno, EWOULDBLOCK
#include <iostream>     // std::cout, std::endl
#include <map>          // std::map
#include <set>          // std::set
#include <string>       // std::string

#include <poll.h>       // poll(), struct pollfd, POLLIN, POLLOUT, POLLERR, POLLHUP
#include <sys/socket.h> // socket(), bind(), listen(), accept(), send(), recv()
#include <sys/time.h>   // timeval (optional, used for timeouts)
#include <sys/types.h>  // socket-related types like socklen_t
#include <unistd.h>     // close(), read(), write()

#include "colors.hpp"     // UNDERLINE, RESET
#include "Logs.class.hpp"
#include "dictionary.hpp"
#include "handlers.hpp"
#include "State.struct.hpp"
#include "utils.hpp"      // spe_error()

class Connection
{
private:
	struct pollfd				_pfd[MAX_CLIENT];
	int							_n_fds;
	std::map<int, std::string>	buffer_in, buffer_out;
	State						&state;
	message_handler_fn			*message_handler;
	Logs						logs;
	std::set<int>				pending_disconnect_fds;

	Connection();

	void	initPollfd(int listen_s_fd);
	int		acceptNewClient();
	int		sendData(int &index);
	int		receiveData(int &index);
	int		disconnectClient(int &index);
	int		shrinkArray(int &index);
	void	closeAll();
	void	fillRegisterOut(Responses &);
	void	onRead(int fd);
	void	onDisconnect(int fd, int index);
	void	clearBuffers(int fd);
	void	clearEvents(int fd);
	int		findIndexByFd(int fd);

public:
	Connection(State &state, message_handler_fn *message_handler);

	int		pollLoop(int listen_s_fd);
};

#endif // #ifndef CONNECTION_CLASS_HPP
