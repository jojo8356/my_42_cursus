/*
 * mini_db.cpp — 42 Exam Rank 06 (v2025)
 *
 * Implementation of the key-value database server. See mini_db.hpp.
 */

#include "mini_db.hpp"

#include <iostream>
#include <sstream>
#include <fstream>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <vector>
#include <stdexcept>
#include <csignal>
#include <algorithm>

#include <unistd.h>
#include <sys/select.h>

/* ---- Globals used by the SIGINT handler (cannot touch members safely) -- */
static Database *g_db = NULL;
static std::string g_path;

static void sigint_handler(int)
{
    /* Persist the database and terminate cleanly. */
    if (g_db != NULL)
        g_db->save(g_path);
    exit(0);
}

/* ============================ Database ================================= */

void Database::load(const std::string &path)
{
    std::ifstream file(path.c_str());
    if (!file.is_open())
        return;

    std::string key;
    std::string value;
    while (file >> key >> value)
        m_data[key] = value;
}

void Database::save(const std::string &path) const
{
    std::ofstream file(path.c_str());
    if (!file.is_open())
        return;

    std::map<std::string, std::string>::const_iterator it;
    for (it = m_data.begin(); it != m_data.end(); ++it)
        file << it->first << " " << it->second << "\n";
}

void Database::post(const std::string &key, const std::string &value)
{
    m_data[key] = value;
}

bool Database::get(const std::string &key, std::string *value) const
{
    std::map<std::string, std::string>::const_iterator it = m_data.find(key);
    if (it == m_data.end())
        return false;
    if (value != NULL)
        *value = it->second;
    return true;
}

bool Database::del(const std::string &key)
{
    return m_data.erase(key) > 0;
}

/* ============================ Socket ================================== */

void Socket::init(int port)
{
    m_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (m_fd < 0)
        throw std::runtime_error("socket creation failed");

    std::memset(&m_addr, 0, sizeof(m_addr));
    m_addr.sin_family = AF_INET;
    m_addr.sin_addr.s_addr = htonl(2130706433); /* 127.0.0.1 */
    m_addr.sin_port = htons((uint16_t)port);

    if (bind(m_fd, (struct sockaddr *)&m_addr, sizeof(m_addr)) < 0)
        throw std::runtime_error("bind failed");
    if (listen(m_fd, 10) < 0)
        throw std::runtime_error("listen failed");
}

int Socket::accept_client()
{
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    return accept(m_fd, (struct sockaddr *)&client_addr, &addr_len);
}

void Socket::close()
{
    if (m_fd >= 0)
    {
        ::close(m_fd);
        m_fd = -1;
    }
}

/* ============================== Server ============================== */

int Server::m_max_fd = 0;

Server::Server(int port, const std::string &path) : m_path(path)
{
    FD_ZERO(&m_master);
    m_socket.init(port);
    m_db.load(path);
}

void Server::respond(int client_fd, const std::string &text)
{
    const char *data = text.c_str();
    size_t len = text.size();
    size_t sent = 0;
    while (sent < len)
    {
        ssize_t n = send(client_fd, data + sent, len - sent, 0);
        if (n <= 0)
            return;
        sent += (size_t)n;
    }
}

void Server::handle_command(int client_fd, const std::string &line)
{
    /* Tokenize on whitespace: "COMMAND [key] [value]". */
    std::istringstream stream(line);
    std::string command;
    std::string key;
    std::string value;
    stream >> command >> key >> value;

    if (command == "POST")
    {
        if (!key.empty() && !value.empty())
        {
            m_db.post(key, value);
            respond(client_fd, "0\n");
        }
        else
        {
            respond(client_fd, "2\n");
        }
    }
    else if (command == "GET")
    {
        if (key.empty())
        {
            respond(client_fd, "2\n");
        }
        else
        {
            std::string val;
            if (m_db.get(key, &val))
                respond(client_fd, "0 " + val + "\n");
            else
                respond(client_fd, "1\n");
        }
    }
    else if (command == "DELETE")
    {
        if (key.empty())
        {
            respond(client_fd, "2\n");
        }
        else if (m_db.del(key))
            respond(client_fd, "0\n");
        else
            respond(client_fd, "1\n");
    }
    else
    {
        respond(client_fd, "2\n");
    }
}

void Server::process_input(int client_fd)
{
    char chunk[1001];
    ssize_t n = recv(client_fd, chunk, 1000, 0);

    if (n <= 0)
    {
        /* Client disconnected. */
        FD_CLR(client_fd, &m_master);
        m_buffers.erase(client_fd);
        ::close(client_fd);
        return;
    }

    std::string &buf = m_buffers[client_fd];
    buf.append(chunk, (size_t)n);

    /* Extract and handle every complete, newline-terminated command. */
    std::string::size_type pos = 0;
    while (true)
    {
        std::string::size_type nl = buf.find('\n', pos);
        if (nl == std::string::npos)
            break;

        std::string line = buf.substr(pos, nl - pos);
        pos = nl + 1;
        if (!line.empty() && line[line.size() - 1] == '\r')
            line.erase(line.size() - 1);
        handle_command(client_fd, line);
    }

    /* Keep only the trailing incomplete line. */
    if (pos > 0)
        buf = buf.substr(pos);
}

void Server::run()
{
    /* Publish the database for the SIGINT handler. */
    g_db = &m_db;
    g_path = m_path;

    FD_SET(m_socket.fd(), &m_master);
    if (m_socket.fd() > m_max_fd)
        m_max_fd = m_socket.fd();

    std::cout << "ready" << std::endl;

    while (true)
    {
        fd_set read_set = m_master;
        if (select(m_max_fd + 1, &read_set, NULL, NULL, NULL) < 0)
        {
            if (errno == EINTR)
                continue; /* interrupted, retry */
            throw std::runtime_error("select failed");
        }

        for (int fd = 0; fd <= m_max_fd; ++fd)
        {
            if (!FD_ISSET(fd, &read_set))
                continue;

            if (fd == m_socket.fd())
            {
                int client = m_socket.accept_client();
                if (client >= 0)
                {
                    FD_SET(client, &m_master);
                    if (client > m_max_fd)
                        m_max_fd = client;
                    m_buffers[client] = "";
                }
            }
            else
            {
                process_input(fd);
            }
        }
    }
}

/* ============================== main =============================== */

int main(int ac, char **av)
{
    if (ac != 3)
    {
        std::cerr << "Wrong number of arguments\n";
        return EXIT_FAILURE;
    }

    int port = atoi(av[1]);
    std::string path = av[2];

    std::signal(SIGINT, sigint_handler);

    try
    {
        Server server(port, path);
        server.run();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Internal server error\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
