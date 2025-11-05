#include "tests.hpp"
#include "handlers.hpp"

void tests_motd()
{
	TEST("MOTD handler")
	{ // no motd
		State s;
		s.motd.clear();
		Responses r;
		motdHandler(Message(42, "MOTD"), s, r);
		assert(1 == r.size());
		assert_eq("422", r.at(0).verb); // ERR_NOMOTD (422)
		assert(2 == r.at(0).params.size());
	}
	{ // accept target even if it is single server
		State s;
		s.motd.clear();
		Responses r;
		motdHandler(Message(42, "MOTD otherserver.org"), s, r);
		assert(1 == r.size());
		assert_eq("422", r.at(0).verb); // ERR_NOMOTD (422)
	}
	{ // motd responses in correct order
		State s;
		s.motd = "idk";
		Responses r;
		motdHandler(Message(42, "MOTD"), s, r);
		assert(r.size() >= 3);
		assert_eq("375", r.at(0).verb); // RPL_MOTDSTART (375)
		assert_eq("372", r.at(1).verb); // RPL_MOTD (372)
		assert_eq("372", r.at(r.size() - 2).verb);
		assert_eq("376", r.back().verb); // RPL_ENDOFMOTD (376)
	}
	TEST_PRINT
}
