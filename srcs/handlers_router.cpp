#include "colors.hpp"
#include "handlers.hpp"
#include "numerics.hpp"

void messageRouter(const Message &m, State &s, std::vector<Message> &r)
{
	int id = m.fd;
	Client &c = s.clients[id]; // IMPORTANT: this creates entry if it doesn't exist

	if (m.verb == "CAP")
		return capHandler(m, s, r);
	if (m.verb == "PASS")
		return passHandler(m, s, r);
	if (m.verb == "QUIT")
		return quitHandler(m, s, r);

	if (c.status != WELCOMED)
	{
		if (c.status == CONNECTED && s.password.empty())
			c.status = AUTHENTICATED; // password not required
		if (m.verb == "USER")
			userHandler(m, s, r);
		else if (m.verb == "NICK")
			nickHandler(m, s, r);
		else if (m.verb == "JOIN")
			return ;
		else
			return r.push_back(Message(id, ERR_NOTREGISTERED, c, RED "You have not registered" RESET));
		if (c.username.empty() || c.nick.empty())
			return;
		if (c.status != AUTHENTICATED)
		{
			r.push_back(Message(id, ERR_PASSWDMISMATCH, c, RED "Password incorrect" RESET));
			r.push_back(Message(id, "ERROR", "Closing Link: " + c.nick + " (Connection failed)"));
			s.removeClient(m.fd);
			return ;
		}
		return welcomeHandler(m, s, r);
	}

	if (m.verb == "USER")
		return userHandler(m, s, r);
	if (m.verb == "NICK")
		return nickHandler(m, s, r);
	if (m.verb == "PART")
		return partHandler(m, s, r);
	if (m.verb == "JOIN")
		return joinHandler(m, s, r);
	if (m.verb == "PING")
		return pingHandler(m, s, r);
	if (m.verb == "KICK")
		return kickHandler(m, s, r);
	if (m.verb == "PRIVMSG")
		return privmsgHandler(m, s, r);
	if (m.verb == "NOTICE")
		return noticeHandler(m, s, r);
	if (m.verb == "MOTD")
		return motdHandler(m, s, r);
	if (m.verb == "MODE")
		return modeHandler(m, s, r);
	if (m.verb == "OPER")
		return operHandler(m, s, r);
	if (m.verb == "INVITE")
		return inviteHandler(m, s, r);
	if (m.verb == "TOPIC")
		return topicHandler(m, s, r);
	if (m.verb == "NAMES")
		return namesHandler(m, s, r);
}

// a handler that does not route messages but simply repeats
void parrot(const Message &m, State &_, Responses &r)
{
	(void)_;
	r.push_back(m);
}
