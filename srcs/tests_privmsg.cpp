#include "tests.hpp"
#include "handlers.hpp"

void tests_privmsg()
{
	TEST("PRIVMSG handler")
	{
		// Action: User envoie PRIVMSG à un autre user
		// Assert: Message reçu par le destinataire
		State s;
		s.clients[42].status = WELCOMED;
		s.clients[42].nick = "alice";
		s.clients[43].status = WELCOMED;
		s.clients[43].nick = "bob";
		Message m(42, "PRIVMSG bob :Hello Bob!");
		Responses r;

		privmsgHandler(m, s, r);

		assert(r.size() == 1);
		assert_eq(43, r[0].fd); // message pour bob
		assert_eq("PRIVMSG", r[0].verb);
		assert(!r[0].source.empty());
		assert(r[0].params.size() == 2);
		assert_eq("bob", r[0].params[0]);
		assert_eq("Hello Bob!", r[0].params[1]);
	}
	{
		// Action: User envoie PRIVMSG à un channel
		// Assert: Tous les membres sauf l'émetteur reçoivent le message
		State s;
		s.clients[42].status = WELCOMED;
		s.clients[42].nick = "alice";
		s.clients[43].status = WELCOMED;
		s.clients[43].nick = "bob";
		s.clients[44].status = WELCOMED;
		s.clients[44].nick = "charlie";
		s.channels["#test"].client_ids.insert(42);
		s.channels["#test"].client_ids.insert(43);
		s.channels["#test"].client_ids.insert(44);
		Message m(42, "PRIVMSG #test :Hello everyone!");
		Responses r;

		privmsgHandler(m, s, r);

		assert(r.size() == 2);
		bool alice_got_message = false;
		for (size_t i = 0; i < r.size(); i++)
		{
			if (r[i].fd == 42)
				alice_got_message = true;
		}
		assert(!alice_got_message);
	}
	{
		// Action: PRIVMSG sans destinataire
		// Assert: ERR_NORECIPIENT (411)
		State s;
		s.clients[42].status = WELCOMED;
		s.clients[42].nick = "alice";
		Message m(42, "PRIVMSG");
		Responses r;

		privmsgHandler(m, s, r);

		assert(r.size() == 1);
		assert_eq("411", r[0].verb); // ERR_NORECIPIENT
	}
	{
		// Action: PRIVMSG sans texte
		// Assert: ERR_NOTEXTTOSEND (412)
		State s;
		s.clients[42].status = WELCOMED;
		s.clients[42].nick = "alice";
		Message m(42, "PRIVMSG bob");
		Responses r;

		privmsgHandler(m, s, r);

		assert(r.size() == 1);
		assert_eq("412", r[0].verb); // ERR_NOTEXTTOSEND
	}
	{
		// Action: PRIVMSG vers un nick inexistant
		// Assert: ERR_NOSUCHNICK (401)
		State s;
		s.clients[42].status = WELCOMED;
		s.clients[42].nick = "alice";
		Message m(42, "PRIVMSG unknownuser :Hello");
		Responses r;

		privmsgHandler(m, s, r);

		assert(r.size() == 1);
		assert_eq("401", r[0].verb); // ERR_NOSUCHNICK
		assert_eq("unknownuser", r[0].params[1]);
	}
	{
		// Action: PRIVMSG vers un channel inexistant
		// Assert: ERR_CANNOTSENDTOCHAN (404)
		State s;
		s.clients[42].status = WELCOMED;
		s.clients[42].nick = "alice";
		Message m(42, "PRIVMSG #nonexistent :Hello");
		Responses r;

		privmsgHandler(m, s, r);

		assert(r.size() == 1);
		assert_eq("404", r[0].verb); // ERR_CANNOTSENDTOCHAN (404)
	}
	{
		// Action: PRIVMSG vers un channel où le user n'est pas membre
		// Assert: ERR_CANNOTSENDTOCHAN (404)
		State s;
		s.clients[42].status = WELCOMED;
		s.clients[42].nick = "alice";
		s.clients[43].status = WELCOMED;
		s.clients[43].nick = "bob";
		s.channels["#test"].client_ids.insert(43);
		Message m(42, "PRIVMSG #test :Hello");
		Responses r;

		privmsgHandler(m, s, r);

		assert(r.size() == 1);
		assert_eq("404", r[0].verb); // ERR_CANNOTSENDTOCHAN
	}
	{
		// Action: Vérifier le format hostmask dans le message
		// Assert: source contient nick!user@host
		State s;
		s.clients[42].status = WELCOMED;
		s.clients[42].nick = "alice";
		s.clients[42].username = "~alice";
		s.clients[43].status = WELCOMED;
		s.clients[43].nick = "bob";
		Message m(42, "PRIVMSG bob :Hello");
		Responses r;

		privmsgHandler(m, s, r);

		assert(r.size() == 1);
		assert(r[0].source.find("alice") != std::string::npos);
		assert(r[0].source.find("!") != std::string::npos);
		assert(r[0].source.find("@") != std::string::npos);
	}
	{
		// Action: PRIVMSG vers un channel avec seulement l'émetteur
		// Assert: Aucun message envoyé (pas d'écho)
		State s;
		s.clients[42].status = WELCOMED;
		s.clients[42].nick = "alice";
		s.channels["#test"].client_ids.insert(42);
		Message m(42, "PRIVMSG #test :Hello");
		Responses r;

		privmsgHandler(m, s, r);

		assert(r.size() == 0); // alice seule, pas d'écho
	}
	{
		// Action: Vérifier que tous les membres du channel reçoivent le message
		// Assert: Chaque membre (sauf émetteur) a exactement le même message
		State s;
		s.clients[42].status = WELCOMED;
		s.clients[42].nick = "alice";
		s.clients[43].status = WELCOMED;
		s.clients[43].nick = "bob";
		s.clients[44].status = WELCOMED;
		s.clients[44].nick = "charlie";
		s.channels["#test"].client_ids.insert(42);
		s.channels["#test"].client_ids.insert(43);
		s.channels["#test"].client_ids.insert(44);
		Message m(42, "PRIVMSG #test :Broadcast test");
		Responses r;

		privmsgHandler(m, s, r);

		assert(r.size() == 2);
		// Vérifier que bob et charlie ont reçu
		bool bob_received = false;
		bool charlie_received = false;
		for (size_t i = 0; i < r.size(); i++)
		{
			if (r[i].fd == 43)
				bob_received = true;
			if (r[i].fd == 44)
				charlie_received = true;
			// Vérifier le contenu identique
			assert_eq("PRIVMSG", r[i].verb);
			assert_eq("#test", r[i].params[0]);
			assert_eq("Broadcast test", r[i].params[1]);
		}
		assert(bob_received && charlie_received);
	}
	TEST_PRINT;

	TEST("NOTICE handler")
	{
		// Action: User envoie NOTICE à un autre user
		// Assert: Message reçu par le destinataire (identique à PRIVMSG)
		State s;
		s.clients[42].status = WELCOMED;
		s.clients[42].nick = "alice";
		s.clients[43].status = WELCOMED;
		s.clients[43].nick = "bob";
		Message m(42, "NOTICE bob :Hello Bob!");
		Responses r;

		noticeHandler(m, s, r);

		assert(r.size() == 1);
		assert_eq(43, r[0].fd);
		assert_eq("NOTICE", r[0].verb);
	}
	{
		// Action: NOTICE sans destinataire
		// Assert: Pas de réponse d'erreur (silencieux)
		State s;
		s.clients[42].status = WELCOMED;
		s.clients[42].nick = "alice";
		Message m(42, "NOTICE");
		Responses r;

		noticeHandler(m, s, r);

		assert(r.size() == 0); // NOTICE ne répond jamais avec des erreurs
	}
	{
		// Action: NOTICE vers un nick inexistant
		// Assert: Pas de réponse d'erreur (silencieux)
		State s;
		s.clients[42].status = WELCOMED;
		s.clients[42].nick = "alice";
		Message m(42, "NOTICE unknownuser :Hello");
		Responses r;

		noticeHandler(m, s, r);

		assert(r.size() == 0); // NOTICE est silencieux
	}
	{
		// Action: NOTICE vers un channel où le user n'est pas membre
		// Assert: Pas de réponse d'erreur (silencieux)
		State s;
		s.clients[42].status = WELCOMED;
		s.clients[42].nick = "alice";
		s.clients[43].status = WELCOMED;
		s.clients[43].nick = "bob";
		s.channels["#test"].client_ids.insert(43);
		Message m(42, "NOTICE #test :Hello");
		Responses r;

		noticeHandler(m, s, r);

		assert(r.size() == 0); // NOTICE est silencieux
	}
	TEST_PRINT;
}
