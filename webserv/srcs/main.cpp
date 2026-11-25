#include "Server.hpp"
#include <csignal>
#include <cstdlib>
#include <cstring>
#include <iostream>

namespace webserv {
static Server *g_server = NULL;
static void handleSignal(int) {
    if (g_server)
        _exit(0);
}
}

int main(int argc, char **argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <config.conf>" << std::endl;
        return 1;
    }
    webserv::Server server;
    if (!server.loadConfig(argv[1])) {
        std::cerr << "webserv: failed to load config " << argv[1] << std::endl;
        return 1;
    }
    server.initListeners();
    if (server.getListenerCount() == 0) {
        std::cerr << "webserv: no listeners could be bound" << std::endl;
        return 1;
    }
    std::signal(SIGINT, webserv::handleSignal);
    std::signal(SIGTERM, webserv::handleSignal);
    std::signal(SIGPIPE, SIG_IGN);
    webserv::g_server = &server;
    server.run();
    return 0;
}
