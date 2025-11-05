#ifndef CHANNEL_STRUCT_HPP
#define CHANNEL_STRUCT_HPP

#include <string>
#include <set>

struct Channel
{
	std::string name, topic, key;
	std::set<int> client_ids, op_ids, invited_ids;
	std::set<char> modes;
	size_t userlimit;
};

#endif // #ifndef CHANNEL_STRUCT_HPP
