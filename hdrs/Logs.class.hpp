#ifndef LOGS_CLASS_HPP
#define LOGS_CLASS_HPP

#include <ctime>
#include <iostream>

#include "colors.hpp"
#include "utils.hpp"

class Logs
{
	public:
		Logs(time_t start);

		void	logsConnect(int new_s_fd, int n_fds);
		void	logsDisconnect(int s_fd, int n_fds);
		void	logsBuffer(int s_fd, std::string &buffer, bool which);
		void	logsBufferOverLimit(int s_fd);
		void	logsEnd(int s_fd, bool which);
		void	logsError(int s_fd);

	private:
		Logs();

		time_t	_start_time;
};

#endif // #ifndef LOGS_CLASS_HPP
