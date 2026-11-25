#ifndef SERVER_HPP
#define SERVER_HPP

#include <string>
#include <map>
#include "Client.hpp"
#include "Channel.hpp"

namespace ft_irc {

class Server {
public:
    int                     port;
    int                     listen_fd;
    std::string             password;      // connection password
    std::string             op_password;   // OPER password (bonus)
    std::map<int, Client*>  clients;       // fd -> Client
    std::map<std::string, Channel*> channels; // name -> Channel
    bool                    running;

    Server(int port, const std::string &pw, const std::string &op_pw);
    ~Server();

    // Main poll() event loop.
    void run();

    // --- helpers used by command handlers ---
    Client*    getClient(int fd);
    Channel*   getChannel(const std::string &name);
    std::string nickOf(int fd);
    std::string userHost(int fd);          // "user@host"

    void sendTo(int fd, const std::string &msg);
    void broadcastChannel(Channel *ch, const std::string &msg, int except_fd);
    void addClientToChannel(int fd, Channel *ch, bool is_operator);
    void removeClientFromChannel(int fd, Channel *ch);
    void removeChannelIfEmpty(const std::string &name);
    void clientQuit(int fd, const std::string &reason);

    // If PASS/NICK/USER are all present, complete registration (or reject).
    void tryRegister(int fd);

    // Parse + dispatch a complete command line for a client.
    void dispatch(int fd, const std::string &line);

private:
    void initSocket();
    void acceptClient();
    void readClient(int fd);
    void writeClient(int fd);
    void removeClient(int fd);
};

} // namespace ft_irc

#endif
