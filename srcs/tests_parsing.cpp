#include "tests.hpp"
#include "Message.struct.hpp"

void tests_parsing()
{
	TEST("Message parsing")
	{
		Message m(42, "");
		assert("" == m.source);
		assert("" == m.verb);
		assert(0 == m.params.size());
	}
	{
		Message m(42, "COMMAND\r\n");
		assert("" == m.source);
		assert("COMMAND" == m.verb);
		assert(0 == m.params.size());
	}
	{
		Message m(42, "foo bar baz asdf\r\n");
		assert("" == m.source);
		assert("foo" == m.verb);
		assert(3 == m.params.size());
		assert_eq("bar", m.params.at(0));
		assert_eq("baz", m.params.at(1));
		assert_eq("asdf", m.params.at(2));
	}
	{
		Message m(42, ":coolguy foo bar baz asdf\r\n");
		assert("coolguy" == m.source);
		assert("foo" == m.verb);
		assert(3 == m.params.size());
		assert_eq("bar", m.params.at(0));
		assert_eq("baz", m.params.at(1));
		assert_eq("asdf", m.params.at(2));
	}
	{
		Message m(42, ":src AWAY\r\n");
		assert("src" == m.source);
		assert("AWAY" == m.verb);
		assert(0 == m.params.size());
	}
	{
		Message m(42, ":src JOIN #chan\r\n");
		assert("src" == m.source);
		assert("JOIN" == m.verb);
		assert(1 == m.params.size());
		assert("#chan" == m.params.at(0));
	}
	{ // more spaces
		Message m(42, ":src    JOIN   #chan\r\n");
		assert("src" == m.source);
		assert("JOIN" == m.verb);
		assert(1 == m.params.size());
		assert("#chan" == m.params.at(0));
	}
	{
		Message m(42, ":irc.example.com COMMAND param1 param2 :param3 param3\r\n");
		assert("irc.example.com" == m.source);
		assert("COMMAND" == m.verb);
		assert(3 == m.params.size());
		assert("param1" == m.params.at(0));
		assert("param2" == m.params.at(1));
		assert("param3 param3" == m.params.at(2));
	}
	{
		Message m(42, "foo bar baz :asdf quux\r\n");
		assert("" == m.source);
		assert("foo" == m.verb);
		assert(3 == m.params.size());
		assert_eq("bar", m.params.at(0));
		assert_eq("baz", m.params.at(1));
		assert("asdf quux" == m.params.at(2));
	}
	{
		Message m(42, ":coolguy foo bar baz :asdf quux\r\n");
		assert("coolguy" == m.source);
		assert("foo" == m.verb);
		assert(3 == m.params.size());
		assert_eq("bar", m.params.at(0));
		assert_eq("baz", m.params.at(1));
		assert("asdf quux" == m.params.at(2));
	}
	{
		Message m(42, ":coolguy PRIVMSG bar :lol :) \r\n");
		assert("coolguy" == m.source);
		assert("PRIVMSG" == m.verb);
		assert(2 == m.params.size());
		assert_eq("bar", m.params.at(0));
		assert_eq("lol :) " , m.params.at(1));
	}
	{
		Message m(42, "MEAN :   \r\n");
		assert("" == m.source);
		assert("MEAN" == m.verb);
		assert(1 == m.params.size());
		assert("   " == m.params.at(0));
	}
	{
		Message m(42, ":src JOIN :#chan\r\n");
		assert("src" == m.source);
		assert("JOIN" == m.verb);
		assert(1 == m.params.size());
		assert("#chan" == m.params.at(0));
	}
	{
		Message m(42, ":SomeOp MODE #channel :+i\r\n");
		assert("SomeOp" == m.source);
		assert("MODE" == m.verb);
		assert(2 == m.params.size());
		assert("#channel" == m.params.at(0));
		assert("+i" == m.params.at(1));
	}
	{
		Message m(42, ":SomeOp MODE #channel +oo SomeUser :AnotherUser\r\n");
		assert("SomeOp" == m.source);
		assert("MODE" == m.verb);
		assert(4 == m.params.size());
		assert("#channel" == m.params.at(0));
		assert("+oo" == m.params.at(1));
		assert("SomeUser" == m.params.at(2));
		assert("AnotherUser" == m.params.at(3));
	}
	{
		Message m(42, "foo bar baz :\r\n");
		assert("" == m.source);
		assert("foo" == m.verb);
		assert(3 == m.params.size());
		assert_eq("bar", m.params.at(0));
		assert_eq("baz", m.params.at(1));
		assert("" == m.params.at(2));
	}
	{
		Message m(42, ":coolguy foo bar baz :\r\n");
		assert("coolguy" == m.source);
		assert("foo" == m.verb);
		assert(3 == m.params.size());
		assert_eq("bar", m.params.at(0));
		assert_eq("baz", m.params.at(1));
		assert("" == m.params.at(2));
	}
	{
		Message m(42, ":coolguy foo bar baz :  \r\n");
		assert("coolguy" == m.source);
		assert("foo" == m.verb);
		assert(3 == m.params.size());
		assert_eq("bar", m.params.at(0));
		assert_eq("baz", m.params.at(1));
		assert("  " == m.params.at(2));
	}
	{
		Message m(42, ":coolguy foo bar baz :  asdf quux \r\n");
		assert("coolguy" == m.source);
		assert("foo" == m.verb);
		assert(3 == m.params.size());
		assert_eq("bar", m.params.at(0));
		assert_eq("baz", m.params.at(1));
		assert("  asdf quux " == m.params.at(2));
	}
	{
		Message m(42, "foo bar baz ::asdf\r\n");
		assert("" == m.source);
		assert("foo" == m.verb);
		assert(3 == m.params.size());
		assert_eq("bar", m.params.at(0));
		assert_eq("baz", m.params.at(1));
		assert(":asdf" == m.params.at(2));
	}
	{
		Message m(42, ":src AWAY \r\n");
		assert("src" == m.source);
		assert("AWAY" == m.verb);
		assert(0 == m.params.size());
	}
	{
		Message m(42, ":gravel.mozilla.org MODE #tckk +n \r\n");
		assert("gravel.mozilla.org" == m.source);
		assert("MODE" == m.verb);
		assert(2 == m.params.size());
		assert("#tckk" == m.params.at(0));
		assert("+n" == m.params.at(1));
	}
	{
		Message m(42, ":services.esper.net MODE #foo-bar +o foobar  \r\n");
		assert("services.esper.net" == m.source);
		assert("MODE" == m.verb);
		assert(3 == m.params.size());
		assert("#foo-bar" == m.params.at(0));
		assert("+o" == m.params.at(1));
		assert("foobar" == m.params.at(2));
	}
	{
		Message m(42, ":gravel.mozilla.org 432  #momo"
					  " :Erroneous Nickname: Illegal characters\r\n");
		assert("gravel.mozilla.org" == m.source);
		assert("432" == m.verb);
		assert(2 == m.params.size());
		assert("#momo" == m.params.at(0));
		assert("Erroneous Nickname: Illegal characters" == m.params.at(1));
	}
	{ // include tabs
		Message m(42, ":cool\tguy foo\tfoo \tbar\t baz\tbaz\r\n");
		assert("cool\tguy" == m.source);
		assert("foo\tfoo" == m.verb);
		assert(2 == m.params.size());
		assert("\tbar\t" == m.params.at(0));
		assert("baz\tbaz" == m.params.at(1));
	}
	{
		Message m(42,
				  ":coolguy!ag@net\x035w\x03ork.admin PRIVMSG foo :bar baz\r\n");
		assert("coolguy!ag@net\x035w\x03ork.admin" == m.source);
		assert("PRIVMSG" == m.verb);
		assert(2 == m.params.size());
		assert("foo" == m.params.at(0));
		assert("bar baz" == m.params.at(1));
	}
	{
		Message m(42, ":coolguy!~ag@n\x02et\x03" "05w\x0fork.admin"
					  " PRIVMSG foo :bar baz\r\n");
		assert_eq("coolguy!~ag@n\x02et\x03"
			   "05w\x0fork.admin" ,m.source);
		assert("PRIVMSG" == m.verb);
		assert(2 == m.params.size());
		assert("foo" == m.params.at(0));
		assert("bar baz" == m.params.at(1));
	}
	{ // accepts 512 characters including \r\n
		std::string five_hundred_chars(500, 'a');
		Message m(42, "TEN CHARS " + five_hundred_chars + "\r\n");
		assert_eq("TEN", m.verb);
		assert(2 == m.params.size());
		assert_eq("CHARS", m.params[0]);
		assert_eq(five_hundred_chars, m.params[1]);
	}
	{ // cuts remaining characters
		std::string five_hundred_chars(500, 'a');
		Message m(42, "TEN CHARS " + five_hundred_chars + "?" + "\r\n");
		assert_eq("TEN", m.verb);
		assert(2 == m.params.size());
		assert_eq("CHARS", m.params[0]);
		assert_eq(five_hundred_chars, m.params[1]);
	}
	TEST_PRINT;

	TEST("Message assemble")
	{
		Message m("", 42, "VERB");
		assert("VERB\r\n" == m.assemble());
	}
	{
		Message m("serv", 42, "VERB");
		assert(":serv VERB\r\n" == m.assemble());
	}
	{
		Message m(42, "VERB", "param1");
		assert("VERB param1\r\n" == m.assemble() || "VERB :param1\r\n" == m.assemble());
	}
	{
		Message m(42, "VERB", "#paramA #paramB");
		assert("VERB :#paramA #paramB\r\n" == m.assemble());
	}
	{
		Message m("serv", 42, "VERB", "param1", "param2");
		assert(":serv VERB param1 param2\r\n" == m.assemble() || ":serv VERB param1 :param2\r\n" == m.assemble());
	}
	{
		Message m("serv", 42, "VERB", "param1", "param two");
		assert(":serv VERB param1 :param two\r\n" == m.assemble());
	}
	{
		Message m(42, "VERB", "param1", "param two");
		assert("VERB param1 :param two\r\n" == m.assemble());
	}
	{
		Message m(42, "VERB", "param1", "    ");
		assert("VERB param1 :    \r\n" == m.assemble());
	}
	TEST_PRINT;

	TEST("Message validation")
	{
		Message m("", 42, "");
		assert(!m.isValid());
	}
	{
		Message m("", 42, "VERB");
		assert(m.isValid());
	}
	{
		Message m("serv", 42, "VERB");
		assert(m.isValid());
	}
	{
		Message m(42, "VERB", "param1");
		assert(m.isValid());
	}
	{
		Message m(42, "VERB", "#paramA #paramB");
		assert(m.isValid());
	}
	{
		Message m("serv", 42, "VERB", "param1", "param2");
		assert(m.isValid());
	}
	{
		Message m("serv", 42, "VERB", "param1", "param two");
		assert(m.isValid());
	}
	{
		Message m(42, "VERB", "param1", "param two");
		assert(m.isValid());
	}
	{
		Message m(42, "VERB", "param1", "    ");
		assert(m.isValid());
	}
	{
		Message m("", 42, " VERB");
		assert(!m.isValid());
	}
	{
		Message m("se rv", 42, "VERB");
		assert(!m.isValid());
	}
	{
		Message m(42, "VERB", "#paramA #paramB", "param C");
		assert(!m.isValid());
	}
	{
		Message m("serv", 42, "VERB", "param one", "param two");
		assert(!m.isValid());
	}
	{
		Message m(42, "VERB", "", "param two");
		assert(!m.isValid());
	}
	TEST_PRINT;
}