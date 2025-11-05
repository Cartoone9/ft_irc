#ifndef UTILS_HPP
#define UTILS_HPP

#include <iostream>
#include <sstream>

#include "colors.hpp"
#include "State.struct.hpp"

// ui
void		displayBanner(int port, State &state);
void		displayFullTime(time_t time);
void		displayElapsedTime(time_t start);
std::string	timeToStr(time_t start);
std::string	dateToStr(time_t start);

// socket
int		initListeningSocket(int port);

// signal
bool	isStopped();

// error
int		error(const std::string &msg);
int		spe_error(const char *msg);

#endif // #ifndef UTILS_HPP
