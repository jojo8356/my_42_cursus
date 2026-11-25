#ifndef SERVER_HPP
#define SERVER_HPP

#include <string>
#include <map>
#include <vector>
#include <set>
#include <sstream>
#include <sys/select.h>
#include "Config.hpp"
#include "Request.hpp"

namespace webserv {

struct ClientState {
    int             fd;
    std::string     in_buf;        // accumulated request bytes
    std::string     out_buf;       // pending response bytes
    Request         req;
    bool            keep_alive;
    bool            headers_done;  // "\r\n\r\n" received
    bool            body_done;
    bool            cgi_running;
    bool            response_sent;
    bool            closed;
    time_t          cgi_start;     // when CGI was forked (for timeout)

    ClientState(int fd_);
    ~ClientState();
};

struct Listener {
    int             fd;
    ServerConfig   *cfg;
};

class Server {
public:
    Config                    config;
    std::vector<Listener>     listeners;
    std::map<int, ClientState*> clients;
    std::map<int, int>        cgi_pipes;  // pipe read fd -> client fd
    std::map<std::string, std::string> sessions;  // session id -> data

    // logging
    void logAccess(int fd, const std::string &method, const std::string &path,
                   int status);
    void logError(const std::string &msg);
    void addCookieHeader(ClientState *c, std::ostringstream &hs);

    Server();
    ~Server();

    bool loadConfig(const std::string &filename);
    void run();
    void initListeners();
    std::size_t getListenerCount() const { return listeners.size(); }

private:
    void buildFdSet(fd_set &readset, fd_set &writeset, int &maxfd);
    void acceptClients(fd_set &readset);
    void readClient(int fd);
    void writeClient(int fd);
    void removeClient(int fd);

    void onClientReadable(int fd);
    void onCgiReadable(int pipe_fd);

    bool haveFullHeaders(const std::string &buf) const;
    void parseAndRoute(ClientState *c);
    void route(ClientState *c);

    void serveStatic(ClientState *c, const std::string &full_path);
    void serveDirectory(ClientState *c, const std::string &full_path,
                        bool autoindex);
    void serveIndex(ClientState *c, const std::string &dir,
                    const std::string &index);
    void serveError(ClientState *c, int status, const std::string &msg);
    void serveRedirect(ClientState *c, int code, const std::string &url);
    std::string resolvePath(ServerConfig *sc, const LocationConfig *loc,
                            const std::string &uri);
    std::string makeListing(const std::string &dir, const std::string &uri);
    void startCGI(ClientState *c, const std::string &script_path,
                  const LocationConfig &loc);
    void sendResponse(ClientState *c, int status, const std::string &body,
                      const std::string &ctype, bool with_body = true);
    void sendHeaders(ClientState *c, int status, const std::string &ctype,
                     std::size_t body_len);
    void addDateHeader(std::string &headers);
    void handleUpload(ClientState *c, const std::string &full_path,
                      const LocationConfig &loc, std::size_t max_body);
};

} // namespace webserv

#endif
