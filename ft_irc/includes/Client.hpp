#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>

namespace ft_irc {

class Client {
public:
    int             fd;
    std::string     nickname;
    std::string     username;
    std::string     hostname;
    std::string     realname;
    bool            pass_ok;      // PASS received & valid
    bool            pass_received;// PASS command received
    bool            has_nick;     // NICK received
    bool            has_user;     // USER received
    bool            registered;   // fully registered
    bool            is_oper;      // server operator (OPER)
    std::string     in_buffer;    // accumulated read bytes
    std::string     out_buffer;   // pending write bytes

    Client(int fd_);
    ~Client();

    bool isRegistered() const { return registered; }

    // Append a line (with CRLF) to the output buffer.
    void addOut(const std::string &line);
    void clearOut();
};

} // namespace ft_irc

#endif
