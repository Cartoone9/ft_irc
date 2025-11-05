#include "colors.hpp"
#include "handlers.hpp"
#include "numerics.hpp"

static void errorNeedMoreParams(const Message &m, State &s, Responses &r)
{
	return r.push_back(Message(m.fd, ERR_NEEDMOREPARAMS, s.clients[m.fd],
							   m.verb, RED "Not enough parameters" RESET));
}

static void errorNoSuchNick(const Message &m, State &s, Responses &r)
{
	return r.push_back(Message(m.fd, ERR_NOSUCHNICK, s.clients[m.fd],
							   m.verb, RED "No such nick/channel" RESET));
}

static void errorUnknownFlag(const Message &m, State &s, Responses &r)
{
	return r.push_back(Message(m.fd, ERR_UMODEUNKNOWNFLAG, s.clients[m.fd],
							   RED "Unknown MODE flag" RESET));
}

static void userModeHandler(const Message &m, State &s, Responses &r)
{
	Client &client = s.clients[m.fd];
	if (m.params.size() < 2)
	{
		std::string modestring = "+" +
								 std::string(client.modes.begin(), client.modes.end());
		return r.push_back(Message(m.fd, RPL_UMODEIS, client, modestring));
	}
	std::string target = m.params.at(0);
	if (s.findClientByNick(target) == -1)
		return r.push_back(Message(m.fd, ERR_NOSUCHNICK, client, target,
								   RED "No such nick" RESET));
	if (target != client.nick)
		return r.push_back(Message(m.fd, ERR_USERSDONTMATCH, client,
								   RED "Cant change mode for other users" RESET));
	std::string modestring = m.params.at(1);
	if (modestring.empty())
		return errorUnknownFlag(m, s, r);
	if (modestring[0] == '+')
	{
		if (modestring.find('o') != std::string::npos)
			return r.push_back(Message(m.fd, ERR_NOPRIVILEGES, client,
									   RED "Permission denied - You're not an IRC op" RESET));
		// irssi sends MODE +i even if it is not supported, replying UMODEIS
		if (modestring == "+i")
			return userModeHandler(Message(m.fd, "MODE", client.nick), s, r);
		return errorUnknownFlag(m, s, r);
	}
	else if (modestring[0] == '-')
	{
		if (modestring.find('o') != std::string::npos)
		{
			client.modes.clear();
			return userModeHandler(Message(m.fd, "MODE", m.params[0]), s, r);
		}
		return errorUnknownFlag(m, s, r);
	}
	else if (modestring[0] == '-')
	{
		if (modestring.find('i') != std::string::npos)
		{
			client.modes.clear();
			return userModeHandler(Message(m.fd, "MODE", m.params[0]), s, r);
		}
		return errorUnknownFlag(m, s, r);
	}
	else
		return errorUnknownFlag(m, s, r);
}

// sends RPL_CHANNELMODEIS <client> <channel> <modestring> <mode arguments>...
// - as a response to MODE <channel>
// - as a response to setting a flag that is already set
// - NOT as a response to changing a flag, that response is "MODE"
static void channelQueryModeHandler(const Message &m, State &s, Responses &r)
{
	const std::string &ch_name = m.params[0];
	const Channel &channel = s.channels[ch_name];
	Message response = Message(m.fd, RPL_CHANNELMODEIS, ch_name, "+");
	std::set<char>::const_iterator it;
	for (it = channel.modes.begin(); it != channel.modes.end(); ++it)
	{
		response.params[1] += *it;
		if (*it == 'l')
			response.params.push_back(size_to_str(channel.userlimit));
	}

	r.push_back(response);
}

static void inviteFlagHandler(const Message &m, State &s, Responses &r)
{
	const std::string &ch_name = m.params[0];
	Channel &channel = s.channels[ch_name];
	if (m.params[1][0] == '-')
	{
		if (!channel.modes.count('i'))
			return ;
		channel.modes.erase('i');
	}
	else
	{
		if (channel.modes.count('i'))
			return ;
		channel.modes.insert('i');
	}

	Message notification = m;
	notification.source = s.clients[m.fd].hostmask();
	Responses broadcast = notification.repeat(channel.client_ids);
	r.insert(r.end(), broadcast.begin(), broadcast.end());
}

static void topicFlagHandler(const Message &m, State &s, Responses &r)
{
	Channel &channel = s.channels[m.params[0]];
	if (m.params[1][0] == '-')
	{
		if (!channel.modes.count('t'))
			return ;
		channel.modes.erase('t');
	}
	else
	{
		if (channel.modes.count('t'))
			return ;
		channel.modes.insert('t');
	}
	Message notification = m;
	notification.source = s.clients[m.fd].hostmask();
	Responses broadcast = notification.repeat(channel.client_ids);
	r.insert(r.end(), broadcast.begin(), broadcast.end());
}

static void keyFlagHandler(const Message &m, State &s, Responses &r)
{
	Channel &channel = s.channels[m.params[0]];
	if (m.params[1][0] == '-')
	{
		if (!channel.modes.count('k'))
			return ;
		channel.modes.erase('k');
		channel.key.clear();
	}
	else
	{
		std::string key = m.params[2];
		// Key can't be empty or contain spaces
		if (key.empty())
			return r.push_back(Message(m.fd, ERR_NEEDMOREPARAMS,
				s.clients[m.fd], "MODE", RED "Key cannot be empty" RESET));
		if (key.find(' ') != std::string::npos || key.find(',') != std::string::npos)
			return r.push_back(Message(m.fd, ERR_INVALIDKEY,
				s.clients[m.fd], m.params[0], RED "Invalid key" RESET));
		channel.modes.insert('k');
		channel.key = key;
	}
	// Broadcast without revealing the key
	Message notification(s.clients[m.fd], m.fd, "MODE", m.params[0], m.params[1]);
	notification.source = s.clients[m.fd].hostmask();
	Responses broadcast = notification.repeat(channel.client_ids);
	r.insert(r.end(), broadcast.begin(), broadcast.end());
}

static void opFlagHandler(const Message &m, State &s, Responses &r)
{
	if (m.params.size() < 3)
		return r.push_back(Message(m.fd, ERR_NEEDMOREPARAMS,
			s.clients[m.fd], "MODE", RED "Need more params" RESET));
	std::string ch_name = m.params[0];
	Channel &channel = s.channels[ch_name];
	const std::string &target_nick = m.params[2];
	int target_fd = s.findClientByNick(target_nick);
	if (!channel.client_ids.count(target_fd))
		return r.push_back(Message(m.fd, ERR_USERNOTINCHANNEL,
			s.clients[m.fd], target_nick, ch_name));
	if (m.params[1][0] == '-')
	{
		if (!channel.op_ids.count(target_fd))
			return ;
		channel.op_ids.erase(target_fd);
	}
	else
	{
		if (channel.op_ids.count(target_fd))
			return ;
		channel.op_ids.insert(target_fd);
	}
	Message notification = m;
	notification.source = s.clients[m.fd].hostmask();
	Responses broadcast = notification.repeat(channel.client_ids);
	r.insert(r.end(), broadcast.begin(), broadcast.end());
}

static void limitFlagHandler(const Message &m, State &s, Responses &r)
{
	Channel &channel = s.channels[m.params[0]];
	if (m.params[1][0] == '-')
	{
		if (!channel.modes.count('l'))
			return ;
		channel.modes.erase('l');
	}
	else
	{
		size_t newlimit = str_to_size(m.params[2]);
		if (channel.modes.count('l') && channel.userlimit == newlimit)
			return ;
		channel.modes.insert('l');
		channel.userlimit = newlimit;
	}
	Message notification = m;
	notification.source = s.clients[m.fd].hostmask();
	Responses broadcast = notification.repeat(channel.client_ids);
	r.insert(r.end(), broadcast.begin(), broadcast.end());
}

static void channelModeHandler(const Message &m, State &s, Responses &r)
{
	if (m.params.size() < 1)
		return errorNeedMoreParams(m, s, r);
	Client &client = s.clients[m.fd];

	const std::string &ch_name = m.params[0];
	if (!s.channels.count(ch_name))
		return r.push_back(Message(m.fd, ERR_NOSUCHCHANNEL, client,
			ch_name, RED "No such channel" RESET));
	Channel &channel = s.channels[ch_name];

	if (m.params.size() == 1)
		return channelQueryModeHandler(m, s, r);

	if (!channel.op_ids.count(m.fd) && !client.isOp())
		return r.push_back(Message(m.fd, ERR_CHANOPRIVSNEEDED, client,
			ch_name, RED "You are not channel operator" RESET));

	std::string modestring = m.params.at(1);
	if (modestring == "b" || modestring == "I")
		// guard for irssi sending unsupported mode queries
		return;

	char plusminus = '?';
	size_t flag_uses_param_index = 2;
	for (size_t i = 0; i < modestring.size(); ++i)
	{
		char flag = modestring[i];
		if (flag == '+' || flag == '-')
		{
			plusminus = flag;
			continue;
		}
		if (plusminus == '?')
			return errorUnknownFlag(m, s, r);
		Message tmp(m.fd, "MODE", ch_name, std::string(1, plusminus) + flag);
		if (flag == 'o' || (plusminus == '+' && (flag == 'k' || flag == 'l')))
		{
			if (m.params.size() < flag_uses_param_index)
			{
				errorNeedMoreParams(tmp, s, r);
				continue;
			}
			tmp.params.push_back(m.params[flag_uses_param_index]);
			flag_uses_param_index++;
		}
		if (flag == 'i')
			inviteFlagHandler(tmp, s, r);
		else if (flag == 't')
			topicFlagHandler(tmp, s, r);
		else if (flag == 'k')
			keyFlagHandler(tmp, s, r);
		else if (flag == 'o')
			opFlagHandler(tmp, s, r);
		else if (flag == 'l')
			limitFlagHandler(tmp, s, r);
		else
			errorUnknownFlag(tmp, s, r);
	}
}

void modeHandler(const Message &m, State &s, Responses &r)
{
	if (m.params.size() < 1)
		return errorNeedMoreParams(m, s, r);
	const std::string &target = m.params.at(0);
	if (target.empty())
		return errorNoSuchNick(m, s, r);
	if (target[0] == '#')
		channelModeHandler(m, s, r);
	else
		userModeHandler(m, s, r);
}

void operHandler(const Message &m, State &s, Responses &r)
{
	Client &client = s.clients[m.fd];
	if (m.params.size() < 2)
		return errorNeedMoreParams(m, s, r);
	if (m.params.at(0) != s.oper_name || m.params.at(1) != s.oper_pass)
		return r.push_back(Message(m.fd, ERR_PASSWDMISMATCH, client,
								   RED "Oper name or password incorrect" RESET));
	client.modes.insert('o');
	r.push_back(Message(m.fd, RPL_YOUREOPER, client, "You are server operator"));
	r.push_back(Message(client.hostmask(), m.fd, "221", client, "+o"));
}
