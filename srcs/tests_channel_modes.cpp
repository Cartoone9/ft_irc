#include "tests.hpp"
#include "handlers.hpp"

void tests_channel_modes()
{
	TEST("Channel mode +i")
	{
		{ // can join with -i
			State s;
			s.clients[1].nick = "tom";
			s.clients[1].status = WELCOMED;
			s.clients[2].nick = "ric";
			s.clients[2].status = WELCOMED;
			Responses r;
			messageRouter(Message(1, "JOIN", "#test"), s, r);
			messageRouter(Message(1, "MODE", "#test", "-kil"), s, r);
			messageRouter(Message(2, "JOIN", "#test"), s, r);
			assert(s.channels.count("#test"));
			assert(s.channels["#test"].client_ids.size() == 2);
		}
		{ // cannot join without invite
			State s;
			s.clients[1].nick = "tom";
			s.clients[1].status = WELCOMED;
			s.clients[2].nick = "ric";
			s.clients[2].status = WELCOMED;
			Responses r;
			messageRouter(Message(1, "JOIN", "#test"), s, r);
			messageRouter(Message(1, "MODE", "#test", "-kl+i"), s, r);
			messageRouter(Message(2, "JOIN", "#test"), s, r);
			assert(s.channels.count("#test"));
			assert(s.channels["#test"].client_ids.size() == 1);
			const Message *m = find_by_fd(r, 2);
			assert(m);
			assert_eq("473", m->verb); // ERR_INVITEONLYCHAN (473)
			assert(m->params.size() == 3);
		}
		{ // can join with invite
			State s;
			s.clients[1].nick = "tom";
			s.clients[1].status = WELCOMED;
			s.clients[2].nick = "ric";
			s.clients[2].status = WELCOMED;
			Responses r;
			messageRouter(Message(1, "JOIN #test"), s, r);
			messageRouter(Message(1, "MODE #test -kl+i"), s, r);
			messageRouter(Message(1, "INVITE ric #test"), s, r);
			messageRouter(Message(2, "JOIN #test"), s, r);
			assert(s.channels.count("#test"));
			assert(s.channels["#test"].client_ids.size() == 2);
		}
		{ // can join after nick change
			State s;
			s.clients[1].nick = "tom";
			s.clients[1].status = WELCOMED;
			s.clients[2].nick = "ric";
			s.clients[2].status = WELCOMED;
			Responses r;
			messageRouter(Message(1, "JOIN #test"), s, r);
			messageRouter(Message(1, "MODE #test -kl+i"), s, r);
			messageRouter(Message(1, "INVITE ric #test"), s, r);
			messageRouter(Message(2, "NICK ric"), s, r);
			messageRouter(Message(2, "JOIN #test"), s, r);
			assert(s.channels.count("#test"));
			assert(s.channels["#test"].client_ids.size() == 2);
		}
		{ // cannot join after reconnecting with same fd
			State s;
			s.clients[1].nick = "tom";
			s.clients[1].status = WELCOMED;
			s.clients[2].nick = "ric";
			s.clients[2].status = WELCOMED;
			Responses r;
			messageRouter(Message(1, "JOIN #test"), s, r);
			messageRouter(Message(1, "MODE #test -kl+i"), s, r);
			messageRouter(Message(1, "INVITE ric #test"), s, r);
			messageRouter(Message(2, "QUIT"), s, r);
			s.clients[2].nick = "ric";
			s.clients[2].status = WELCOMED;
			messageRouter(Message(2, "JOIN #test"), s, r);
			assert(s.channels.count("#test"));
			assert(s.channels["#test"].client_ids.size() == 1);
		}
		{ // cannot use invite twice
			State s;
			s.clients[1].nick = "tom";
			s.clients[1].status = WELCOMED;
			s.clients[2].nick = "ric";
			s.clients[2].status = WELCOMED;
			Responses r;
			messageRouter(Message(1, "JOIN #test"), s, r);
			messageRouter(Message(1, "MODE #test -kl+i"), s, r);
			messageRouter(Message(1, "INVITE ric #test"), s, r);
			messageRouter(Message(2, "JOIN #test"), s, r);
			messageRouter(Message(2, "PART #test"), s, r);
			messageRouter(Message(2, "JOIN #test"), s, r);
			assert(s.channels.count("#test"));
			assert(s.channels["#test"].client_ids.size() == 1);
		}
		{ // cannot join after being kicked
			State s;
			s.clients[1].nick = "tom";
			s.clients[1].status = WELCOMED;
			s.clients[2].nick = "ric";
			s.clients[2].status = WELCOMED;
			Responses r;
			messageRouter(Message(1, "JOIN #test"), s, r);
			messageRouter(Message(1, "MODE #test -kl+i"), s, r);
			messageRouter(Message(1, "INVITE ric #test"), s, r);
			messageRouter(Message(2, "JOIN #test"), s, r);
			messageRouter(Message(1, "KICK #test ric"), s, r);
			messageRouter(Message(2, "JOIN #test"), s, r);
			assert(s.channels.count("#test"));
			assert(s.channels["#test"].client_ids.size() == 1);
		}
	}
	TEST_PRINT

	TEST("Channel mode +t")
	{
		{ // query no topic
			State s;
			s.clients[42].nick = "ric";
			s.clients[42].status = WELCOMED;
			Responses r;
			messageRouter(Message(42, "JOIN #test"), s, r);
			messageRouter(Message(42, "TOPIC #test"), s, r);
			assert(r.size() >= 2);
			assert_eq("331", r.back().verb); // RPL_NOTOPIC (331)
			assert(r.back().params.size() >= 3);
		}
		{ // query topic
			State s;
			s.clients[42].nick = "ric";
			s.clients[42].status = WELCOMED;
			Responses r;
			messageRouter(Message(42, "JOIN #test"), s, r);
			messageRouter(Message(42, "TOPIC #test :the topic"), s, r);
			messageRouter(Message(42, "TOPIC #test"), s, r);
			assert(r.size() >= 2);
			assert_eq("332", r.back().verb); // RPL_TOPIC (332)
			assert(r.back().params.size() >= 3);
			assert_eq("the topic", r.back().params[2]);
		}
		{ // mode +t allows op to change topic
			State s;
			s.clients[42].nick = "john";
			s.clients[42].status = WELCOMED;
			Responses r;
			messageRouter(Message(42, "JOIN #test"), s, r);
			messageRouter(Message(42, "MODE #test +t"), s, r);
			messageRouter(Message(42, "TOPIC #test :the topic from op"), s, r);
			messageRouter(Message(42, "TOPIC #test"), s, r);
			assert(r.size() >= 4);
			assert_eq("332", r.back().verb); // RPL_TOPIC (332)
			assert(r.back().params.size() >= 3);
			assert_eq("the topic from op", r.back().params[2]);
		}
		{ // mode +t blocks normal user from changing topic
			State s;
			s.clients[42].nick = "john";
			s.clients[42].status = WELCOMED;
			s.clients[11].nick = "hacker";
			s.clients[11].status = WELCOMED;
			Responses r;
			messageRouter(Message(42, "JOIN #test"), s, r);
			messageRouter(Message(42, "MODE #test +t"), s, r);
			messageRouter(Message(42, "TOPIC #test :the topic from op"), s, r);
			messageRouter(Message(11, "JOIN #test"), s, r);
			messageRouter(Message(11, "TOPIC #test :hacked"), s, r);
			messageRouter(Message(42, "TOPIC #test"), s, r);
			assert(r.size() >= 6);
			assert_eq("332", r.back().verb); // RPL_TOPIC (332)
			assert(r.back().params.size() >= 3);
			assert_eq("the topic from op", r.back().params[2]);
		}
	}
	TEST_PRINT

	TEST("Channel mode +k")
	{
		// Action: Channel op définit le mode +k avec mot de passe "secret"
		// Assert: Le mode +k est activé et la clé est stockée
		{
			State s;
			s.clients[42].status = WELCOMED;
			s.clients[42].nick = "chanop";
			s.channels["#test"].client_ids.insert(42);
			s.channels["#test"].op_ids.insert(42);
			Responses r;

			modeHandler(Message(42, "MODE #test +k secret"), s, r);

			assert(s.channels["#test"].modes.count('k') == 1);
			assert(s.channels["#test"].key == "secret");
		}
		// Action: User essaie de JOIN un channel +k sans mot de passe
		// Assert: JOIN refusé avec ERR_BADCHANNELKEY (475)
		{
			State s;
			s.clients[42].status = WELCOMED;
			s.clients[42].nick = "chanop";
			s.clients[99].status = WELCOMED;
			s.clients[99].nick = "user";
			s.channels["#test"].client_ids.insert(42);
			s.channels["#test"].op_ids.insert(42);
			s.channels["#test"].modes.insert('k');
			s.channels["#test"].key = "secret";
			Responses r;

			joinHandler(Message(99, "JOIN #test"), s, r);

			assert(r.size() > 0);
			assert_eq("475", r[0].verb);
			assert(s.channels["#test"].client_ids.count(99) == 0);
		}
		// Action: User JOIN avec le bon mot de passe
		// Assert: JOIN réussit
		{
			State s;
			s.clients[42].status = WELCOMED;
			s.clients[42].nick = "chanop";
			s.clients[99].status = WELCOMED;
			s.clients[99].nick = "user";
			s.channels["#test"].client_ids.insert(42);
			s.channels["#test"].op_ids.insert(42);
			s.channels["#test"].modes.insert('k');
			s.channels["#test"].key = "secret";
			Responses r;

			joinHandler(Message(99, "JOIN #test secret"), s, r);

			assert(s.channels["#test"].client_ids.count(99) == 1);
		}
		// Action: Channel op enlève le mode +k avec MODE -k
		// Assert: Mode +k désactivé et clé effacée
		{
			State s;
			s.clients[42].status = WELCOMED;
			s.clients[42].nick = "chanop";
			s.channels["#test"].client_ids.insert(42);
			s.channels["#test"].op_ids.insert(42);
			s.channels["#test"].modes.insert('k');
			s.channels["#test"].key = "secret";
			Responses r;

			modeHandler(Message(42, "MODE #test -k"), s, r);

			assert(s.channels["#test"].modes.count('k') == 0);
			assert(s.channels["#test"].key.empty());
		}
	}
	TEST_PRINT

	TEST("Channel mode +l")
	{
		{ // no limit by default
			State s;
			s.clients[1].nick = "tom";
			s.clients[1].status = WELCOMED;
			Responses r;
			messageRouter(Message(1, "JOIN", "#test"), s, r);
			assert(s.channels.count("#test"));
			assert(s.channels["#test"].modes.count('l') == 0);
		}
		{ // can join if no limit
			State s;
			s.clients[1].nick = "tom";
			s.clients[1].status = WELCOMED;
			s.clients[2].nick = "ric";
			s.clients[2].status = WELCOMED;
			Responses r;
			messageRouter(Message(1, "JOIN", "#test"), s, r);
			messageRouter(Message(1, "MODE", "#test", "-lk"), s, r);
			messageRouter(Message(2, "JOIN", "#test"), s, r);
			assert(s.channels.count("#test"));
			assert(s.channels["#test"].client_ids.size() == 2);
		}
		{ // can join big limit
			State s;
			s.clients[1].nick = "tom";
			s.clients[1].status = WELCOMED;
			s.clients[2].nick = "ric";
			s.clients[2].status = WELCOMED;
			Responses r;
			messageRouter(Message(1, "JOIN", "#test"), s, r);
			messageRouter(Message(1, "MODE", "#test", "+l-k", "2"), s, r);
			messageRouter(Message(2, "JOIN", "#test"), s, r);
			assert(s.channels.count("#test"));
			assert(s.channels["#test"].client_ids.size() == 2);
		}
		{ // cannot join limit reached
			State s;
			s.clients[1].nick = "tom";
			s.clients[1].status = WELCOMED;
			s.clients[2].nick = "ric";
			s.clients[2].status = WELCOMED;
			Responses r;
			messageRouter(Message(1, "JOIN", "#test"), s, r);
			messageRouter(Message(1, "MODE", "#test", "+l-k", "1"), s, r);
			messageRouter(Message(2, "JOIN", "#test"), s, r);
			assert(s.channels.count("#test"));
			assert(s.channels["#test"].client_ids.size() == 1);
			const Message *m = find_by_fd(r, 2);
			assert(m);
			assert_eq("471", m->verb); // ERR_CHANNELISFULL (471)
		}
	}
	TEST_PRINT

}
