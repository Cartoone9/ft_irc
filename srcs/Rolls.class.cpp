#include "Rolls.class.hpp"
#include "colors.hpp"

Rolls::Rolls()
{
	std::string	temp_keys[] = {"d2", "d4", "d6", "d8",
								"d10", "d12", "d20", "d100"};

	_n_keys = sizeof(temp_keys)/sizeof(temp_keys[0]);

	_keys.insert(_keys.end(), temp_keys, temp_keys + _n_keys);

	for (size_t i = 0; i < _n_keys; i++)
		_counts[_keys[i]] = 0;
}

static void	clearOss(std::ostringstream& oss)
{
	// empty and clear oss
	oss.str("");
	oss.clear();
}

static void	addElementsOss(std::ostringstream& oss,
		std::vector<std::string>& ret_vec,
		const char* str_to_add)
{
	oss << str_to_add;
	ret_vec.push_back(oss.str());
	clearOss(oss);
}

std::vector<std::string>	Rolls::info() const
{
	std::vector<std::string>	ret_vec;
	std::ostringstream			oss;

	addElementsOss(oss, ret_vec, "-----");

	addElementsOss(oss, ret_vec, UGREEN "Available dice:" RESET);
	for (size_t i = 0; i < _n_keys; i++)
	{
		oss << _keys.at(i);
		if (i + 1 < _n_keys)
			oss << ", ";
	}
	ret_vec.push_back(oss.str());
	clearOss(oss);
	addElementsOss(oss, ret_vec, "");

	addElementsOss(oss, ret_vec, UYELLOW "Format:" RESET);
	addElementsOss(oss, ret_vec, "[multiplier]d<dice_type>");
	addElementsOss(oss, ret_vec, "The multiplier is optional.");
	addElementsOss(oss, ret_vec, "");

	addElementsOss(oss, ret_vec, UORANGE "Examples:" RESET);
	addElementsOss(oss, ret_vec, "d6     = roll one 6-sided die");
	addElementsOss(oss, ret_vec, "3d8    = roll three 8-sided dice");
	addElementsOss(oss, ret_vec, "10d2   = roll ten 2-sided dice");
	addElementsOss(oss, ret_vec, "");
	addElementsOss(oss, ret_vec, "Multiple dice can be rolled at once.");
	addElementsOss(oss, ret_vec, "Syntax: d2 d4d6 2d8 6d20");
	addElementsOss(oss, ret_vec, "");

	addElementsOss(oss, ret_vec, URED "Limits:" RESET);
	addElementsOss(oss, ret_vec, "100 rolls per dice type.");

	addElementsOss(oss, ret_vec, "-----");

	return (ret_vec);
}

std::string	Rolls::list() const
{
	std::ostringstream	oss;
	bool				sep_needed = false;

	for (size_t i = 0; i < _n_keys; i++)
	{
		const std::string& key = _keys[i];
		size_t key_count = _counts.at(key);
		if (key_count > 0)
		{
			if (sep_needed)
				oss << " / ";
			oss << key << ": " << key_count;
			sep_needed = true;
		}
	}

	return (oss.str());
}

std::string	Rolls::getKey(size_t index) const
{
	if (index < _keys.size())
		return (_keys[index]);
	else
		return ("");
}

size_t	Rolls::getKeysNum() const
{
	return (_n_keys);
}

void	Rolls::addToCount(const std::string& key)
{
	if (_counts[key] + 1 <= 100)
		_counts[key]++;
}

void	Rolls::addToCount(const std::string& key, int value)
{
	if (_counts[key] + value <= 100)
		_counts[key] += value;
	else
		_counts[key] = 100;
}

size_t	Rolls::getCount(const std::string& key) const
{
	std::map<std::string, size_t>::const_iterator it = _counts.find(key);
	if (it != _counts.end())
		return (it->second);
	else
		return (0);
}
