#include <sstream>

// returns 0
size_t str_to_size(const std::string &str)
{
	std::istringstream is(str);
	size_t n;
	is >> n;
	if (is.bad())
		return (0);
	return n;
}

std::string size_to_str(size_t size)
{
	std::ostringstream str;
	str << size;
	return str.str();
}