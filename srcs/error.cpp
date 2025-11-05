#include <cstdio> // perror()
#include <iostream>

#include "dictionary.hpp"

int error(const std::string &msg)
{
	std::cerr << "ircserv: " << msg << std::endl;
	return (ERROR);
}

int spe_error(const char *msg)
{
	std::cerr << "ircserv: ";
	perror(msg);
	return (ERROR);
}
