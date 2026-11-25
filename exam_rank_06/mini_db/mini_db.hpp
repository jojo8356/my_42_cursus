/*
 * mini_db.hpp — 42 Exam Rank 06 (v2025)
 *
 * A simple key-value database served over TCP. Clients connect on
 * 127.0.0.1:<port> and exchange commands (POST / GET / DELETE) over
 * persistent connections. The database is persisted to a file on SIGINT
 * and loaded from it on startup.
 */

#ifndef MINI_DB_HPP
#define MINI_DB_HPP

#include <string>
#include <map>

#include <netinet/in.h>

/* --------------------------------------------------------------------- */
/* Database: an in-memory key-value store with persistence.             */
/* --------------------------------------------------------------------- */
class Database
{
public:
    Database() {}

    /* Load key-value pairs from a file (one "key value" pair per line). */
    void load(const std::string &path);

    /* Write every key-value pair to a file (one "key value" pair per line). */
    void save(const std::string &path) const;

    /* POST <key> <value> — add/overwrite an entry. Returns true. */
    void post(const std::string &key, const std::string &value);

    /* GET <key> — returns true and sets *value if the key exists. */
    bool get(const std::string &key, std::string *value) const;

    /* DELETE <key> — returns true if the key existed and was erased. */
    bool del(const std::string &key);

private:
    std::map<std::string, std::string> m_data;
};

/* --------------------------------------------------------------------- */
/* Socket: a thin RAII wrapper around a listening TCP socket.           */
/* --------------------------------------------------------------------- */
class Socket
{
public:
    Socket() : m_fd(-1) {}
    ~Socket() { close(); }

    /* Create + bind + listen on 127.0.0.1:<port>. Throws std::runtime_error
     * on failure. */
    void init(int port);

    /* Accept a new client. Returns the client fd, or -1 on error. */
    int accept_client();

    int fd() const { return m_fd; }
    void close();

private:
    int m_fd;
    struct sockaddr_in m_addr;
};

/* --------------------------------------------------------------------- */
/* Server: the select()-driven event loop.                              */
/* --------------------------------------------------------------------- */
class Server
{
public:
    /* port: TCP port to bind. path: file used for persistence. */
    Server(int port, const std::string &path);

    /* Run the accept/read loop until interrupted (SIGINT). */
    void run();

private:
    void handle_command(int client_fd, const std::string &line);
    void respond(int client_fd, const std::string &text);
    void process_input(int client_fd);

    Socket m_socket;
    Database m_db;
    std::string m_path;

    /* Per-client accumulation buffers (indexed by fd). */
    static int m_max_fd;
    fd_set m_master;
    std::map<int, std::string> m_buffers;
};

#endif /* MINI_DB_HPP */
