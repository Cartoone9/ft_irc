#include "Client.struct.hpp"

// full hostmask for message sources
std::string Client::hostmask() const
{
	if (nick.empty())
		return "*";
	return nick + "!" + username + "@0.0.0.0";
}

// just the nick or * for numeric replies
Client::operator std::string() const
{
	if (nick.empty())
		return "*";
	return nick;
}

bool Client::isOp() const
{
	return modes.count('o') == 1;
}
