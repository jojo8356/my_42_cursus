#include "Server.hpp"
#include "Commands.hpp"
#include "Utils.hpp"
#include <poll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <cerrno>
#include <iostream>
#include <cstdlib>
#include <csignal>
#include <netdb.h>

extern volatile sig_atomic_t g_stop;

namespace ft_irc {

Server::Server(int port, const std::string &pw, const std::string &op_pw)
    : port(port), listen_fd(-1), password(pw), op_password(op_pw),
      running(true) {
    initSocket();
}

Server::~Server() {
    for (std::map<int, Client*>::iterator it = clients.begin();
         it != clients.end(); ++it)
        delete it->second;
    for (std::map<std::string, Channel*>::iterator it = channels.begin();
         it != channels.end(); ++it)
        delete it->second;
    if (listen_fd >= 0)
        close(listen_fd);
}

void Server::initSocket() {
    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0) {
        std::cerr << "socket() failed" << std::endl;
        exit(1);
    }
    int opt = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(static_cast<uint16_t>(this->port));

    if (bind(listen_fd, reinterpret_cast<struct sockaddr *>(&addr),
             sizeof(addr)) < 0) {
        std::cerr << "bind() failed on port " << this->port << std::endl;
        exit(1);
    }
    if (listen(listen_fd, 16) < 0) {
        std::cerr << "listen() failed" << std::endl;
        exit(1);
    }
    // Non-blocking listen socket
    fcntl(listen_fd, F_SETFL, O_NONBLOCK);
}

void Server::acceptClient() {
    struct sockaddr_in client_addr;
    socklen_t len = sizeof(client_addr);
    int fd = accept(listen_fd, reinterpret_cast<struct sockaddr *>(&client_addr),
                    &len);
    if (fd < 0)
        return; // EAGAIN or error — ignore
    fcntl(fd, F_SETFL, O_NONBLOCK);

    Client *c = new Client(fd);
    char host[NI_MAXHOST];
    if (getpeername(fd, reinterpret_cast<struct sockaddr *>(&client_addr),
                    &len) == 0 &&
        getnameinfo(reinterpret_cast<struct sockaddr *>(&client_addr), len,
                    host, sizeof(host), NULL, 0, 0) == 0) {
        c->hostname = host;
    } else {
        c->hostname = "localhost";
    }
    clients[fd] = c;
}

void Server::readClient(int fd) {
    Client *c = getClient(fd);
    if (!c)
        return;
    char buf[4096];
    ssize_t n = recv(fd, buf, sizeof(buf), 0);
    if (n == 0) {
        clientQuit(fd, "Connection closed");
        return;
    }
    if (n < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            return;
        clientQuit(fd, "Connection error");
        return;
    }
    c->in_buffer.append(buf, static_cast<std::size_t>(n));

    // Extract complete lines (delimited by \n)
    std::size_t pos = 0;
    while (true) {
        std::size_t nl = c->in_buffer.find('\n', pos);
        if (nl == std::string::npos)
            break;
        std::string line = c->in_buffer.substr(pos, nl - pos);
        pos = nl + 1;
        line = trim(line);
        if (!line.empty())
            dispatch(fd, line);
    }
    // Keep the remainder (partial line) in the buffer
    c->in_buffer.erase(0, pos);
}

void Server::writeClient(int fd) {
    Client *c = getClient(fd);
    if (!c || c->out_buffer.empty())
        return;
    ssize_t n = send(fd, c->out_buffer.c_str(), c->out_buffer.size(), 0);
    if (n <= 0) {
        if (n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK))
            return; // will retry when writable
        clientQuit(fd, "Write error");
        return;
    }
    c->out_buffer.erase(0, static_cast<std::size_t>(n));
}

void Server::run() {
    while (running) {
        std::vector<pollfd> fds;
        pollfd listen_pfd;
        listen_pfd.fd = listen_fd;
        listen_pfd.events = POLLIN;
        listen_pfd.revents = 0;
        fds.push_back(listen_pfd);

        for (std::map<int, Client*>::iterator it = clients.begin();
             it != clients.end(); ++it) {
            pollfd p;
            p.fd = it->first;
            p.events = POLLIN;
            if (!it->second->out_buffer.empty())
                p.events |= POLLOUT;
            p.revents = 0;
            fds.push_back(p);
        }

        int nready = poll(&fds[0], fds.size(), -1);
        if (g_stop)
            break;
        if (nready < 0) {
            if (errno == EINTR) {
                if (!running || g_stop)
                    break;
                continue;
            }
            break;
        }

        if (nready > 0 && (fds[0].revents & POLLIN))
            acceptClient();

        for (std::size_t i = 1; i < fds.size(); ++i) {
            int fd = fds[i].fd;
            // client may have been removed by a previous operation
            if (clients.find(fd) == clients.end())
                continue;
            if (fds[i].revents & POLLIN)
                readClient(fd);
            if (clients.find(fd) == clients.end())
                continue;
            if (fds[i].revents & POLLOUT)
                writeClient(fd);
        }
    }
}

// --- helpers -----------------------------------------------------------------

Client* Server::getClient(int fd) {
    std::map<int, Client*>::iterator it = clients.find(fd);
    return it == clients.end() ? NULL : it->second;
}

Channel* Server::getChannel(const std::string &name) {
    std::map<std::string, Channel*>::iterator it = channels.find(name);
    return it == channels.end() ? NULL : it->second;
}

std::string Server::nickOf(int fd) {
    Client *c = getClient(fd);
    return c ? c->nickname : std::string("?");
}

std::string Server::userHost(int fd) {
    Client *c = getClient(fd);
    if (!c)
        return "?@?";
    return c->username + "@" + c->hostname;
}

void Server::sendTo(int fd, const std::string &msg) {
    Client *c = getClient(fd);
    if (c)
        c->addOut(msg);
}

void Server::broadcastChannel(Channel *ch, const std::string &msg, int except_fd) {
    if (!ch)
        return;
    std::set<int> members = ch->members;
    for (std::set<int>::iterator it = members.begin(); it != members.end(); ++it) {
        if (*it == except_fd)
            continue;
        sendTo(*it, msg);
    }
}

void Server::addClientToChannel(int fd, Channel *ch, bool is_operator) {
    if (!ch)
        return;
    ch->members.insert(fd);
    if (is_operator)
        ch->operators.insert(fd);
    ch->invited.erase(fd);
}

void Server::removeClientFromChannel(int fd, Channel *ch) {
    if (!ch)
        return;
    ch->members.erase(fd);
    ch->operators.erase(fd);
    ch->invited.erase(fd);
}

void Server::removeChannelIfEmpty(const std::string &name) {
    Channel *ch = getChannel(name);
    if (ch && ch->isEmpty()) {
        channels.erase(name);
        delete ch;
    }
}

void Server::clientQuit(int fd, const std::string &reason) {
    Client *c = getClient(fd);
    if (!c)
        return;
    // Announce to every channel the client was on
    std::vector<std::string> to_announce;
    for (std::map<std::string, Channel*>::iterator it = channels.begin();
         it != channels.end(); ++it) {
        if (it->second->isMember(fd)) {
            std::string msg = makeMsg(makePrefix(c->nickname, c->username,
                                                c->hostname),
                                      "QUIT", reason.empty() ? "Goodbye" : reason);
            broadcastChannel(it->second, msg, fd);
            to_announce.push_back(it->first);
        }
    }
    for (std::size_t i = 0; i < to_announce.size(); ++i)
        removeChannelIfEmpty(to_announce[i]);
    removeClient(fd);
}

void Server::removeClient(int fd) {
    std::map<int, Client*>::iterator it = clients.find(fd);
    if (it == clients.end())
        return;
    delete it->second;
    clients.erase(it);
    close(fd);
}

void Server::tryRegister(int fd) {
    Client *c = getClient(fd);
    if (!c || c->registered)
        return;
    if (c->pass_received && c->has_nick && c->has_user) {
        if (c->pass_ok) {
            c->registered = true;
            sendTo(fd, ":ircserv 001 " + c->nickname + " :Welcome");
            sendTo(fd, ":ircserv 002 " + c->nickname + " :Your host is ircserv");
            sendTo(fd, ":ircserv 003 " + c->nickname + " :Server created");
            sendTo(fd, ":ircserv 004 " + c->nickname + " ircserv 1.0");
            sendTo(fd, ":ircserv 005 " + c->nickname + " :are available modes: i t k o l");
        } else {
            sendTo(fd, ":ircserv 464 " + c->nickname + " :Password incorrect");
        }
    }
}

void Server::dispatch(int fd, const std::string &line) {
    handleLine(*this, fd, line);
}

} // namespace ft_irc
