#ifndef HANDLERS_HPP
#define HANDLERS_HPP

#include "Message.struct.hpp"
#include "State.struct.hpp"
#include "dictionary.hpp"

typedef std::vector<Message> Responses;

typedef 
    void message_handler_fn(
        const Message &in, 
        State &state, 
        Responses &out);

// Entry point for the rest of message handlers
void messageRouter(const Message &, State &, Responses &);
void parrot(const Message &, State &, Responses &);

// specific commands

void capHandler(const Message &, State &, Responses &);
void passHandler(const Message &, State &, Responses &);
void nickHandler(const Message &, State &, Responses &);
void userHandler(const Message &, State &, Responses &);
void quitHandler(const Message &, State &, Responses &);

void welcomeHandler(const Message &, State &, Responses &);
void motdHandler(const Message &, State &, Responses &);
void operHandler(const Message &, State &, Responses &);
void modeHandler(const Message &, State &, Responses &);
void pingHandler(const Message &, State &, Responses &);

void partHandler(const Message &, State &, Responses &);
void joinHandler(const Message &, State &, Responses &);
void kickHandler(const Message &, State &, Responses &);
void inviteHandler(const Message &, State &, Responses &);
void topicHandler(const Message &, State &, Responses &);
void namesHandler(const Message &, State &, Responses &);

void privmsgHandler(const Message &, State &, Responses &);
void noticeHandler(const Message &, State &, Responses &);

// bonus
Client createBotClient();
void botRouter(const Message &, State &, Responses &);
void botHandler(const Message &, State &, Responses &); 

// handlers_utils.cpp
size_t		str_to_size(const std::string &str);
std::string size_to_str(size_t);

#endif
