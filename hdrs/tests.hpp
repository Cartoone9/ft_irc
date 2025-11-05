#ifndef TESTS_HPP
#define TESTS_HPP

#include <iostream>
#include <sstream>

extern int test_exit_code;
extern std::string test_name;

inline void throw_if_false(bool condition, std::string file, int line)
{
	if (!condition)
	{
		std::ostringstream oss;
		oss << file << ":" << line;
		throw std::runtime_error(oss.str());
	}
}

template <typename T, typename U>
inline void throw_if_different(T a, U b, std::string file, int line)
{
	if (a != b)
	{
		std::ostringstream oss;
		oss << file << ":" << line << " expected `" << a << "`, found `" << b << "`";
		throw std::runtime_error(oss.str());
	}
}

#define assert(condition) throw_if_false(condition, __FILE__, __LINE__)
#define assert_eq(a, b) throw_if_different(a, b, __FILE__, __LINE__)

#define TEST(name)    \
	test_name = name; \
	try               \
	{

#define TEST_PRINT                                                                              \
	std::cout << "[\033[32m OK \033[0m] " << test_name << std::endl;                            \
	}                                                                                           \
	catch (const std::exception &e)                                                             \
	{                                                                                           \
		std::cout << "[\033[31mFAIL\033[0m] " << test_name << ": at " << e.what() << std::endl; \
		test_exit_code = 1;                                                                     \
	}

#endif