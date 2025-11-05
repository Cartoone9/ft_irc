#include <cerrno>
#include <cstdlib>

#include "handlers.hpp"
#include "BotLogs.class.hpp"
#include "Rolls.class.hpp"

// --- Helper Functions Declaration ---
static void		autoInviteService(const Message &msg, State &state, Responses &resp);
static bool		checkClankMessageValid(const std::string &message, bool priv_msg);
static bool		parseInfoClankMessage(const std::string &message, bool priv_msg);
static bool		parseDiceClankMessage(const std::string &message, Rolls &rolls);
static Message	diceRoll(const std::string &key, size_t key_value,
		size_t count, const std::string &dest);
static void		vectorToStr(std::vector<int> &ret_vector,
		std::string &ret_str);

// --- Main File Functions ---
Client createBotClient()
{
	std::srand(std::time(0) ^ (getpid()<<16)); // seed for the incoming rolls
	
	Client bot;

	bot.nick = BOT_NICK;
	bot.username = BOT_USERNAME;
	bot.realname = BOT_REALNAME;
	bot.status = WELCOMED; // registered
	bot.modes.insert('o'); // can join any server

	return (bot);
}

void botRouter(const Message &msg, State &s, Responses &r)
{
	// handle as usual
	messageRouter(msg, s, r);

	// handle server responses for the bot
	size_t i = 0;
	while (i < r.size())
	{
		if (r[i].fd == BOT_ID)
			botHandler(r[i], s, r);
		
		// someone joined a channel
		else if (r[i].verb == "JOIN")
			autoInviteService(msg, s, r);
		++i;
	}
}

void botHandler(const Message &msg, State &s, Responses &r)
{
	static BotLogs bot_logs(s.start_time);
	static Client &my = s.clients[BOT_ID];

	// Log message received
	std::string buffer = msg.assemble();
	bot_logs.botLogsBuffer(buffer, true);

	// I am the message source
	// (for example the confirmation that I joined a channel)
	if (msg.source == my.hostmask())
	{
		// I have joined a channel, send greetings
		if (msg.verb == "JOIN")
		{
			Message command(BOT_ID, "NOTICE", msg.params[0], "is ready!");
			bot_logs.botLogsBuffer(command.assemble(), false);
			messageRouter(command, s, r);
		}
	}

	// Someone else is the source
	// (for example, someone left the channel)
	else if (!msg.source.empty())
	{
		// Someone left the channel or quit
		if (msg.verb == "PART")
		{
			const std::string &channel_name = msg.params[0];
			if (s.channels.count(channel_name)) // Check if channel exists
			{
				Channel &channel = s.channels[channel_name];
				if (channel.client_ids.size() == 1 && channel.client_ids.count(BOT_ID))
				{
					// I am alone here, leave
					Message command(BOT_ID, "PART", channel_name);
					bot_logs.botLogsBuffer(command.assemble(), false);
					messageRouter(command, s, r);
				}
			}
		}
		else if (msg.verb == "QUIT")
		{
			std::map<std::string, Channel>::iterator it;
			for (it = s.channels.begin(); it != s.channels.end();)
			{
				std::map<std::string, Channel>::iterator next = it;
				next++;

				if (it->second.client_ids.size() == 1)
				{
					// I am alone here, leave
					Message command(BOT_ID, "PART", it->first);
					bot_logs.botLogsBuffer(command.assemble(), false);
					messageRouter(command, s, r);
				}
				it = next;
			}
		}
		// Someone sent a message to the bot, or the channel
		else if (msg.verb == "PRIVMSG")
		{
			bool	priv_msg = false;
			std::string destination;
			if (msg.params[0] == my.nick)
			{
				size_t excl_pos = msg.source.find('!', 0);
				destination = msg.source.substr(0, excl_pos);
				priv_msg = true;
			}
			else
				destination = msg.params[0];

			const std::string &message = msg.params[1];

			Rolls rolls;
			if (checkClankMessageValid(message, priv_msg))
			{
				if (parseInfoClankMessage(message, priv_msg))
				{
					std::vector<std::string> rolls_info = rolls.info();
					for (size_t i = 0; i < rolls_info.size(); i++)
					{
						Message info_msg(BOT_ID, "NOTICE", destination, rolls_info.at(i));
						bot_logs.botLogsBuffer(info_msg.assemble(), false);
						messageRouter(info_msg, s, r);
					}
				}
				else if (parseDiceClankMessage(message, rolls))
				{
					Message line_top(BOT_ID, "NOTICE", destination, "-----");
					Message command(BOT_ID, "NOTICE", destination,
							REVERSED "List of requested rolls" RESET " => " + rolls.list());
					bot_logs.botLogsBuffer(line_top.assemble(), false);
					bot_logs.botLogsBuffer(command.assemble(), false);
					messageRouter(line_top, s, r);
					messageRouter(command, s, r);

					size_t	n_keys = rolls.getKeysNum();
					for (size_t i = 0; i < n_keys; i++)
					{
						std::string key = rolls.getKey(i);
						size_t		count = rolls.getCount(key);
						if (count > 0)
						{
							int	key_value = std::atoi(key.substr(1, key.size() - 1).c_str());
							Message empty_line(BOT_ID, "NOTICE", destination, "");
							Message dice_roll_ret = diceRoll(key, static_cast<size_t>(key_value),
									count, destination);
							bot_logs.botLogsBuffer("", false);
							bot_logs.botLogsBuffer(dice_roll_ret.assemble(), false);
							messageRouter(empty_line, s, r);
							messageRouter(dice_roll_ret, s, r);
						}
					}
					Message line_bot(BOT_ID, "NOTICE", destination, "-----");
					bot_logs.botLogsBuffer(line_bot.assemble(), false);
					messageRouter(line_bot, s, r);
				}
			}
		}
	}

	// The server sent me a message
	else
	{
		// Someone just connected ot the server
		if (msg.verb == "INVITE")
		{
			// Their Irssi sent "JOIN :" and I am getting this invite
			if (msg.params[1].empty())
			{
			}
			// I am invited to a channel
			else
			{
				Message command(BOT_ID, "JOIN", msg.params[1]);
				bot_logs.botLogsBuffer(command.assemble(), false);
				messageRouter(command, s, r);
			}
		}
	}
}

// --- Helper Functions ---
static void autoInviteService(const Message &msg, State &state, Responses &r)
{
	// assume msg.verb is a JOIN broadcast from the joinHandler
	const std::string &channel_name = msg.params[0];
	Channel &channel = state.channels[channel_name];

	// if the bot is not in the channel, invite
	if (!channel.client_ids.count(BOT_ID))
	{
		r.push_back(Message(BOT_ID, "INVITE", "*", channel_name));
	}
}

static bool	checkClankMessageValid(const std::string &message, bool priv_msg)
{
	if (!priv_msg && message.compare(0, std::string(BOT_NICK).size(), BOT_NICK) != 0)
		return (false);
	return (true);
}

static bool	parseInfoClankMessage(const std::string &message, bool priv_msg)
{
	std::istringstream			iss(message);
	std::string					token;
	std::vector<std::string>	token_vector;

	while (iss >> token)
		token_vector.push_back(token);

	if (priv_msg && token_vector.size() == 1 && token_vector.at(0) == "info")
		return (true);
	else if (!priv_msg && token_vector.size() == 2 && token_vector.at(1) == "info")
		return (true);
	else
		return (false);
}

static bool parseDiceClankMessage(const std::string &message, Rolls &rolls)
{
	std::istringstream	iss(message);
	std::string			token;
	bool				found_dice = false;

	while (iss >> token)
	{
		size_t pos = 0;
		while (pos < token.size())
		{
			// Skip until we find a 'd'
			size_t dpos = token.find('d', pos);
			if (dpos == std::string::npos)
				break;

			// Read digits before 'd'
			size_t start = dpos;
			while (start > 0 && std::isdigit(token[start - 1]))
				start--;
			// Check if the previous digits belong to another dice
			if (start > 0 && token[start - 1] == 'd')
			{
				std::string prev_dice = "d" + token.substr(start, dpos - start);
				for (size_t i = 0; !rolls.getKey(i).empty(); i++)
				{
					if (prev_dice == rolls.getKey(i))
					{
						start = dpos;
						break;
					}
				}
			}

			// Read digits after 'd'
			size_t end = dpos + 1;
			while (end < token.size() && std::isdigit(token[end]))
				end++;

			if (end == dpos + 1) // no digits after 'd'
			{
				pos = dpos + 1;
				continue;
			}

			// Convert multiplier if present
			int multiplier = 1;
			if (start < dpos)
			{
				std::string	sub = token.substr(start, dpos - start);
				const char*	str_ptr = sub.c_str();
				char*		end_ptr = NULL;
				errno = 0; 

				long long	long_val = std::strtol(str_ptr, &end_ptr, 10);

				if (end_ptr == str_ptr || errno == ERANGE || long_val < 0) 
					multiplier = 1;
				else if (long_val > 100)
					multiplier = 100;
				else
					multiplier = static_cast<int>(long_val);

				if (multiplier == 0)
				{
					pos = end;
					continue;
				}
			}


			std::string dice_type = "d" + token.substr(dpos + 1, end - (dpos + 1));

			// Validate that this dice type exists
			for (size_t i = 0; !rolls.getKey(i).empty(); i++)
			{
				if (dice_type == rolls.getKey(i))
				{
					if (!found_dice)
						found_dice = true;
					rolls.addToCount(dice_type, multiplier);
					break;
				}
			}

			// Move past the entire match
			pos = end;
		}
	}

	return (found_dice);
}

static Message	diceRoll(const std::string &key, size_t key_value,
		size_t count, const std::string &dest)
{
	std::vector<int> ret_vector;
	for (size_t i = 0; i < count; i++)
		ret_vector.push_back((std::rand() % key_value) + 1);

	std::string ret_str;
	vectorToStr(ret_vector, ret_str);

	Message command(BOT_ID, "NOTICE", dest, 
			UBOLD + key + ":" + RESET + " " + ret_str);
	return (command);
}

static void	vectorToStr(std::vector<int> &ret_vector,
		std::string &ret_str)
{
	size_t				vector_size = ret_vector.size();
	std::ostringstream	oss;

	for (size_t i = 0; i < vector_size; i++)
	{
		oss << ret_vector[i];
		if (i + 1 < vector_size)
			oss << ", ";
	}
	ret_str = oss.str();
}
