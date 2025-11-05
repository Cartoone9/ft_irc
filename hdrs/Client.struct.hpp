#ifndef CLIENT_STRUCT_HPP
#define CLIENT_STRUCT_HPP

#include <string>
#include <set>

enum e_client_status
{
    CONNECTED, // default
    AUTHENTICATED,
    WELCOMED
};

struct Client
{
    e_client_status status;
    std::string nick, username, realname;
    std::set<char> modes;

    // is this client an IRCop (not channel op)
    bool isOp() const;

    // When the spect indicates <client> as source of a broadcast
    // irssi expects the full hostmask <nick>!<user>@<host>
    // so it can handle for example the clients in a channel correctly
    std::string hostmask() const;

    // when the spec indicates <client> in a numeric response
    // irssi expects just the nick
    // however if the nick is empty we need to use "*"
    // This converter to string does that
    operator std::string() const;
};

#endif // #ifndef CLIENT_STRUCT_HPP
