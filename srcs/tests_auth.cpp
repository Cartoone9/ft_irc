#include "tests.hpp"
#include "handlers.hpp"

void tests_auth()
{
	TEST("PASS handler")
	{ // on correct password change status
		State s;
		s.password = "good";
		Responses r;
		passHandler(Message(42, "PASS good"), s, r);
		assert(AUTHENTICATED == s.clients[42].status);
	}
	{ // on wrong password default status
		State s;
		s.password = "good";
		Responses r;
		passHandler(Message(42, "PASS bad"), s, r);
		assert(CONNECTED == s.clients[42].status);
	}
	{ // after PASS/USER/NICK, already registered
		State s;
		s.password = "good";
		s.clients[42].status = WELCOMED;
		Responses r;
		passHandler(Message(42, "PASS good"), s, r);
		assert(WELCOMED == s.clients[42].status);
		assert(!r.empty());
		assert("462" == r.at(0).verb); // ERR_ALREADYREGISTERED (462)
		assert(2 == r.at(0).params.size());
	}
	{ // same on wrong password
		State s;
		s.password = "good";
		s.clients[42].status = WELCOMED;
		Responses r;
		passHandler(Message(42, "PASS bad"), s, r);
		assert(WELCOMED == s.clients[42].status);
		assert(!r.empty());
		assert("462" == r.at(0).verb); // ERR_ALREADYREGISTERED (462)
		assert(2 == r.at(0).params.size());
	}
	{ // last password is used (good then bad)
		State s;
		s.password = "good";
		Responses r;
		passHandler(Message(42, "PASS good"), s, r);
		passHandler(Message(42, "PASS bad"), s, r);
		assert(CONNECTED == s.clients[42].status);
	}
	{ // last password is used (bad then good)
		State s;
		s.password = "good";
		Responses r;
		passHandler(Message(42, "PASS bad"), s, r);
		passHandler(Message(42, "PASS good"), s, r);
		assert(AUTHENTICATED == s.clients[42].status);
	}
	{ // need more params
		State s;
		Responses r;
		passHandler(Message(42, "PASS"), s, r);
		assert(CONNECTED == s.clients[42].status);
		assert(!r.empty());
		assert("461" == r.at(0).verb); // ERR_NEEDMOREPARAMS (461)
		assert(3 == r.at(0).params.size());
		assert_eq("PASS", r.at(0).params.at(1));
	}
	TEST_PRINT;

	TEST("NICK handler")
	{ // nick before welcome
		State s;
		s.clients[42].status = AUTHENTICATED;
		Responses r;
		nickHandler(Message(42, "NICK newnick"), s, r);
		assert_eq("newnick", s.clients[42].nick);
	}
	{ // nick after welcome
		State s;
		s.clients[42].status = WELCOMED;
		s.clients[42].nick = "oldnick";
		Responses r;
		nickHandler(Message(42, "NICK newnick"), s, r);
		assert_eq("newnick", s.clients[42].nick);
	}
	{ // nick in use before welcome
		State s;
		s.clients[1].nick = "samenick";
		s.clients[42].status = AUTHENTICATED;
		Responses r;
		nickHandler(Message(42, "NICK samenick"), s, r);
		assert_eq("", s.clients[42].nick);
		assert(!r.empty());
		assert_eq("433", r.at(0).verb); // ERR_NICKNAMEINUSE (433)
		assert(3 == r.at(0).params.size());
		assert_eq("samenick", r.at(0).params.at(1));
	}
	{ // nick in use after welcome
		State s;
		s.clients[1].nick = "samenick";
		s.clients[42].status = WELCOMED;
		s.clients[42].nick = "oldnick";
		Responses r;
		nickHandler(Message(42, "NICK samenick"), s, r);
		assert_eq("oldnick", s.clients[42].nick);
		assert(!r.empty());
		assert_eq("433", r.at(0).verb); // ERR_NICKNAMEINUSE (433)
		assert(3 == r.at(0).params.size());
		assert_eq("samenick", r.at(0).params.at(1));
	}
	{ // nick starting with `:`
		State s;
		Responses r;
		nickHandler(Message(42, "NICK ::bad"), s, r);
		assert_eq("", s.clients[42].nick);
		assert(!r.empty());
		assert_eq("432", r.at(0).verb); // ERR_ERRONEUSNICKNAME (432)
		assert(3 == r.at(0).params.size());
		assert_eq("*", r.at(0).params.at(0));
	}
	{ // nick starting with `#`
		State s;
		Responses r;
		nickHandler(Message(42, "NICK #bad"), s, r);
		assert_eq("", s.clients[42].nick);
		assert(!r.empty());
		assert_eq("432", r.at(0).verb); // ERR_ERRONEUSNICKNAME (432)
		assert(3 == r.at(0).params.size());
		assert_eq("*", r.at(0).params.at(0));
	}
	{ // nick starting with `&`
		State s;
		Responses r;
		nickHandler(Message(42, "NICK &bad"), s, r);
		assert_eq("", s.clients[42].nick);
		assert(!r.empty());
		assert_eq("432", r.at(0).verb); // ERR_ERRONEUSNICKNAME (432)
		assert(3 == r.at(0).params.size());
		assert_eq("*", r.at(0).params.at(0));
	}
	{ // nick with spaces
		State s;
		Responses r;
		nickHandler(Message(42, "NICK :bad spaces"), s, r);
		assert_eq("", s.clients[42].nick);
		assert(!r.empty());
		assert_eq("432", r.at(0).verb); // ERR_ERRONEUSNICKNAME (432)
		assert(3 == r.at(0).params.size());
		assert_eq("*", r.at(0).params.at(0));
	}
	{ // nick empty
		State s;
		Responses r;
		nickHandler(Message(42, "NICK :"), s, r);
		assert_eq("", s.clients[42].nick);
		assert(!r.empty());
		assert_eq("432", r.at(0).verb); // ERR_ERRONEUSNICKNAME (432)
		assert(3 == r.at(0).params.size());
		assert_eq("*", r.at(0).params.at(0));
	}
	{ // alphanumeric allowed and []{}\\|
		State s;
		Responses r;
		std::string nick = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
						   "abcdefghijklmnopqrstuvwxyz1234567890[]\\|";
		nickHandler(Message(42, "NICK " + nick), s, r);
		assert_eq(nick, s.clients[42].nick);
	}
	{ // starting with number not allowed
		State s;
		Responses r;
		nickHandler(Message(42, "NICK 9bad"), s, r);
		assert_eq("", s.clients[42].nick);
		assert(!r.empty());
		assert_eq("432", r.at(0).verb); // ERR_ERRONEUSNICKNAME (432)
		assert(3 == r.at(0).params.size());
		assert_eq("*", r.at(0).params.at(0));
	}
	TEST_PRINT;

	TEST("USER handler")
	{ // can set username before correct password
		State s;
		Responses r;
		Message m(42, "USER username 0 * realname");
		userHandler(m, s, r);
		assert_eq("~username", s.clients[42].username);
		assert_eq("realname", s.clients[42].realname);
	}
	{ // works before welcome
		State s;
		s.clients[42].status = AUTHENTICATED;
		Responses r;
		Message m(42, "USER username 0 * realname");
		userHandler(m, s, r);
		assert_eq("~username", s.clients[42].username);
		assert_eq("realname", s.clients[42].realname);
	}
	{ // not change after welcome
		State s;
		s.clients[42].status = WELCOMED;
		s.clients[42].username = "~olduser";
		s.clients[42].realname = "oldreal";
		Responses r;
		Message m(42, "USER username 0 * realname");
		userHandler(m, s, r);
		assert_eq("~olduser", s.clients[42].username);
		assert_eq("oldreal", s.clients[42].realname);
		assert(!r.empty());
		assert_eq("462", r.at(0).verb); // ERR_ALREADYREGISTERED (462)
		assert(2 == r.at(0).params.size());
	}
	{ // require parameters
		State s;
		s.clients[42].status = AUTHENTICATED;
		Responses r;
		userHandler(Message(42, "USER"), s, r);
		assert_eq("", s.clients[42].username);
		assert_eq("", s.clients[42].realname);
		assert(!r.empty());
		assert_eq("461", r.at(0).verb); // ERR_NEEDMOREPARAMS (461)
		assert(3 == r.at(0).params.size());
		assert_eq("USER", r.at(0).params.at(1));
	}
	{ // require 4 parameters (not 1)
		State s;
		s.clients[42].status = AUTHENTICATED;
		Responses r;
		userHandler(Message(42, "USER us"), s, r);
		assert_eq("", s.clients[42].username);
		assert_eq("", s.clients[42].realname);
		assert(!r.empty());
		assert_eq("461", r.at(0).verb); // ERR_NEEDMOREPARAMS (461)
		assert(3 == r.at(0).params.size());
		assert_eq("USER", r.at(0).params.at(1));
	}
	{ // require 4 parameters (not 2)
		State s;
		s.clients[42].status = AUTHENTICATED;
		Responses r;
		userHandler(Message(42, "USER us 0"), s, r);
		assert_eq("", s.clients[42].username);
		assert_eq("", s.clients[42].realname);
		assert(!r.empty());
		assert_eq("461", r.at(0).verb); // ERR_NEEDMOREPARAMS (461)
		assert(3 == r.at(0).params.size());
		assert_eq("USER", r.at(0).params.at(1));
	}
	{ // require 4 parameters (not 3)
		State s;
		s.clients[42].status = AUTHENTICATED;
		Responses r;
		userHandler(Message(42, "USER us 0 *"), s, r);
		assert_eq("", s.clients[42].username);
		assert_eq("", s.clients[42].realname);
		assert(!r.empty());
		assert_eq("461", r.at(0).verb); // ERR_NEEDMOREPARAMS (461)
		assert(3 == r.at(0).params.size());
		assert_eq("USER", r.at(0).params.at(1));
	}
	{ // ignore hostname and servername, even if not 0 *
		State s;
		s.clients[42].status = AUTHENTICATED;
		Responses r;
		userHandler(Message(42, "USER us xx yy rn"), s, r);
		assert_eq("~us", s.clients[42].username);
		assert_eq("rn", s.clients[42].realname);
		assert(r.empty());
	}
	TEST_PRINT;

	TEST("QUIT handler")
	{ // removes client authenticated, no broadcast
		Responses r;
		State s;
		s.clients[42].status = AUTHENTICATED;
		s.clients[42].nick = "ric";
		quitHandler(Message(42, "QUIT :rage quit"), s, r);
		assert(s.clients.empty());
		assert(r.empty() || r[0].fd == 42);
	}
	{ // removes client not authenticated, no broadcast
		Responses r;
		State s;
		s.clients[42].status = CONNECTED;
		quitHandler(Message(42, "QUIT :rage quit"), s, r);
		assert(s.clients.empty());
		assert(r.empty() || r[0].fd == 42);
	}
	{ // removes client registered from state and channels
		Responses r;
		State s;
		s.clients[42].status = WELCOMED;
		s.clients[42].nick = "ric";
		s.channels["pingpong"].client_ids.insert(42);
		s.channels["pingpong"].op_ids.insert(42);
		quitHandler(Message(42, "QUIT :rage quit"), s, r);
		assert(s.clients.empty());
		assert(s.channels["pingpong"].client_ids.count(42) == 0);
		assert(s.channels["pingpong"].op_ids.count(42) == 0);
	}
	TEST_PRINT;

	TEST("Authentication workflow")
	{ // can change nick after welcome
		Responses r;
		State s;
		s.password = "goodpass";
		s.clients[42].status = WELCOMED;
		s.clients[42].nick = "oldnick";
		Message m(42, "NICK newnick");
		messageRouter(m, s, r);
		assert_eq("newnick", s.clients[42].nick);
	}
	{ // cannot change username after welcome
		Responses r;
		State s;
		s.password = "goodpass";
		s.clients[42].status = WELCOMED;
		s.clients[42].username = "~oldname";
		Message m(42, "USER newname 0 * rlname");
		messageRouter(m, s, r);
		assert_eq("~oldname", s.clients[42].username);
	}
	{ // can use NICK before PASS
		Responses r;
		State s;
		s.password = "goodpass";
		Message m(42, "NICK newnick");
		messageRouter(m, s, r);
		assert_eq("newnick", s.clients[42].nick);
		assert(CONNECTED == s.clients[42].status);
	}
	{ // can use USER before PASS
		Responses r;
		State s;
		s.password = "goodpass";
		Message m(42, "USER username host server :real name");
		messageRouter(m, s, r);
		assert_eq("~username", s.clients[42].username);
		assert_eq("real name", s.clients[42].realname);
		assert(CONNECTED == s.clients[42].status);
	}
	{ // NICK broadcasts change to channels
		State s;
		s.clients[4].nick = "john";
		s.clients[4].status = WELCOMED;
		s.clients[2].nick = "anon";
		s.clients[2].status = WELCOMED;
		s.channels["team"].client_ids.insert(4);
		s.channels["team"].client_ids.insert(2);
		Responses r;
		messageRouter(Message(2, "NICK tom"), s, r);
		const Message *m;
		m = find_by_fd(r, 2); // to self
		assert(m);
		assert_eq("anon", m->source.substr(0, 4));
		assert_eq("NICK", m->verb);
		assert(!m->params.empty());
		assert_eq("tom", m->params.at(0));
		m = find_by_fd(r, 4); // to others
		assert(m);
		assert_eq("anon", m->source.substr(0, 4));
		assert_eq("NICK", m->verb);
		assert(!m->params.empty());
		assert_eq("tom", m->params.at(0));
	}
	{ // QUIT broadcasts change to channels
		State s;
		s.clients[4].nick = "john";
		s.clients[4].status = WELCOMED;
		s.clients[2].nick = "ric";
		s.clients[2].status = WELCOMED;
		s.channels["pingpong"].client_ids.insert(4);
		s.channels["pingpong"].client_ids.insert(2);
		Responses r;
		messageRouter(Message(2, "QUIT :rage quit"), s, r);
		const Message *m;
		m = find_by_fd(r, 2); // to self
		assert(!m || m->verb == "ERROR");
		m = find_by_fd(r, 4); // to others
		assert(m);
		assert_eq("ric", m->source.substr(0, 3));
		assert_eq("QUIT", m->verb);
		assert(!m->params.empty());
		assert_eq("rage quit", m->params.at(0));
	}
	TEST_PRINT;

	TEST("Welcome handler")
	{ // Welcome after PASS/NICK/USER
		Responses r;
		State s;
		s.clients[42].status = AUTHENTICATED;
		s.clients[42].nick = "tom";
		messageRouter(Message(42, "USER tom 0 * :tom"), s, r);
		assert(r.size() >= 5);
		assert_eq("001", r.at(0).verb);
		assert_eq("002", r.at(1).verb);
		assert_eq("003", r.at(2).verb);
		assert_eq("004", r.at(3).verb);
		assert_eq("005", r.at(4).verb);
	}
	{ // Welcome after PASS/USER/NICK
		Responses r;
		State s;
		s.clients[42].status = AUTHENTICATED;
		s.clients[42].username = "john";
		s.clients[42].realname = "john";
		messageRouter(Message(42, "NICK john"), s, r);
		assert(r.size() >= 5);

		assert_eq("001", r.at(0).verb);
		assert(r.at(0).params.size() >= 2);
		assert_eq("john", r.at(0).params.at(0).substr(0, 4));

		assert_eq("002", r.at(1).verb);
		assert(r.at(1).params.size() >= 2);
		assert_eq("john", r.at(1).params.at(0).substr(0, 4));

		assert_eq("003", r.at(2).verb);
		assert(r.at(2).params.size() >= 2);
		assert_eq("john", r.at(2).params.at(0).substr(0, 4));

		assert_eq("004", r.at(3).verb);
		assert(r.at(3).params.size() >= 6);
		assert_eq("john", r.at(3).params.at(0).substr(0, 4));

		assert_eq("005", r.at(4).verb);
		assert(r.at(4).params.size() >= 3);
		assert_eq("john", r.at(4).params.at(0).substr(0, 4));

		assert(WELCOMED == s.clients[42].status);
	}
	{ // welcome must include MOTD
		Responses r;
		State s;
		s.clients[42].status = AUTHENTICATED;
		s.clients[42].username = "john";
		s.clients[42].realname = "john";
		messageRouter(Message(42, "NICK john"), s, r);
		bool has_motd = false;
		for (Responses::iterator it = r.begin(); it != r.end(); ++it)
		{
			// ERR_NOMOTD (422)
			// RPL_MOTD (372)
			if (it->verb == "422" || it->verb == "372")
			{
				has_motd = true;
				break;
			}
		}
		assert(has_motd);
	}
	TEST_PRINT;
}
