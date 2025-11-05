#include "Message.struct.hpp"
#include <sstream>
#include <algorithm>
#include "numerics.hpp"

// Example: Message(42, "NICK alice\r\n")
// parse raw string into verb="NICK", params=["alice"]
Message::Message(int fd, const std::string &raw)
	: fd(fd)
{
	// find end, skipping single \r or \n
	size_t end_pos = raw.find("\r\n");
	if (end_pos == std::string::npos)
		end_pos = raw.length();

	// max 512 bytes including \r\n
	if (end_pos > 510)
		end_pos = 510;
	
	std::stringstream ss(raw.substr(0, end_pos));

	// parse optional source prefix
	if (ss.peek() == ':')
	{
		ss.ignore(1);
		std::getline(ss, source, ' ');
	}

	// skip spaces
	while (ss.peek() == ' ')
		ss.ignore(1);

	// parse verb
	std::getline(ss, verb, ' ');

	// skip spaces
	while (ss.peek() == ' ')
		ss.ignore(1);

	// parse optional parameteers, and trailing text
	std::string param;
	while (ss.good())
	{
		if (ss.peek() == ':')
		{
			ss.ignore(1);
			std::getline(ss, param);
			params.push_back(param);
			break ;
		}
		std::getline(ss, param, ' ');
		if (!param.empty())
			params.push_back(param);

		// skip spaces
		while (ss.peek() == ' ')
			ss.ignore(1);
	}
}

// Example: verb="PRIVMSG", params=["#channel", "hello"]
// return "PRIVMSG #channel :hello\r\n"
std::string Message::assemble() const
{
	std::string raw;

	if (!source.empty())
		raw += ":" + source + " ";

	raw += verb;

	for (size_t i = 0; i + 1 < params.size(); i++)
		raw += " " + params[i];

	if (!params.empty())
		raw += " :" + params.back();

	raw += "\r\n";
	return raw;
}

static bool _invalid(const std::string &s)
{
	return s.empty() || s.find(' ') != std::string::npos;
}

// Validate message format
// check: verb not empty, params don't contain spaces (except last)
bool Message::isValid() const
{
	if (_invalid(verb))
		return false;
	if (!source.empty() && _invalid(source))
		return false;
	for (size_t i = 0; i + 1 < params.size(); i++)
		if (_invalid(params[i]))
			return false;
	return true;
}

bool operator==(const Message &msg1, const Message &msg2)
{
	return msg1.fd == msg2.fd &&
		   msg1.source == msg2.source &&
		   msg1.verb == msg2.verb &&
		   msg1.params == msg2.params;
}

bool operator==(const Message &msg, const std::string &raw)
{
	return msg == Message(msg.fd, raw);
}

bool operator==(const std::string &raw, const Message &msg)
{
	return msg == Message(msg.fd, raw);
}

bool Message::shouldDisconnect() const
{
	return verb == "ERROR";
}

Message::Message(
	const std::string &source,
	int fd,
	const std::string &verb,
	const std::vector<std::string> &params)
	: fd(fd), source(source), verb(verb), params(params)
{
}

Message::Message(
	const std::string &source,
	int fd,
	const std::string &verb)
	: fd(fd), source(source), verb(verb)
{
}

Message::Message(
	const std::string &source,
	int fd,
	const std::string &verb,
	const std::string &param)
	: fd(fd), source(source), verb(verb)
{
	params.push_back(param);
}

Message::Message(
	const std::string &source,
	int fd,
	const std::string &verb,
	const std::string &param1,
	const std::string &param2)
	: fd(fd), source(source), verb(verb)
{
	params.push_back(param1);
	params.push_back(param2);
}

Message::Message(
	const std::string &source,
	int fd,
	const std::string &verb,
	const std::string &param1,
	const std::string &param2,
	const std::string &param3)
	: fd(fd), source(source), verb(verb)
{
	params.push_back(param1);
	params.push_back(param2);
	params.push_back(param3);
}

Message::Message(
	int fd,
	const std::string &verb,
	const std::string &param)
	: fd(fd), verb(verb)
{
	params.push_back(param);
}

Message::Message(
	int fd,
	const std::string &verb,
	const std::string &param1,
	const std::string &param2)
	: fd(fd), verb(verb)
{
	params.push_back(param1);
	params.push_back(param2);
}

Message::Message(
	int fd,
	const std::string &verb,
	const std::string &param1,
	const std::string &param2,
	const std::string &param3)
	: fd(fd), verb(verb)
{
	params.push_back(param1);
	params.push_back(param2);
	params.push_back(param3);
}

// functor needed for std::find_if
struct HasFd
{
	int target_fd;
	explicit HasFd(int fd) : target_fd(fd) {}
	bool operator()(const Message &msg) const
	{
		return msg.fd == target_fd;
	}
};

// easy find a message by its id (fd)
const Message* find_by_fd(const std::vector<Message>& vec, int fd) {
    std::vector<Message>::const_iterator it = 
        std::find_if(vec.begin(), vec.end(), HasFd(fd));
    return (it != vec.end()) ? &(*it) : 0;
}

// helper to broadcast the same message to other clients
// (the vector doesn't include the original 
// fd unless it is in the fds parameter)
std::vector<Message> Message::repeat(const std::vector<int> &fds) const
{
	std::vector<Message> messages(fds.size(), *this);
    for (size_t i = 0; i < fds.size(); ++i)
        messages[i].fd = fds[i];
    return messages;
}

// helper to broadcast the same message to other clients
// example usage: msg.repeat(channel.client_ids)
std::vector<Message> Message::repeat(const std::set<int> &fds) const
{
	std::vector<Message> messages(fds.size(), *this);
	std::set<int>::const_iterator it_fd = fds.begin();
	std::vector<Message>::iterator it_msg = messages.begin();
	while (it_fd != fds.end())
	{
		it_msg->fd = *it_fd;
		++it_msg;
		++it_fd;
	}
	return messages;
}

Message &Message::operator=(const Message &other)
{
	if (&other == this)
		return *this;
	this->fd = other.fd;
	this->source = other.source;
	this->verb = other.verb;
	this->params = other.params;
	return *this;
}