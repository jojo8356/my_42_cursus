#include "Server.hpp"
#include <csignal>
#include <cstdlib>
#include <cstring>
#include <iostream>

volatile sig_atomic_t g_stop = 0;

static void handleSignal(int) {
    g_stop = 1;
}

static void usage(const char *prog) {
    std::cerr << "Usage: " << prog << " <port> <password> [operator_password]"
              << std::endl;
}

int main(int argc, char **argv) {
    if (argc < 3 || argc > 4) {
        usage(argv[0]);
        return 1;
    }

    char *end = NULL;
    long port = std::strtol(argv[1], &end, 10);
    if (end == argv[1] || *end != '\0' || port < 1024 || port > 65535) {
        std::cerr << "Invalid port (must be 1024-65535)" << std::endl;
        usage(argv[0]);
        return 1;
    }

    std::string password = argv[2];
    std::string op_password = (argc >= 4) ? argv[3] : "";

    std::signal(SIGINT, handleSignal);
    std::signal(SIGQUIT, handleSignal);
    // Ignore broken pipe on client disconnects
    std::signal(SIGPIPE, SIG_IGN);

    ft_irc::Server server(static_cast<int>(port), password, op_password);
    std::cout << "ft_irc server listening on port " << port << std::endl;
    server.run();
    std::cout << "ft_irc server stopped." << std::endl;
    return 0;
}
