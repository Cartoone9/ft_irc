#include <cerrno>       // errno
#include <fcntl.h>      // fcntl(), F_GETFL, F_SETFL, O_NONBLOCK
#include <netinet/in.h> // sockaddr_in, INADDR_ANY, htons(), htonl()
#include <sys/socket.h> // socket(), setsockopt(), bind(), listen()
#include <unistd.h>     // close()

#include "dictionary.hpp" // OK, ERROR, L_QUEUE
#include "utils.hpp"      // spe_error()

static int	setNonBlocking(int s_fd)
{
	int flags = fcntl(s_fd, F_GETFL, 0);

	if (flags == ERROR)
		return (ERROR);

	if (fcntl(s_fd, F_SETFL, flags | O_NONBLOCK) == ERROR)
		return (ERROR);

	return (OK);
}

int	initListeningSocket(int port)
{
	struct sockaddr_in	server_addr;
	int					s_fd;

	server_addr.sin_family = AF_INET; // IPv4
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY); // Bind to all local network interfaces (0.0.0.0) 
	server_addr.sin_port = htons(port);

	// Create the listening socket
	// AF_INET == IPv4
	// SOCK_STREAM == TCP
	s_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (s_fd == ERROR)
		return (spe_error("socket"), ERROR);

	// Set the socket to be able to reuse a port locked in 'time wait'
	// useful in case of a crash or if we restart the server
	int	opt = 1;
	if (setsockopt(s_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == ERROR)
		return (close(s_fd), spe_error("setsockopt"), ERROR);

	// Use fcntl() to set the socket_fd as non blocking
	// meaning it won't wait for a syscall to return if it isn't ready 
	if (setNonBlocking(s_fd) == ERROR)
		return (close(s_fd), spe_error("fcntl"), ERROR);

	// Bind and "reserve" a local IP and port for this listening socket
	if (bind(s_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == ERROR)
		return (close(s_fd), spe_error("bind"), ERROR);

	// L_QUEUE == max clients queue waiting to connect to the listening socket
	if (listen(s_fd, L_QUEUE) == ERROR)
		return (close(s_fd), spe_error("listen"), ERROR);

	return (s_fd);
}
