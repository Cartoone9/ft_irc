#include "tests.hpp"

int test_exit_code = 0;
std::string test_name;

void tests_parsing();
void tests_auth();
void tests_part();
void tests_join();
void tests_kick();
void tests_motd();
void tests_privmsg();
void tests_oper();
void tests_channel_modes();

int tests()
{
	tests_parsing();
	tests_auth();
	tests_part();
	tests_join();
	tests_kick();
	tests_motd();
	tests_privmsg();
	tests_oper();
	tests_channel_modes();
	return test_exit_code;
}