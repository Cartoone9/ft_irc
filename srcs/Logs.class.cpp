#include "Logs.class.hpp"
#include <unistd.h>
#include <sys/socket.h>

Logs::Logs(time_t start)
	: _start_time(start)
{

}

static std::string badEndlinesInRed(const std::string& text)
{
	std::string	result;
	for (std::string::size_type i = 0; i < text.length(); ++i)
	{
		if (text[i] == '\r') {
			if (i + 1 < text.length() && text[i + 1] == '\n')
			{
				result += "\n";
				++i;
			}
			else
				result += RED "\\r" RESET;
		}
		else if (text[i] == '\n')
			result += RED "\\n" RESET;
		else
			result += text[i];
	}
	return (result);
}

void	Logs::logsConnect(int new_s_fd, int n_fds)
{
	displayElapsedTime(_start_time);

	std::cout << GREEN "New connection" RESET << " with client (" << new_s_fd << ")" << std::endl;
	std::cout << "Total connected: " << n_fds << "\n" << std::endl;
}

void	Logs::logsDisconnect(int s_fd, int n_fds)
{
	displayElapsedTime(_start_time);

	std::cout << RED "Disconnected" RESET << " client (" << s_fd << ")" << std::endl;
	std::cout << "Total connected: " << n_fds - 1 << "\n" << std::endl;
}

void	Logs::logsBuffer(int s_fd, std::string &buffer, bool which)
{
	displayElapsedTime(_start_time);

	if (which) // buffer_in
		std::cout << TEAL "Buffer_in" RESET;
	else // buffer_out
		std::cout << MAGENTA "Buffer_out" RESET;

	std::cout << " for client (" << s_fd << "):" << std::endl;
	std::cout << badEndlinesInRed(buffer) << std::endl; 
}

void	Logs::logsBufferOverLimit(int s_fd)
{
	displayElapsedTime(_start_time);

	std::cout << "Client (" << s_fd << ") incoming data is " << ORANGE "over 512 bytes" RESET << std::endl;
	std::cout << "It will be disconnected to avoid flooding\n" << std::endl;
}

void	Logs::logsEnd(int s_fd, bool which)
{
	displayElapsedTime(_start_time);

	if (which) // client
		std::cout << "Closing connection with client (" << s_fd << ")" << std::endl;
	else // socket
		std::cout << "Closing the listening socket" << std::endl;
}
void	Logs::logsError(int fd)
{
	int err = 0;
	socklen_t len = sizeof(err);
	getsockopt(fd, SOL_SOCKET, SO_ERROR, &err, &len);

	displayElapsedTime(_start_time);
	std::cout << RED "Poll error: " << err << "" RESET << std::endl;
}
