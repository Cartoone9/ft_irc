#ifndef MESSAGE_STRUCT_HPP
#define MESSAGE_STRUCT_HPP

#include <string>
#include <vector>
#include <set>

struct Message
{
	int fd;
	std::string source, verb;
	std::vector<std::string> params;

	Message(int fd, const std::string &raw);
	Message(
		const std::string &source,
		int fd,
		const std::string &verb,
		const std::vector<std::string> &params);

	// support for copy
	Message &operator=(const Message &);

	// assemble/serialize outcoming message to raw
	std::string assemble() const;

	// validate message, middle params with no space, verb not empty
	bool isValid() const;

	bool shouldDisconnect() const;

	// more constructors for easier usage
	Message(
		const std::string &source,
		int fd,
		const std::string &verb);
	Message(
		const std::string &source,
		int fd,
		const std::string &verb,
		const std::string &param);
	Message(
		const std::string &source,
		int fd,
		const std::string &verb,
		const std::string &param1,
		const std::string &param2);
	Message(
		const std::string &source,
		int fd,
		const std::string &verb,
		const std::string &param1,
		const std::string &param2,
		const std::string &param3);

	// same without source prefix, for example for error responses
	Message(
		int fd,
		const std::string &verb,
		const std::string &param);
	Message(
		int fd,
		const std::string &verb,
		const std::string &param1,
		const std::string &param2);
	Message(
		int fd,
		const std::string &verb,
		const std::string &param1,
		const std::string &param2,
		const std::string &param3);

	// duplicate the same message to various fd, only the fd changes
	std::vector<Message> repeat(const std::vector<int>& fds) const;
	std::vector<Message> repeat(const std::set<int>& fds) const;

};

bool operator==(const Message &msg1, const Message &msg2);
bool operator==(const Message &msg, const std::string &raw);
bool operator==(const std::string &raw, const Message &msg);

// easy find a message by its fd, nullptr if not found
const Message* find_by_fd(const std::vector<Message>& vec, int id);

#endif // #ifndef MESSAGE_STRUCT_HPP
