#include "tests.hpp"
#include "handlers.hpp"

void tests_part()
{
	TEST("PART handler")
	{
		// Action: User quitte un channel
		// Assert: User retiré de la liste des clients
		State s;
		s.clients[42].status = WELCOMED;
		s.clients[42].nick = "alice";
		s.channels["#test"].client_ids.insert(42);
		Message m(42, "PART #test");
		Responses r;

		partHandler(m, s, r);

		assert(s.channels["#test"].client_ids.count(42) == 0);
	}
	{
		// Action: Dernier user quitte le channel
		// Assert: Channel supprimé complètement
		State s;
		s.clients[42].status = WELCOMED;
		s.clients[42].nick = "alice";
		s.channels["#test"].client_ids.insert(42);
		s.channels["#test"].op_ids.insert(42);
		Message m(42, "PART #test");
		Responses r;

		partHandler(m, s, r);

		assert(s.channels.find("#test") == s.channels.end());
	}
	{
		// Action: User quitte un channel avec d'autres users présents
		// Assert: Channel préservé avec les users restants
		State s;
		s.clients[42].status = WELCOMED;
		s.clients[42].nick = "alice";
		s.clients[43].status = WELCOMED;
		s.clients[43].nick = "bob";
		s.channels["#test"].client_ids.insert(42);
		s.channels["#test"].client_ids.insert(43);
		s.channels["#test"].op_ids.insert(42);
		Message m(42, "PART #test");
		Responses r;

		partHandler(m, s, r);

		assert(s.channels.find("#test") != s.channels.end());
		assert(s.channels["#test"].client_ids.size() == 1);
		assert(s.channels["#test"].op_ids.size() == 0);
	}
	TEST_PRINT;
}