#include "handlers.hpp"
#include "numerics.hpp"
#include "utils.hpp"

// Example: PASS secretpassword
// check against s.password, set client status to AUTHENTICATED
void passHandler(const Message &m, State &s, Responses &r)
{
	if (m.verb != "PASS")
		return;
	Client &client = s.clients[m.fd];
	if (m.params.size() < 1)
		return r.push_back(Message(m.fd, ERR_NEEDMOREPARAMS, client, "PASS", RED "Need more params" RESET));
	if (s.clients[m.fd].status == WELCOMED)
		return r.push_back(Message(m.fd, ERR_ALREADYREGISTERED, client, RED "Already registered" RESET));
	if (m.params.at(0) != s.password)
	{
		s.clients[m.fd].status = CONNECTED;
		return r.push_back(Message(m.fd, ERR_PASSWDMISMATCH, client, RED "Incorrect password" RESET));
	}
	else
		s.clients[m.fd].status = AUTHENTICATED;
}

static bool isValidNick(const std::string &nick)
{
	if (nick.empty())
		return false;
	char c = nick.at(0);
	if (c == ':' || c == '#' || c == '&')
		return false;
	if (c >= '0' && c <= '9')
		return false;
	if (nick.find(' ') < nick.size())
		return false;
	return true;
}

// Example: NICK alice
// set s.clients[fd].nick = "alice", check for conflicts
void nickHandler(const Message &m, State &s, Responses &r)
{
	Client &client = s.clients[m.fd];
	if (m.params.size() < 1)
		return r.push_back(Message(m.fd, ERR_NONICKNAMEGIVEN, client, RED "No nick given" RESET));
	std::string new_nick = m.params.at(0);
	if (!isValidNick(new_nick))
		return r.push_back(Message(m.fd, ERR_ERRONEUSNICKNAME, client, "*", RED "Erroneus nick" RESET));
	for (std::map<int, Client>::iterator it = s.clients.begin();
		 it != s.clients.end(); ++it)
	{
		if ((*it).second.nick == new_nick)
			return r.push_back(Message(m.fd, ERR_NICKNAMEINUSE, client, new_nick, RED "Nick in use" RESET));
	}
	std::string old_nick = s.clients[m.fd].nick;
	if (!old_nick.empty())
	{
		Message broadcast(client.hostmask(), m.fd, "NICK", new_nick);
		std::vector<int> others = s.clientsInChannelsWith(m.fd);
		std::vector<Message> messages = broadcast.repeat(others);
		r.push_back(broadcast);
		r.insert(r.end(), messages.begin(), messages.end());
	}
	s.clients[m.fd].nick = new_nick;
}

// Example: USER alice 0 * :Alice Smith
// set s.clients[fd].user = "alice"
void userHandler(const Message &m, State &s, Responses &r)
{
	Client &client = s.clients[m.fd];
	if (m.params.size() < 4)
		return r.push_back(Message(m.fd, ERR_NEEDMOREPARAMS, client, "USER", RED "Need more params" RESET));
	if (client.status == WELCOMED)
		return r.push_back(Message(m.fd, ERR_ALREADYREGISTERED, client, RED "Already registered" RESET));
	std::string username = m.params.at(0);
	if (username.at(0) != '~') // no ident server
		username = "~" + username;
	client.username = username;
	client.realname = m.params.at(3);
}

// Example: QUIT :Goodbye
// remove client from s.clients and all channels
void quitHandler(const Message &m, State &s, Responses &r)
{
	Client &client = s.clients[m.fd];
	if (client.status == WELCOMED)
	{
		Message broadcast(client.hostmask(), m.fd, "QUIT");
		if (m.params.size() == 1)
			broadcast.params.push_back(m.params.at(0));
		std::vector<int> others = s.clientsInChannelsWith(m.fd);
		std::vector<Message> messages = broadcast.repeat(others);
		r.insert(r.end(), messages.begin(), messages.end());
	}
	r.push_back(Message(m.fd, "ERROR", "Terminated"));
	s.removeClient(m.fd);
}

// Example: after NICK + USER completed
// send 001, 002, 003, 004 numeric messages to client
void welcomeHandler(const Message &m, State &s, Responses &r)
{
	std::string client = s.clients[m.fd];
	Message m1(m.fd, RPL_WELCOME, client, RED "Welcome!" RESET);
	Message m2(m.fd, RPL_YOURHOST, client, ORANGE "Your host is 0.0.0.0, running " SERVER_NAME RESET);
	Message m3(m.fd, RPL_CREATED, client, YELLOW "Created at " + timeToStr(s.start_time), dateToStr(s.start_time) + RESET);
	Message m4(m.fd, RPL_MYINFO, client, GREEN SERVER_NAME);
	m4.params.push_back(SERVER_VERSION); // server version
	m4.params.push_back("o"); // available user modes
	m4.params.push_back("it"); // available channel modes
	m4.params.push_back("klo" RESET); // available channel modes with parameters
	Message m5(m.fd, RPL_ISUPPORT, client);
	m5.params.push_back(TEAL "CASEMAPPING=ascii");
	m5.params.push_back("is supported by this server" RESET);
	r.push_back(m1);
	r.push_back(m2);
	r.push_back(m3);
	r.push_back(m4);
	r.push_back(m5);
	s.clients[m.fd].status = WELCOMED;
	return motdHandler(m, s, r);
}
