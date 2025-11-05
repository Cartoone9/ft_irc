#include "tests.hpp"
#include "handlers.hpp"

void tests_join()
{
	// Action: User rejoint un channel avec un nom valide (#test)
	// Assert: Le channel est créé
	TEST("JOIN handler")
	{
		State s;
		s.clients[42].status = WELCOMED;
		s.clients[42].nick = "alice";
		Message m(42, "JOIN #test");
		Responses r;

		joinHandler(m, s, r);

		assert(s.channels.find("#test") != s.channels.end());
	}
	// Action: User rejoint un channel avec un nom invalide (sans #)
	// Assert: Channel pas créé + réponse d'erreur
	{
		State s;
		s.clients[42].status = WELCOMED;
		s.clients[42].nick = "alice";
		Message m(42, "JOIN test");
		Responses r;

		joinHandler(m, s, r);

		assert(s.channels.find("test") == s.channels.end());
		assert(r.size() > 0);
		assert_eq("403", r[0].verb);
	}
	// Action: User rejoint un nouveau channel
	// Assert: User ajouté à la liste des clients du channel
	{
		State s;
		s.clients[42].status = WELCOMED;
		s.clients[42].nick = "alice";
		Message m(42, "JOIN #test");
		Responses r;

		joinHandler(m, s, r);

		assert(s.channels["#test"].client_ids.count(42) == 1);
		assert(s.channels["#test"].client_ids.size() == 1);
		assert(s.channels["#test"].op_ids.count(42) == 1);
		assert(s.channels["#test"].op_ids.size() == 1);
	}
	// Action: Deuxième user rejoint un channel existant
	// Assert: Les deux users sont dans la liste des clients
	{
		State s;
		s.clients[42].status = WELCOMED;
		s.clients[42].nick = "alice";
		s.clients[43].status = WELCOMED;
		s.clients[43].nick = "bob";
		s.channels["#test"].client_ids.insert(42);
		s.channels["#test"].op_ids.insert(42);
		Message m(43, "JOIN #test");
		Responses r;

		joinHandler(m, s, r);

		assert(s.channels["#test"].client_ids.count(43) == 1);
		assert(s.channels["#test"].client_ids.size() == 2);
		assert(s.channels["#test"].op_ids.count(43) == 0);
		assert(s.channels["#test"].op_ids.size() == 1);
	}
	// Action: User rejoint un channel où il est déjà
	// Assert: Pas de duplication dans la liste des clients
	{
		State s;
		s.clients[42].status = WELCOMED;
		s.clients[42].nick = "alice";
		s.channels["#test"].client_ids.insert(42);
		s.channels["#test"].op_ids.insert(42);
		Message m(42, "JOIN #test");
		Responses r;

		joinHandler(m, s, r);

		assert(s.channels["#test"].client_ids.size() == 1);
		assert(s.channels["#test"].op_ids.size() == 1);
		assert(r.size() == 1);
		assert_eq("443", r.at(0).verb); // ERR_USERONCHANNEL (443)
	}
	{ // JOIN replies JOIN, TOPIC and NAMES
		State s;
		s.clients[42].status = WELCOMED;
		s.clients[42].nick = "alice";
		Message m(42, "JOIN #test");
		Responses r;

		joinHandler(m, s, r);
		assert(r.size() >=4);
		assert_eq("JOIN", r[0].verb);
		assert_eq("331", r[1].verb); //RPL_NOTOPIC (331)
		assert_eq("366", r.back().verb); //RPL_ENDOFNAMES (366)
	}
	TEST_PRINT;
}