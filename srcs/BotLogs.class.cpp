#include "BotLogs.class.hpp"

BotLogs::BotLogs(time_t start)
	: _start_time(start)
{

}

void	BotLogs::botLogsBuffer(const std::string &buffer, bool which)
{
	displayElapsedTime(_start_time);

	if (which) // buffer_in
		std::cout << TEAL "Buffer_in" RESET;
	else // buffer_out
		std::cout << MAGENTA "Buffer_out" RESET;
	
	std::cout << " for " << ORANGE "BOT" RESET << ":" << std::endl;
	std::cout << buffer << std::endl; 
}
