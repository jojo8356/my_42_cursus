#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>
#include <vector>

namespace ft_irc {

// Split a string by a delimiter into a vector of tokens.
std::vector<std::string> split(const std::string &s, char delim);

// Remove trailing/leading whitespace and CR/LF.
std::string trim(const std::string &s);

// IRC case mapping (RFC 1459): A-Z -> a-z, [ \ ] ^ ` { | -> ~ ] } ^ _ |
std::string toLowerIrc(const std::string &s);

// Validate a nickname: 1-9 chars, allowed chars only.
bool isValidNick(const std::string &nick);

// Validate a channel name: starts with '#' or '&', 1-50 chars.
bool isValidChannel(const std::string &name);

// Send a raw line (with CRLF) to a socket fd. Returns bytes sent (<0 on error).
int sendLine(int fd, const std::string &line);

// Build an IRC prefix "nick!user@host".
std::string makePrefix(const std::string &nick, const std::string &user,
                       const std::string &host);

// Build a message with optional prefix.
std::string makeMsg(const std::string &prefix, const std::string &cmd,
                    const std::string &params);

} // namespace ft_irc

#endif
