#include "State.struct.hpp"
#include <vector>
#include <set>
#include <algorithm>

void State::removeClient(int fd)
{
	std::vector<std::string> emptyChannels;
	std::map<std::string, Channel>::iterator channel;
	for (channel = channels.begin(); channel != channels.end(); ++channel)
	{
		channel->second.client_ids.erase(fd);
		channel->second.op_ids.erase(fd);
		channel->second.invited_ids.erase(fd);
		if (channel->second.client_ids.empty())
			emptyChannels.push_back(channel->first);
	}
	clients.erase(fd);
	std::vector<std::string>::iterator name;
	for (name = emptyChannels.begin(); name != emptyChannels.end(); ++name)
	{
		channels.erase(*name);
	}
}

std::vector<int> State::clientsInChannelsWith(int fd) const
{
	std::set<int> clients;
	std::map<std::string, Channel>::const_iterator it;
	for (it = channels.begin(); it != channels.end(); ++it)
	{
		if (it->second.client_ids.count(fd) != 0)
		{
			clients.insert(it->second.client_ids.begin(), it->second.client_ids.end());
		}
	}
	clients.erase(fd);
	return std::vector<int>(clients.begin(), clients.end());
}

int State::findClientByNick(const std::string &nick) const
{
	for (std::map<int, Client>::const_iterator it = clients.begin();
		 it != clients.end(); ++it)
	{
		if (it->second.nick == nick)
			return it->first;
	}
	return (-1);
}

// for NAME command, it gives a single string with nicks
// channel op are marked with @, for example "tom,@john,ric,clank"
std::string State::getNamesInChannel(const std::string &channel_name)
{
	std::string names;

	if (!channels.count(channel_name))
		return names;
	Channel &channel = channels[channel_name];
	std::set<int>::const_iterator it;

	// operators first
	for (it = channel.client_ids.begin(); it != channel.client_ids.end(); ++it)
	{
		if (channel.op_ids.count(*it))
		{
			names += "@" + clients[*it].nick + " ";
		}
	}

	// then regular users
	for (it = channel.client_ids.begin(); it != channel.client_ids.end(); ++it)
	{
		if (!channel.op_ids.count(*it))
			names += clients[*it].nick + " ";
	}

	return names;
}
