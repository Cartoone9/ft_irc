#include "colors.hpp"
#include "handlers.hpp"
#include "numerics.hpp"

// Example: CAP LS
// Answers the client for a list of the server's capabilities
void capHandler(const Message &m, State &s, Responses &r)
{
	if (m.verb != "CAP")
		return;
	Client &client = s.clients[m.fd];
	if (m.params.size() < 1)
		return r.push_back(Message(m.fd, ERR_NEEDMOREPARAMS, client, "CAP", RED "Need more params" RESET));
	std::string arg = m.params.at(0);
	if (arg == "LS")
		return r.push_back(Message(m.fd, "CAP * LS :none"));
}

// Example: PING localhost
// Answers the ping request with the token
void pingHandler(const Message &m, State &s, Responses &r)
{
	if (m.verb != "PING")
		return;
	Client &client = s.clients[m.fd];
	if (m.params.size() < 1)
		return r.push_back(Message(m.fd, ERR_NEEDMOREPARAMS, client, "PING", RED "Need more params" RESET));
	std::string token = m.params.at(0);
	return r.push_back(Message(m.fd, "PONG", token));
}

void motdHandler(const Message &m, State &s, Responses &r)
{
	Client &client = s.clients[m.fd];
	if (s.motd.empty())
	{
		Message msg = Message(m.fd, ERR_NOMOTD, client, PURPLE "No message of the day" RESET);
		r.push_back(msg);
		return ;
	}
	r.push_back(Message(m.fd, RPL_MOTDSTART, client, PURPLE "- Message of the day -" RESET));

	// Split MOTD by newlines, send each line as separate RPL_MOTD
	std::string motd_copy = s.motd;
	std::string line;
	size_t pos = 0;
	while ((pos = motd_copy.find('\n')) != std::string::npos)
	{
		line = motd_copy.substr(0, pos);
		if (!line.empty())
			r.push_back(Message(m.fd, RPL_MOTD, client, line));
		motd_copy.erase(0, pos + 1);
	}
	if (!motd_copy.empty())
		r.push_back(Message(m.fd, RPL_MOTD, client, motd_copy));

	r.push_back(Message(m.fd, RPL_ENDOFMOTD, client, PURPLE "- End of /MOTD -" RESET));
}
