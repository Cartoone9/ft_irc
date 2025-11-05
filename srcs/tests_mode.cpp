#include "tests.hpp"
#include "handlers.hpp"

void tests_oper()
{
	TEST("OPER handler");
	{ // requires user and pass
		State s;
		s.clients[42].status = WELCOMED;
		s.clients[42].nick = "risotto";
		Responses r;
		operHandler(Message(42, "OPER"), s, r);
		operHandler(Message(42, "OPER name"), s, r);
		assert(2 == r.size());
		assert_eq("461", r[0].verb); // ERR_NEEDMOREPARAMS (461)
		assert(3 == r[0].params.size());
		assert_eq("risotto", r[0].params[0].substr(0, 7));
		assert_eq("OPER", r[0].params[1]);
		assert_eq("461", r[1].verb);
	}
	{ // requires registered user
		State s;
		s.oper_name = "goodname";
		s.oper_pass = "goodpass";
		s.clients[42].nick = "pepe";
		s.clients[42].status = AUTHENTICATED;
		Responses r;
		messageRouter(Message(42, "OPER goodname goodpass"), s, r);
		assert(1 == r.size());
		assert_eq("451", r[0].verb); // ERR_NOTREGISTERED (451)
	}
	{ // wrong user or pass
		State s;
		s.oper_name = "goodname";
		s.oper_pass = "goodpass";
		s.clients[42].status = WELCOMED;
		s.clients[42].nick = "ric";
		Responses r;
		operHandler(Message(42, "OPER xxxx yyyyy"), s, r);
		operHandler(Message(42, "OPER xxxx goodpass"), s, r);
		operHandler(Message(42, "OPER goodname xxxx"), s, r);
		assert(3 == r.size());
		assert_eq("464", r[0].verb); // ERR_PASSWDMISMATCH (464)
		assert_eq("464", r[1].verb);
		assert_eq("464", r[2].verb);
		assert(2 == r[2].params.size());
		assert_eq("ric", r[2].params.at(0).substr(0, 3));
	}
	{ // correct user and pass
		State s;
		s.oper_name = "goodname";
		s.oper_pass = "goodpass";
		s.clients[42].status = WELCOMED;
		s.clients[42].nick = "ric";
		Responses r;
		operHandler(Message(42, "OPER goodname goodpass"), s, r);
		assert(r.size() >= 2);
		assert_eq("381", r[0].verb); // RPL_YOUREOPER (381)
		assert(2 == r[0].params.size());
		assert_eq("ric", r[0].params[0].substr(0, 3));
		assert_eq("221", r[1].verb); //  RPL_UMODEIS (221)
		assert(2 == r[1].params.size());
		assert(s.clients[42].modes.count('o')); // +o for local operator
	}
	TEST_PRINT;

	TEST("MODE handler & normal user");
	{ // does not segfault or create phantom clients or channels
		State s;
		s.clients[42].status = WELCOMED;
		s.clients[42].nick = "ric";
		Responses r;
		modeHandler(Message(42, "MODE :"), s, r);
		modeHandler(Message(42, "MODE kasper :"), s, r);
		modeHandler(Message(42, "MODE #graveyard :"), s, r);
		assert(s.clients.size() == 1);
		assert(s.channels.empty());
		assert(s.clients[42].modes.empty());
	}
	{ // user can query own modes
		State s;
		s.clients[42].status = WELCOMED;
		s.clients[42].nick = "ric";
		Responses r;
		modeHandler(Message(42, "MODE", "ric"), s, r);
		assert(1 == r.size());
		assert_eq("221", r[0].verb); // RPL_UMODEIS (221)
		assert(2 == r[0].params.size());
		assert(s.clients[42].modes.empty());
	}
	{ // user can't set mode that is not recognised
		State s;
		s.clients[42].status = WELCOMED;
		s.clients[42].nick = "john";
		Responses r;
		modeHandler(Message(42, "MODE john +p"), s, r);
		assert(!s.clients[42].modes.count('p'));
		assert(r.size() >= 1);
		assert_eq("501", r[0].verb); // ERR_UMODEUNKNOWNFLAG (501)
		assert(r[0].params.size() == 2);
	}
	{ // normal user cannot set +o
		State s;
		s.clients[42].status = WELCOMED;
		s.clients[42].nick = "ric";
		Responses r;
		modeHandler(Message(42, "MODE ric +o"), s, r);
		assert(1 == r.size());
		assert_eq("481", r[0].verb); // ERR_NOPRIVILEGES (481)
		assert(r[0].params.size() == 2);
		assert(!s.clients[42].modes.count('o'));
	}
	{ // no sneaky +o in many modes
		State s;
		s.clients[42].status = WELCOMED;
		s.clients[42].nick = "ric";
		Responses r;
		modeHandler(Message(42, "MODE ric +iow"), s, r);
		assert(1 == r.size());
		assert_eq("481", r[0].verb); // ERR_NOPRIVILEGES (481)
		assert(r[0].params.size() == 2);
		assert(!s.clients[42].modes.count('o'));
		assert(!s.clients[42].modes.count('i'));
		assert(!s.clients[42].modes.count('w'));
	}
	{ // error if target other than self
		State s;
		s.clients[42].status = WELCOMED;
		s.clients[42].nick = "ircop";
		s.clients[42].modes.insert('o');
		s.clients[1].status = WELCOMED;
		s.clients[1].nick = "tom";
		s.clients[1].modes.insert('o');
		Responses r;
		modeHandler(Message(42, "MODE tom -o"), s, r);
		modeHandler(Message(42, "MODE kasper -o"), s, r);
		assert(2 == r.size());
		assert_eq("502", r[0].verb); // ERR_USERSDONTMATCH (502)
		assert(r[0].params.size() == 2);
		assert_eq("401", r[1].verb); // ERR_NOSUCHNICK (401)
		assert(r[1].params.size() == 3);
		assert_eq("kasper", r[1].params[1]);
		assert(s.clients[42].modes.count('o'));
		assert(s.clients[1].modes.count('o'));
		assert(s.clients.size() == 2);
	}
	{ // error if no prefix +/-
		State s;
		s.clients[42].status = WELCOMED;
		s.clients[42].nick = "john";
		Responses r;
		modeHandler(Message(42, "MODE john o"), s, r);
		assert(1 == r.size());
		assert_eq("501", r[0].verb); // ERR_UMODEUNKNOWNFLAG (501)
		assert(r[0].params.size() == 2);
		assert_eq("john", r[0].params[0].substr(0, 4));
		assert(!s.clients[42].modes.count('o')); // mode not set
	}
	{ // normal user cannot set modes on others
		State s;
		s.clients[42].status = WELCOMED;
		s.clients[42].nick = "john";
		s.clients[99].status = WELCOMED;
		s.clients[99].nick = "ric";
		Responses r;
		modeHandler(Message(42, "MODE ric +i"), s, r);
		assert(1 == r.size());
		assert_eq("502", r[0].verb); // ERR_USERSDONTMATCH (502)
		assert(r[0].params.size() == 2);
		assert(!s.clients[99].modes.count('i'));
	}
	{ // user cannot set channel modes without being op
		State s;
		s.clients[42].status = WELCOMED;
		s.clients[42].nick = "tom";
		s.channels["#test"].client_ids.insert(42);
		Responses r;
		modeHandler(Message(42, "MODE #test +i"), s, r);
		assert(1 == r.size());
		assert_eq("482", r[0].verb); // ERR_CHANOPRIVSNEEDED (482)
		assert(r[0].params.size() == 3);
		assert(r[0].params[1] == "#test");
		assert(!s.channels["#test"].modes.count('i'));
	}
	{ // user cannot grant channel op without being op
		State s;
		s.clients[42].status = WELCOMED;
		s.clients[42].nick = "tom";
		s.clients[99].status = WELCOMED;
		s.clients[99].nick = "lisa";
		s.channels["#test"].client_ids.insert(42);
		s.channels["#test"].client_ids.insert(99);
		Responses r;
		modeHandler(Message(42, "MODE #test +o lisa"), s, r);
		assert(1 == r.size());
		assert_eq("482", r[0].verb); // ERR_CHANOPRIVSNEEDED (482)
		assert(r[0].params.size() == 3);
		assert(r[0].params[1] == "#test");
		assert(!s.clients[99].modes.count('o'));
		assert(!s.channels["#test"].op_ids.count(99));
	}
	{ // user cannot remove channel op without being op
		State s;
		s.clients[42].status = WELCOMED;
		s.clients[42].nick = "tom";
		s.clients[99].status = WELCOMED;
		s.clients[99].nick = "lisa";
		s.channels["#test"].client_ids.insert(42);
		s.channels["#test"].client_ids.insert(99);
		s.channels["#test"].op_ids.insert(99);
		Responses r;
		modeHandler(Message(42, "MODE #test -o lisa"), s, r);
		assert(1 == r.size());
		assert_eq("482", r[0].verb); // ERR_CHANOPRIVSNEEDED (482)
		assert(r[0].params.size() == 3);
		assert(r[0].params[1] == "#test");
		assert(!s.clients[99].modes.count('o'));
		assert(s.channels["#test"].op_ids.count(99));
	}
	{ // user cannot set channel modes without being op
		State s;
		s.clients[42].status = WELCOMED;
		s.clients[42].nick = "tom";
		s.channels["#test"].client_ids.insert(42);
		Responses r;
		modeHandler(Message(42, "MODE #test +i"), s, r);
		assert(1 == r.size());
		assert_eq("482", r[0].verb); // ERR_CHANOPRIVSNEEDED (482)
		assert(r[0].params.size() == 3);
		assert(r[0].params[1] == "#test");
		assert(!s.channels["#test"].modes.count('i'));
	}
	{ // user requires WELCOMED status to use MODE
		State s;
		s.clients[42].status = AUTHENTICATED;
		s.clients[42].nick = "john";
		Responses r;
		messageRouter(Message(42, "MODE john +i"), s, r);
		assert(1 == r.size());
		assert_eq("451", r[0].verb); // ERR_NOTREGISTERED (451)
		assert(r[0].params.size() == 2);
	}
	{ // user can query their own modes
		State s;
		s.clients[42].status = WELCOMED;
		s.clients[42].nick = "john";
		s.clients[42].modes.insert('i');
		s.clients[42].modes.insert('w');
		Responses r;
		modeHandler(Message(42, "MODE john"), s, r);
		assert(1 == r.size());
		assert_eq("221", r[0].verb); // RPL_UMODEIS (221)
		assert_eq("+iw", r[0].params.at(1));
		assert(2 == r[0].params.size());
		assert_eq("john", r[0].params[0].substr(0, 4));
	}
	{ // user can query channel modes
		State s;
		s.clients[42].status = WELCOMED;
		s.clients[42].nick = "john";
		s.channels["#test"].client_ids.insert(42);
		s.channels["#test"].modes.insert('i');
		s.channels["#test"].modes.insert('t');
		Responses r;
		modeHandler(Message(42, "MODE #test"), s, r);
		assert(1 == r.size());
		assert_eq("324", r[0].verb); // RPL_CHANNELMODEIS (324)
		assert(r[0].params.size() >= 2);
		assert_eq("#test", r[0].params[0]);
		assert_eq("+it", r[0].params[1]);
	}
	{ // user cannot query channel modes if channel does not exist
		State s;
		s.clients[42].status = WELCOMED;
		s.clients[42].nick = "john";
		Responses r;
		modeHandler(Message(42, "MODE #test"), s, r);
		assert(1 == r.size());
		assert_eq("403", r[0].verb); // RPL_NOSUCHCHANNEL (403)
		assert(3 == r[0].params.size());
		assert_eq("john", r[0].params[0].substr(0, 4));
		assert_eq("#test", r[0].params[1]);
	}
	TEST_PRINT;

	TEST("MODE handler & channel op");
	{ // channel op can grant op to another user
		State s;
		s.clients[42].status = WELCOMED;
		s.clients[42].nick = "chanop";
		s.clients[99].status = WELCOMED;
		s.clients[99].nick = "regular";
		s.channels["#test"].client_ids.insert(42);
		s.channels["#test"].client_ids.insert(99);
		s.channels["#test"].op_ids.insert(42);
		Responses r;
		modeHandler(Message(42, "MODE #test +o regular"), s, r);
		assert(r.size() >= 2);
		assert_eq("MODE", r[0].verb);
		assert_eq("MODE", r[1].verb);
		assert(s.channels["#test"].op_ids.count(99));
		assert(!s.clients[99].modes.count('o'));
	}
	{ // channel op can remove op from another user
		State s;
		s.clients[42].status = WELCOMED;
		s.clients[42].nick = "chanop";
		s.clients[99].status = WELCOMED;
		s.clients[99].nick = "other";
		s.channels["#test"].client_ids.insert(42);
		s.channels["#test"].client_ids.insert(99);
		s.channels["#test"].op_ids.insert(42);
		s.channels["#test"].op_ids.insert(99);
		Responses r;
		modeHandler(Message(42, "MODE #test -o other"), s, r);
		assert(r.size() >= 2);
		assert_eq("MODE", r[0].verb);
		assert_eq("MODE", r[1].verb);
		assert(!s.channels["#test"].op_ids.count(99));
	}
	{ // cannot grant op to user not in channel
		State s;
		s.clients[42].status = WELCOMED;
		s.clients[42].nick = "chanop";
		s.clients[99].status = WELCOMED;
		s.clients[99].nick = "outsider";
		s.channels["#test"].client_ids.insert(42);
		s.channels["#test"].op_ids.insert(42);
		Responses r;
		modeHandler(Message(42, "MODE #test +o outsider"), s, r);
		assert(1 == r.size());
		assert_eq("441", r[0].verb); // ERR_USERNOTINCHANNEL (441)
		assert_eq("outsider", r[0].params[1]);
		assert_eq("#test", r[0].params[2]);
	}
	{ // multiple ops can coexist in channel
		State s;
		s.clients[42].status = WELCOMED;
		s.clients[42].nick = "op1";
		s.clients[99].status = WELCOMED;
		s.clients[99].nick = "op2";
		s.clients[77].status = WELCOMED;
		s.clients[77].nick = "op3";
		s.channels["#test"].client_ids.insert(42);
		s.channels["#test"].client_ids.insert(99);
		s.channels["#test"].client_ids.insert(77);
		s.channels["#test"].op_ids.insert(42);
		Responses r;
		modeHandler(Message(42, "MODE #test +o op2"), s, r);
		modeHandler(Message(99, "MODE #test +o op3"), s, r);
		assert(s.channels["#test"].op_ids.count(42));
		assert(s.channels["#test"].op_ids.count(99));
		assert(s.channels["#test"].op_ids.count(77));
	}
	TEST_PRINT;

	TEST("MODE handler & IRC op");
	{ // IRC operator can query own modes
		State s;
		s.clients[42].status = WELCOMED;
		s.clients[42].nick = "ric";
		s.clients[42].modes.insert('o');
		Responses r;
		modeHandler(Message(42, "MODE", "ric"), s, r);
		assert(1 == r.size());
		assert_eq("221", r[0].verb); // RPL_UMODEIS (221)
		assert(2 == r[0].params.size());
		assert(r[0].params[1].find('o') != std::string::npos);
		assert(s.clients[42].modes.count('o'));
	}
	{ // IRC operator can set channel modes without being channel op
		State s;
		s.clients[42].status = WELCOMED;
		s.clients[42].nick = "ircop";
		s.clients[42].modes.insert('o');
		s.clients[99].status = WELCOMED;
		s.clients[99].nick = "regular";
		s.channels["#test"].client_ids.insert(42);
		s.channels["#test"].client_ids.insert(99);
		s.channels["#test"].op_ids.insert(99);
		Responses r;
		modeHandler(Message(42, "MODE #test +i"), s, r);
		assert(r.size() >= 1);
		assert_eq("MODE", r[0].verb);
		assert(s.channels["#test"].modes.count('i'));
	}
	{ // IRC operator can grant channel op without being channel op
		State s;
		s.clients[42].status = WELCOMED;
		s.clients[42].nick = "ircop";
		s.clients[42].modes.insert('o');
		s.clients[99].status = WELCOMED;
		s.clients[99].nick = "target";
		s.channels["#test"].client_ids.insert(42);
		s.channels["#test"].client_ids.insert(99);
		Responses r;
		modeHandler(Message(42, "MODE #test +o target"), s, r);
		assert(r.size() >= 1);
		assert(s.channels["#test"].op_ids.count(99));
		assert(!s.clients[99].modes.count('o'));
	}
	{ // channel op can remove channel op from IRCop
		State s;
		s.clients[42].status = WELCOMED;
		s.clients[42].nick = "chanop";
		s.clients[99].status = WELCOMED;
		s.clients[99].nick = "other";
		s.clients[99].modes.insert('o');
		s.channels["#test"].client_ids.insert(42);
		s.channels["#test"].client_ids.insert(99);
		s.channels["#test"].op_ids.insert(42);
		s.channels["#test"].op_ids.insert(99);
		Responses r;
		modeHandler(Message(42, "MODE #test -o other"), s, r);
		assert(r.size() >= 1);
		assert_eq("MODE", r[0].verb);
		assert(!s.channels["#test"].op_ids.count(99));
		assert(s.clients[99].modes.count('o'));
	}
	{ // IRC op can remove channel op status
		State s;
		s.clients[42].status = WELCOMED;
		s.clients[42].nick = "ircop";
		s.clients[42].modes.insert('o');
		s.clients[99].status = WELCOMED;
		s.clients[99].nick = "chanop";
		s.channels["#test"].client_ids.insert(42);
		s.channels["#test"].client_ids.insert(99);
		s.channels["#test"].op_ids.insert(99);
		Responses r;
		modeHandler(Message(42, "MODE #test -o chanop"), s, r);
		assert(r.size() >= 1);
		assert(!s.channels["#test"].op_ids.count(99));
	}
	{ // IRC operator can deop themselves
		State s;
		s.clients[42].status = WELCOMED;
		s.clients[42].nick = "ircop";
		s.clients[42].modes.insert('o');
		Responses r;
		modeHandler(Message(42, "MODE ircop -o"), s, r);
		assert(r.size() >= 1);
		assert_eq("221", r[0].verb);				 // RPL_UMODEIS (221)
		assert(s.clients[42].modes.count('o') == 0); // +o removed
	}
	TEST_PRINT;
}
