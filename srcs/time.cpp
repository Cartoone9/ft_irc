#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>

#include "colors.hpp"

void	displayFullTime(time_t timestamp)
{
	// Convert to local time
	tm* localTime = localtime(&timestamp);
	
	std::cout << UNDERLINE "Starting time:" RESET << std::endl;

	// Date
	std::cout << "(" << (localTime->tm_year + 1900) << "-"
		<< (localTime->tm_mon + 1) << "-"
		<< localTime->tm_mday << ") ";
	// Time
	std::cout << std::setfill('0') 
		<< std::setw(2) << localTime->tm_hour << ":"
		<< std::setw(2) << localTime->tm_min << ":"
		<< std::setw(2) << localTime->tm_sec << std::endl;

	std::cout << std::endl;
}

void	displayElapsedTime(time_t start)
{
	time_t elapsed = time(0) - start;
	int hours = elapsed / 3600;
	int minutes = (elapsed % 3600) / 60;
	int seconds = elapsed % 60;

	// Time
	std::cout << std::setfill('0') << "["
		<< std::setw(2) << hours << ":"
		<< std::setw(2) << minutes << ":"
		<< std::setw(2) << seconds << "] ";
}

std::string	timeToStr(time_t start)
{
	std::ostringstream oss;

	// Convert to local time
	tm* localTime = localtime(&start);

	// Time
	oss << std::setfill('0') 
		<< std::setw(2) << localTime->tm_hour << ":"
		<< std::setw(2) << localTime->tm_min << ":"
		<< std::setw(2) << localTime->tm_sec;

	return (oss.str());
}

std::string	dateToStr(time_t start)
{
	std::ostringstream oss;

	// Convert to local time
	tm* localTime = localtime(&start);
	//
	// Date
	oss << "(" << (localTime->tm_year + 1900) << "-"
		<< (localTime->tm_mon + 1) << "-"
		<< localTime->tm_mday << ") ";

	return (oss.str());
}
