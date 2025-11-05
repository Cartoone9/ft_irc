#include "tests.hpp"
#include "handlers.hpp"

void tests_kick()
{
	// Action: User kick un autre user du channel
	// Assert: User target retiré, kicker reste
	TEST("KICK handler")
	{
		State s;
		s.clients[42].status = WELCOMED;
		s.clients[42].nick = "alice";
		s.clients[43].status = WELCOMED;
		s.clients[43].nick = "bob";
		s.channels["#test"].client_ids.insert(42);
		s.channels["#test"].client_ids.insert(43);
		s.channels["#test"].op_ids.insert(42);
		Message m(42, "KICK #test bob");
		Responses r;

		kickHandler(m, s, r);

		assert(s.channels["#test"].client_ids.count(43) == 0);
		assert(s.channels["#test"].client_ids.count(42) == 1);
	}
	// Action: User utilise KICK sans spécifier de target
	// Assert: Erreur paramètres insuffisants
	{
		State s;
		s.clients[42].status = WELCOMED;
		s.clients[42].nick = "alice";
		s.channels["#test"].client_ids.insert(42);
		s.channels["#test"].op_ids.insert(42);
		Message m(42, "KICK #test");
		Responses r;

		kickHandler(m, s, r);

		assert(r.size() > 0);
		assert_eq("461", r[0].verb); // ERR_NEEDMOREPARAMS (461)
		assert(r[0].params.size() == 3);
	}
	{ // user cannot kick without being channel op
		State s;
		s.clients[42].status = WELCOMED;
		s.clients[42].nick = "tom";
		s.clients[99].status = WELCOMED;
		s.clients[99].nick = "jack";
		s.channels["#test"].client_ids.insert(42);
		s.channels["#test"].client_ids.insert(99);
		Responses r;
		kickHandler(Message(42, "KICK #test jack"), s, r);
		assert(1 == r.size());
		assert_eq("482", r[0].verb); // ERR_CHANOPRIVSNEEDED (482)
		assert(r[0].params.size() == 3);
		assert(r[0].params[1] == "#test");
		assert(s.channels["#test"].client_ids.count(99));
	}
	{ // channel op can kick users
		State s;
		s.clients[42].status = WELCOMED;
		s.clients[42].nick = "chanop";
		s.clients[99].status = WELCOMED;
		s.clients[99].nick = "victim";
		s.channels["#test"].client_ids.insert(42);
		s.channels["#test"].client_ids.insert(99);
		s.channels["#test"].op_ids.insert(42);
		Responses r;
		kickHandler(Message(42, "KICK #test victim"), s, r);
		assert(r.size() >= 1);
		assert_eq("KICK", r[0].verb);
		assert(!s.channels["#test"].client_ids.count(99));
	}
	{ // IRC op can kick without being channel op
		State s;
		s.clients[42].status = WELCOMED;
		s.clients[42].nick = "ircop";
		s.clients[42].modes.insert('o');
		s.clients[99].status = WELCOMED;
		s.clients[99].nick = "victim";
		s.channels["#test"].client_ids.insert(42);
		s.channels["#test"].client_ids.insert(99);
		s.channels["#test"].op_ids.insert(99);
		Responses r;
		kickHandler(Message(42, "KICK #test victim"), s, r);
		assert(r.size() >= 1);
		assert_eq("KICK", r[0].verb);
		assert(!s.channels["#test"].client_ids.count(99));
	}
	{ // IRC op can kick from outside the channel
		State s;
		s.clients[42].status = WELCOMED;
		s.clients[42].nick = "ircop";
		s.clients[42].modes.insert('o');
		s.clients[99].status = WELCOMED;
		s.clients[99].nick = "victim";
		s.channels["#test"].client_ids.insert(99);
		s.channels["#test"].op_ids.insert(99);
		Responses r;
		kickHandler(Message(42, "KICK #test victim"), s, r);
		assert(r.size() >= 1);
		assert_eq("KICK", r[0].verb);
		assert(s.channels.empty());
	}
	{ // should show the reason to everyone
		State s;
		s.clients[42].status = WELCOMED;
		s.clients[42].nick = "john";
		s.clients[11].status = WELCOMED;
		s.clients[11].nick = "ric";
		s.clients[22].status = WELCOMED;
		s.clients[22].nick = "tom";
		s.channels["#test"].client_ids.insert(42);
		s.channels["#test"].client_ids.insert(11);
		s.channels["#test"].client_ids.insert(22);
		s.channels["#test"].op_ids.insert(42);
		Responses r;
		kickHandler(Message(42, "KICK #test ric :not human"), s, r);
		const Message *m;
		m = find_by_fd(r, 42);
		assert(m);
		assert_eq("KICK", m->verb);
		assert_eq("#test", m->params.at(0));
		assert_eq("ric", m->params.at(1));
		assert_eq("not human", m->params.at(2));
		m = find_by_fd(r, 22);
		assert(m);
		assert_eq("KICK", m->verb);
		assert_eq("#test", m->params.at(0));
		assert_eq("ric", m->params.at(1));
		assert_eq("not human", m->params.at(2));
		m = find_by_fd(r, 11);
		assert(m);
		assert_eq("KICK", m->verb);
		assert_eq("#test", m->params.at(0));
		assert_eq("ric", m->params.at(1));
		assert_eq("not human", m->params.at(2));
	}
	{ // should show a default reason if not given
		State s;
		s.clients[42].status = WELCOMED;
		s.clients[42].nick = "john";
		s.clients[11].status = WELCOMED;
		s.clients[11].nick = "ric";
		s.channels["#test"].client_ids.insert(42);
		s.channels["#test"].client_ids.insert(11);
		s.channels["#test"].op_ids.insert(42);
		Responses r;
		kickHandler(Message(42, "KICK #test ric"), s, r);
		const Message *m;
		m = find_by_fd(r, 42);
		assert(m);
		assert(!m->params.at(2).empty());
		m = find_by_fd(r, 11);
		assert(m);
		assert(!m->params.at(2).empty());
	}
	TEST_PRINT;
}
