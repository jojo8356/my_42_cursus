#include "Client.hpp"

namespace ft_irc {

Client::Client(int fd_)
    : fd(fd_), pass_ok(false), pass_received(false), has_nick(false),
      has_user(false), registered(false), is_oper(false) {}

Client::~Client() {}

void Client::addOut(const std::string &line) {
    out_buffer += line;
    out_buffer += "\r\n";
}

void Client::clearOut() {
    out_buffer.clear();
}

} // namespace ft_irc
