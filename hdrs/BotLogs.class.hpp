#ifndef BOTLOGS_CLASS_HPP
#define BOTLOGS_CLASS_HPP

#include <ctime>
#include <iostream>

#include "colors.hpp"
#include "utils.hpp"

class BotLogs
{
	public:
		BotLogs(time_t start);

		void	botLogsBuffer(const std::string &buffer, bool which);

	private:
		BotLogs();

		time_t	_start_time;
};

#endif // #ifndef BOTLOGS_CLASS_HPP
