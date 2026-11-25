#ifndef COMMANDS_HPP
#define COMMANDS_HPP

#include <string>
#include "Server.hpp"

namespace ft_irc {

// Parse a complete command line and dispatch it to the appropriate handler.
void handleLine(Server &srv, int fd, const std::string &line);

} // namespace ft_irc

#endif
