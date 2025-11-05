#ifndef ROLLS_CLASS_HPP
#define ROLLS_CLASS_HPP

#include <ctime> // time()
#include <iostream>
#include <map>
#include <string>
#include <sstream>
#include <vector>

#include <unistd.h> // getpid()

class Rolls
{
	public:
		Rolls();

		std::vector<std::string>	info() const;
		std::string					list() const;
		std::string					getKey(size_t index) const;
		size_t						getKeysNum() const;
		void						addToCount(const std::string& key);
		void						addToCount(const std::string& key, int value);
		size_t						getCount(const std::string& key) const;
	
	private:
		std::map<std::string, size_t>	_counts;
		std::vector<std::string>		_keys;
		size_t							_n_keys;
};

#endif // #ifndef ROLLS_CLASS_HPP
