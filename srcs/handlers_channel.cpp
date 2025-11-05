#include "colors.hpp"
#include "handlers.hpp"
#include "numerics.hpp"

static bool isValidChannelName(const std::string &name)
{
	if (name.size() < 2)
		return false;
	return name[0] == '#';
}

void joinHandler(const Message &m, State &s, Responses &r)
{
	Client &client = s.clients[m.fd];
	if (m.params.size() < 1)
		return r.push_back(Message(m.fd, ERR_NEEDMOREPARAMS, client, m.verb, RED "Not enough parameters" RESET));

	std::string channel_name = m.params.at(0);

	if (!isValidChannelName(channel_name))
		return r.push_back(Message(m.fd, ERR_NOSUCHCHANNEL, client, channel_name, RED "No such channel" RESET));

	// create if no channel
	if (s.channels.find(channel_name) == s.channels.end())
	{
		s.channels[channel_name].name = channel_name;
	}

	Channel &channel = s.channels[channel_name];

	if (channel.client_ids.find(m.fd) != channel.client_ids.end())
		return r.push_back(Message(m.fd, ERR_USERONCHANNEL, client, "JOIN", RED "Already on channel" RESET));

	// check channel mode +k
	if (!client.isOp() && channel.modes.count('k'))
	{
		if (m.params.size() < 2 || m.params.at(1) != channel.key)
			return r.push_back(Message(m.fd, ERR_BADCHANNELKEY, client, channel_name, RED "Cannot join channel (+k)" RESET));
	}

	// check channel mode +l
	if (!client.isOp() && channel.modes.count('l'))
	{
		if (channel.client_ids.size() >= channel.userlimit)
			return r.push_back(Message(m.fd, ERR_CHANNELISFULL, client, channel_name, RED "Cannot join channel (+l)" RESET));
	}

	// check channel mode +i
	if (!client.isOp() && channel.modes.count('i'))
	{
		if (!channel.invited_ids.count(m.fd))
			return r.push_back(Message(m.fd, ERR_INVITEONLYCHAN, client, channel_name, RED "Cannot join channel (+i)" RESET));
	}

	// add to channel
	if (channel.client_ids.empty())
		channel.op_ids.insert(m.fd);
	channel.client_ids.insert(m.fd);

	// remove from invited list, invites have one-use
	channel.invited_ids.erase(m.fd);

	// broadcast to everyone in the channel after joining
	Responses broadcast = Message(client.hostmask(), m.fd, "JOIN", channel_name)
		.repeat(channel.client_ids);
	r.insert(r.end(), broadcast.begin(), broadcast.end());

	// adds TOPIC and NAMES
	topicHandler(Message(m.fd, "TOPIC", channel_name), s, r);
	namesHandler(Message(m.fd, "NAMES", channel_name), s, r);
}

void partHandler(const Message &m, State &s, Responses &r)
{
	Client &client = s.clients[m.fd];

	if (m.params.size() < 1)
		return r.push_back(Message(m.fd, ERR_NEEDMOREPARAMS, client, m.verb, RED "Not enough parameters" RESET));

	std::string channel_name = m.params.at(0);

	if (s.channels.find(channel_name) == s.channels.end())
		return r.push_back(Message(m.fd, ERR_NOSUCHCHANNEL, client, channel_name, RED "No such channel" RESET));

	Channel &channel = s.channels[channel_name];
	if (channel.client_ids.find(m.fd) == channel.client_ids.end())
		return r.push_back(Message(m.fd, ERR_NOTONCHANNEL, client, channel_name, RED "You're not on that channel" RESET));

	// broadcast to everyone in the channel before leaving
	Responses broadcast = Message(client.hostmask(), m.fd, "PART", channel_name).repeat(channel.client_ids);
	r.insert(r.end(), broadcast.begin(), broadcast.end());

	channel.client_ids.erase(m.fd); // remove client from channel member list
	channel.op_ids.erase(m.fd);
	if (channel.client_ids.empty()) // if no user in channel
		s.channels.erase(channel_name); // delete channel
}

void kickHandler(const Message &m, State &s, Responses &r)
{
	Client &client = s.clients[m.fd];

	if (m.params.size() < 2)
		return r.push_back(Message(m.fd, ERR_NEEDMOREPARAMS, client, "KICK", RED "Not enough parameters" RESET));

	std::string channel_name = m.params.at(0);
	std::string target_nick = m.params.at(1);
	std::string reason = "Force removed from the channel";
	if (m.params.size() >= 3 && !m.params.at(2).empty())
		reason = m.params.at(2);

	if (s.channels.find(channel_name) == s.channels.end())
		return r.push_back(Message(m.fd, ERR_NOSUCHCHANNEL, client, channel_name, RED "No such channel" RESET));

	Channel &channel = s.channels[channel_name];
	if (!client.isOp() && channel.client_ids.find(m.fd) == channel.client_ids.end())
		return r.push_back(Message(m.fd, ERR_NOTONCHANNEL, client, channel_name, RED "You're not on that channel" RESET));

	if (!client.isOp() && channel.op_ids.find(m.fd) == channel.op_ids.end())
		return r.push_back(Message(m.fd, ERR_CHANOPRIVSNEEDED, client, channel_name, RED "You're not channel operator" RESET));

	int target_fd = s.findClientByNick(target_nick);
	if (target_fd == -1) // target nick not found
		return r.push_back(Message(m.fd, ERR_NOSUCHNICK, client, target_nick, RED "No such nick"));

	if (channel.client_ids.find(target_fd) == channel.client_ids.end()) // target not in channel
		return r.push_back(Message(m.fd, ERR_USERNOTINCHANNEL, client, target_nick + " " + channel_name, RED "They aren't on that channel" RESET));

	// broadcast to everyone in the channel
	Responses broadcast = Message(client.hostmask(), m.fd, "KICK", channel_name, s.clients[target_fd], reason)
		.repeat(channel.client_ids);
	r.insert(r.end(), broadcast.begin(), broadcast.end());

	// remove target from channel
	channel.client_ids.erase(target_fd); // remove target from channel member list
	channel.invited_ids.erase(target_fd); // remove target from channel invit list
	channel.op_ids.erase(target_fd); // remove from ops if was op
	if (channel.client_ids.empty()) // if no user in channel
		s.channels.erase(channel_name); // delete channel
}

void inviteHandler(const Message &m, State &s, Responses &r)
{
	Client &client = s.clients[m.fd];
	if (m.params.size() < 2)
		return r.push_back(Message(m.fd, ERR_NEEDMOREPARAMS, client, "INVITE", RED "Not enough parameters" RESET));

	std::string target_nick = m.params.at(0);
	std::string channel_name = m.params.at(1);
	std::string reason;
	if (m.params.size() >= 3)
		reason = m.params.at(2);

	if (s.channels.find(channel_name) == s.channels.end())
		return r.push_back(Message(m.fd, ERR_NOSUCHCHANNEL, client, channel_name, RED "No such channel" RESET));

	Channel &channel = s.channels[channel_name];
	if (!client.isOp() && channel.client_ids.find(m.fd) == channel.client_ids.end())
		return r.push_back(Message(m.fd, ERR_NOTONCHANNEL, client, channel_name, RED "You're not on that channel" RESET));

	
	if (!client.isOp() && channel.op_ids.find(m.fd) == channel.op_ids.end())
		return r.push_back(Message(m.fd, ERR_CHANOPRIVSNEEDED, client, channel_name, RED "You're not channel operator" RESET));

	int target_fd = s.findClientByNick(target_nick);
	if (target_fd == -1) // target nick not found
		return r.push_back(Message(m.fd, ERR_NOSUCHNICK, client, target_nick, RED "No such nick"));

	if (channel.client_ids.find(target_fd) != channel.client_ids.end()) // target in channel
		return r.push_back(Message(m.fd, ERR_USERONCHANNEL, client, RED + target_nick + " " + channel_name, "is already on channel" RESET));

	channel.invited_ids.insert(target_fd);
	r.push_back(Message(m.fd, RPL_INVITING, client, target_nick, channel_name));
	r.push_back(Message(client.hostmask(), target_fd, "INVITE", target_nick, channel_name));
}

void topicHandler(const Message &m, State &s, Responses &r)
{
	Client &client = s.clients[m.fd];
	if (m.params.size() < 1)
		return r.push_back(Message(m.fd, ERR_NEEDMOREPARAMS, client, "TOPIC", RED "Not enough parameters" RESET));

	std::string channel_name = m.params.at(0);
	if (s.channels.find(channel_name) == s.channels.end())
		return r.push_back(Message(m.fd, ERR_NOSUCHCHANNEL, client, channel_name, RED "No such channel" RESET));
	Channel &channel = s.channels[channel_name];

	if (m.params.size() < 2) // view topic only
	{
		if (channel.topic.empty())
			return r.push_back(Message(m.fd, RPL_NOTOPIC, client, channel_name, "No topic is set"));
		return r.push_back(Message(m.fd, RPL_TOPIC, client, channel_name, channel.topic));
	}

	std::string new_topic = m.params.at(1);

	if (!client.isOp() && channel.client_ids.find(m.fd) == channel.client_ids.end())
		return r.push_back(Message(m.fd, ERR_NOTONCHANNEL, client, channel_name, RED "You're not on that channel" RESET));

	if (!client.isOp() && channel.modes.count('t') && channel.op_ids.find(m.fd) == channel.op_ids.end())
		return r.push_back(Message(m.fd, ERR_CHANOPRIVSNEEDED, client, channel_name, RED "You're not channel operator. Mode is +t" RESET));

	channel.topic = new_topic;
	Responses broadcast = Message(client.hostmask(), m.fd, "TOPIC", channel_name, channel.topic)
		.repeat(channel.client_ids);
	r.insert(r.end(), broadcast.begin(), broadcast.end());
}

void namesHandler(const Message &m, State &s, Responses &r)
{
	Client &client = s.clients[m.fd];
	if (m.params.size() < 1)
		return r.push_back(Message(m.fd, ERR_NEEDMOREPARAMS, client, "NAMES", RED "Not enough parameters" RESET));

	std::string channel_name = m.params.at(0);
	if (s.channels.find(channel_name) == s.channels.end())
		return r.push_back(Message(m.fd, ERR_NOSUCHCHANNEL, client, channel_name, RED "No such channel" RESET));

	Message namreply(m.fd, RPL_NAMREPLY, client, "=", channel_name);
	namreply.params.push_back(s.getNamesInChannel(channel_name));
	r.push_back(namreply);
	r.push_back(Message(m.fd, RPL_ENDOFNAMES, client, channel_name, "End of /names reply"));
}
