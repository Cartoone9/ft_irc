#include "Connection.class.hpp"

Connection::Connection(State &state, message_handler_fn *message_handler)
	: state(state), message_handler(message_handler), logs(state.start_time)
{

}

void	Connection::initPollfd(int listen_s_fd)
{
	_n_fds = 1;

	// Add the listening s_fd to the pollfd struct array
	_pfd[0].fd = listen_s_fd;
	_pfd[0].events = POLLIN;
	_pfd[0].revents = 0;

	for (size_t i = 1; i < MAX_CLIENT; ++i)
	{
		_pfd[i].fd = -1;
		_pfd[i].events = 0;
		_pfd[i].revents = 0;
	}
}

int	Connection::pollLoop(int listen_s_fd)
{
	int	poll_ret;

	initPollfd(listen_s_fd);

	while (!isStopped())
	{
		poll_ret = poll(_pfd, _n_fds, 100);
		if (poll_ret == ERROR)
		{
			closeAll();
			if (errno == EINTR)
				return (OK);
			else
				return (spe_error("poll"), ERROR);
		}

		else if (poll_ret > 0)
		{
			// event found
			for (int index = 0; index < _n_fds; index++)
			{
				if (_pfd[index].revents)
				{
					if (_pfd[index].revents & POLLHUP) // client disconnects
					{
						if (disconnectClient(index) == ERROR)
							return (closeAll(), ERROR);
						continue;
					}
					if (_pfd[index].revents & POLLERR) // error, show reason
					{
						logs.logsError(_pfd[index].fd);
						if (disconnectClient(index) == ERROR)
							return (closeAll(), ERROR);
						continue;
					}
					if (_pfd[index].revents & POLLIN) // socket has incoming data for buffer
					{
						_pfd[index].revents &= ~POLLIN;
						// listening socket
						if (index == 0)
						{
							if (acceptNewClient() == ERROR)
								return (closeAll(), ERROR);
						}
						// client socket
						else if (receiveData(index) == ERROR)
							return (closeAll(), ERROR);
					}
					if (_pfd[index].revents & POLLOUT) // socket buffer ready for outgoing data
					{
						_pfd[index].revents &= ~POLLOUT;
						if (sendData(index) == ERROR)
							return (closeAll(), ERROR);
					}
					poll_ret--;
					if (poll_ret == 0)
						break;
				}
			}
		}
	}

	closeAll();
	return (OK);
}

int	Connection::acceptNewClient()
{
	int	listen_s_fd = _pfd[0].fd;
	int	new_s_fd = accept(listen_s_fd, NULL, NULL);
	if (new_s_fd == ERROR)
	{
		if (errno != EWOULDBLOCK)
			return (spe_error("accept"), ERROR);
		else
		{
			errno = 0;
			return (OK);
		}
	}

	if (_n_fds >= MAX_CLIENT)
	{
		close(new_s_fd);
		return (error("too many clients"), OK);
	}

	logs.logsConnect(new_s_fd, _n_fds);

	_pfd[_n_fds].fd = new_s_fd;
	_pfd[_n_fds].events = POLLIN;
	_n_fds++;

	return (OK);
}

int Connection::sendData(int &index)
{
	int			s_fd = _pfd[index].fd;
	std::string	&buffer = buffer_out[s_fd];
	int			b_send;

	if (buffer.empty())
	{
		_pfd[index].events &= ~POLLOUT;
		return (NOK);
	}

	logs.logsBuffer(s_fd, buffer_out[s_fd], false);

	b_send = send(s_fd, buffer.c_str(), buffer.size(), 0);
	if (b_send <= 0)
	{
		if (b_send == 0)
		{
			// client disconnected
			if (disconnectClient(index) == ERROR)
				return (closeAll(), ERROR);
			return (OK);
		}
		if (errno != EWOULDBLOCK)
			return (spe_error("send"), ERROR);
		else
		{
			errno = 0;
			return (OK);
		}
	}

	buffer.erase(0, b_send);

	if (buffer.empty())
	{
		_pfd[index].events &= ~POLLOUT;
		if (pending_disconnect_fds.count(s_fd))
			if (disconnectClient(index) == ERROR)
				return (closeAll(), ERROR);
	}

	return (OK);
}

int Connection::receiveData(int &index)
{
	int		s_fd = _pfd[index].fd;
	char	buffer[513];
	int		b_read;

	b_read = recv(s_fd, buffer, sizeof(buffer), 0);
	if (b_read <= 0)
	{
		if (b_read == 0)
		{
			// client disconnected
			if (disconnectClient(index) == ERROR)
				return (closeAll(), ERROR);
			return (OK);
		}
		if (errno != EWOULDBLOCK)
			return (spe_error("recv"), ERROR);
		else
		{
			errno = 0;
			return (OK);
		}
	}
	else if (b_read == 513)
	{
		logs.logsBufferOverLimit(s_fd);

		// disconnect client to avoid flooding
		// does not send message to malfunctioning client
		if (disconnectClient(index) == ERROR)
			return (closeAll(), ERROR);

		return (OK);
	}

	buffer_in[s_fd].append(buffer, b_read);

	logs.logsBuffer(s_fd, buffer_in[s_fd], true);

	onRead(s_fd);

	return (OK);
}

int	Connection::disconnectClient(int &index)
{
	int	s_fd = _pfd[index].fd;

	// send disconnect message
	// & clean connection buffers
	onDisconnect(s_fd, index);

	// close s_fd
	if (close(s_fd) == ERROR)
		return (error("close"), ERROR);

	// shrink the array
	shrinkArray(index);

	logs.logsDisconnect(s_fd, _n_fds);

	return (OK);
}

int	Connection::shrinkArray(int &index)
{
	for (int i = index; i < _n_fds - 1; i++)
	{
		_pfd[i] = _pfd[i + 1];
	}

	index--;
	_n_fds--;

	_pfd[_n_fds].fd = -1; 
	_pfd[_n_fds].events = 0; 
	_pfd[_n_fds].revents = 0; 

	return (OK);
}

void	Connection::closeAll()
{
	// debug
	std::cout << UNDERLINE "\n\nClosing all connections:" RESET << std::endl;

	// client sockets
	for (size_t i = 1; i < MAX_CLIENT; i++)
	{
		if (_pfd[i].fd >= 0)
		{
			logs.logsEnd(_pfd[i].fd, true);

			close(_pfd[i].fd);
			_pfd[i].fd = -1;
		}
	}

	logs.logsEnd(0, false);

	// listening socket
	close(_pfd[0].fd);
	_pfd[0].fd = -1;
}

void Connection::onRead(int fd)
{
	std::string &buff = buffer_in.at(fd);
	while (buff.find("\r\n") != std::string::npos)
	{
		size_t size = buff.find("\r\n") + 2;
		Message in(fd, buff.substr(0, size));
		buff.erase(0, size);
		Responses output;
		message_handler(in, state, output);
		fillRegisterOut(output);
	}
}

void Connection::onDisconnect(int fd, int index)
{
	Message msg(fd, "QUIT :Disconnected");
	Responses output;
	message_handler(msg, state, output);
	fillRegisterOut(output);
	clearBuffers(fd);
	clearEvents(index);
}

void Connection::fillRegisterOut(Responses &r)
{
	for (Responses::iterator it = r.begin(); it != r.end(); ++it)
	{
		int fd = it->fd;
		if (fd < 0)
			continue;
		buffer_out[fd] += it->assemble();

		if (!buffer_out[fd].empty())
		{
			int index = findIndexByFd(fd);
			if (index != ERROR)
				_pfd[index].events |= POLLOUT;
		}

		// will disconnect after sending
		if (it->shouldDisconnect())
			pending_disconnect_fds.insert(it->fd);
	}
}

int Connection::findIndexByFd(int fd)
{
	for (int index = 0; index < _n_fds; index++)
	{
		if (_pfd[index].fd == fd)
			return (index);
	}
	return (ERROR);
}

void Connection::clearBuffers(int fd)
{
	buffer_in[fd].clear();
	buffer_out[fd].clear();
	pending_disconnect_fds.erase(fd);
}

void Connection::clearEvents(int index)
{
	_pfd[index].events = 0;
	_pfd[index].revents = 0;
}
