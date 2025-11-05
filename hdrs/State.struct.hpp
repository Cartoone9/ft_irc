#ifndef STATE_STRUCT_HPP
#define STATE_STRUCT_HPP

#include <map>
#include <vector>
#include "Client.struct.hpp"
#include "Channel.struct.hpp"
#include <ctime>

struct State
{
	std::map<int, Client> clients;
	std::map<std::string, Channel> channels;
	std::string password, motd, oper_name, oper_pass;
	time_t start_time;

	// should be used by QUIT and PART handlers
	void removeClient(int fd);

	// helper functions for broadcasting
	std::vector<int> clientsInChannelsWith(int fd) const;

	// helper function for targetted commands
	// returns their fd, -1 if nick not found
	int findClientByNick(const std::string &) const;

	std::string getNamesInChannel(const std::string &channel_name);
};

#endif // #ifndef STATE_STRUCT_HPP
