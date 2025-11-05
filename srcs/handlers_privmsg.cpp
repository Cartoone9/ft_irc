#include "colors.hpp"
#include "handlers.hpp"
#include "numerics.hpp"

// PRIVMSG <target> :<message>
void privmsgHandler(const Message &m, State &s, Responses &r)
{	
	// au moins 1 parametre
	if (m.params.size() < 1)
	return r.push_back(Message(m.fd, ERR_NORECIPIENT, s.clients[m.fd], RED "No recipient given PRIVMSG" RESET));
	
	// au moins 2 parametres (target + message)
	if (m.params.size() < 2)
	return r.push_back(Message(m.fd, ERR_NOTEXTTOSEND, s.clients[m.fd], RED "No text to send" RESET));
	
	std::string target = m.params[0];
	std::string text = m.params[1];
	
	// SI target : channel
	if (target[0] == '#')
	{
		if (s.channels.find(target) == s.channels.end())
			return r.push_back(Message(m.fd, ERR_CANNOTSENDTOCHAN, s.clients[m.fd], target, RED "No such channel" RESET));
		Channel &channel = s.channels[target];

		if (channel.client_ids.find(m.fd) == channel.client_ids.end())
			return r.push_back(Message(m.fd, ERR_CANNOTSENDTOCHAN, s.clients[m.fd], target, RED "Cannot send to channel" RESET));

		// envoi a tous les membres SAUF a celui qui envoie
		std::string source = s.clients[m.fd].hostmask();
		for (std::set<int>::iterator it = channel.client_ids.begin();
			 it != channel.client_ids.end(); ++it)
		{
			if (*it != m.fd)
			{
				Message msg(source, *it, "PRIVMSG", target, text);
				r.push_back(msg);
			}
		}
	}
	// SI target : un nickname
	else
	{
		int target_fd = s.findClientByNick(target);
		if (target_fd == -1)
			return r.push_back(Message(m.fd, ERR_NOSUCHNICK, s.clients[m.fd], target, RED "No such nick/channel" RESET));

		std::string source = s.clients[m.fd].hostmask();
		Message msg(source, target_fd, "PRIVMSG", target, text);
		r.push_back(msg);
	}
}

void noticeHandler(const Message &m, State &s, Responses &r)
{
	if (m.params.size() < 2)
		return;

	std::string target = m.params[0];
	std::string text = m.params[1];

	// SI target : un channel
	if (target[0] == '#')
	{
		if (s.channels.find(target) == s.channels.end())
			return;

		Channel &channel = s.channels[target];

		if (channel.client_ids.find(m.fd) == channel.client_ids.end())
			return;

		std::string source = s.clients[m.fd].hostmask();
		for (std::set<int>::iterator it = channel.client_ids.begin();
			 it != channel.client_ids.end(); ++it)
		{
			if (*it != m.fd)
			{
				Message msg(source, *it, "NOTICE", target, text);
				r.push_back(msg);
			}
		}
	}
	// SI target : un nickname
	else
	{
		int target_fd = s.findClientByNick(target);
		if (target_fd == -1)
			return;

		std::string source = s.clients[m.fd].hostmask();
		Message msg(source, target_fd, "NOTICE", target, text);
		r.push_back(msg);
	}
}
