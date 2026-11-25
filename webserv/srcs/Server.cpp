#include "Server.hpp"
#include "Utils.hpp"
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <cerrno>
#include <ctime>
#include <cstdlib>
#include <sstream>
#include <iostream>
#include <dirent.h>
#include <sys/stat.h>

namespace webserv {

static std::string to_str(long n) {
    std::ostringstream os;
    os << n;
    return os.str();
}

ClientState::ClientState(int fd_)
    : fd(fd_), keep_alive(false), headers_done(false), body_done(false),
      cgi_running(false), response_sent(false), closed(false), cgi_start(0) {}

ClientState::~ClientState() {}

Server::Server() {}

void Server::logAccess(int fd, const std::string &method, const std::string &path,
                       int status) {
    std::ofstream f("access.log", std::ios::app);
    if (f.is_open())
        f << "fd=" << fd << " " << method << " " << path << " " << status << "\n";
}

void Server::logError(const std::string &msg) {
    std::ofstream f("error.log", std::ios::app);
    if (f.is_open())
        f << msg << "\n";
}

void Server::addCookieHeader(ClientState *c, std::ostringstream &hs) {
    // Set a session cookie if not already present
    if (c->req.getHeader("cookie").find("webservsid=") == std::string::npos) {
        char sid[64];
        snprintf(sid, sizeof(sid), "%ld", (long)time(NULL));
        sessions[std::string(sid)] = "active";
        hs << "Set-Cookie: webservsid=" << sid << "; Path=/\r\n";
    }
}

Server::~Server() {
    for (std::map<int, ClientState*>::iterator it = clients.begin();
         it != clients.end(); ++it)
        delete it->second;
    for (std::size_t i = 0; i < listeners.size(); ++i)
        if (listeners[i].fd >= 0)
            close(listeners[i].fd);
}

bool Server::loadConfig(const std::string &filename) {
    return config.load(filename);
}

void Server::initListeners() {
    for (std::size_t i = 0; i < config.servers.size(); ++i) {
        ServerConfig &sc = config.servers[i];
        int fd = socket(AF_INET, SOCK_STREAM, 0);
        if (fd < 0)
            continue;
        int opt = 1;
        setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
        struct sockaddr_in addr;
        std::memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(static_cast<uint16_t>(sc.listen_port));
        if (sc.listen_host.empty())
            addr.sin_addr.s_addr = INADDR_ANY;
        else
            inet_pton(AF_INET, sc.listen_host.c_str(), &addr.sin_addr);
        if (bind(fd, reinterpret_cast<struct sockaddr *>(&addr),
                 sizeof(addr)) < 0) {
            std::cerr << "webserv: bind failed on port " << sc.listen_port
                      << std::endl;
            close(fd);
            continue;
        }
        if (listen(fd, 64) < 0) {
            close(fd);
            continue;
        }
        fcntl(fd, F_SETFL, O_NONBLOCK);
        Listener l;
        l.fd = fd;
        l.cfg = &sc;
        listeners.push_back(l);
    }
}

void Server::buildFdSet(fd_set &readset, fd_set &writeset, int &maxfd) {
    FD_ZERO(&readset);
    FD_ZERO(&writeset);
    maxfd = -1;
    for (std::size_t i = 0; i < listeners.size(); ++i) {
        FD_SET(listeners[i].fd, &readset);
        if (listeners[i].fd > maxfd) maxfd = listeners[i].fd;
    }
    for (std::map<int, ClientState*>::iterator it = clients.begin();
         it != clients.end(); ++it) {
        if (it->second->closed)
            continue;
        if (it->second == NULL)
            continue;
        FD_SET(it->first, &readset);
        if (!it->second->out_buf.empty() && !it->second->cgi_running)
            FD_SET(it->first, &writeset);
        if (it->first > maxfd) maxfd = it->first;
    }
    for (std::map<int, int>::iterator it = cgi_pipes.begin();
         it != cgi_pipes.end(); ++it) {
        FD_SET(it->first, &readset);
        if (it->first > maxfd) maxfd = it->first;
    }
}

void Server::acceptClients(fd_set &readset) {
    for (std::size_t i = 0; i < listeners.size(); ++i) {
        if (!FD_ISSET(listeners[i].fd, &readset))
            continue;
        while (true) {
            struct sockaddr_in addr;
            socklen_t len = sizeof(addr);
            int fd = accept(listeners[i].fd,
                            reinterpret_cast<struct sockaddr *>(&addr), &len);
            if (fd < 0)
                break;
            fcntl(fd, F_SETFL, O_NONBLOCK);
            clients[fd] = new ClientState(fd);
        }
    }
}

bool Server::haveFullHeaders(const std::string &buf) const {
    return buf.find("\r\n\r\n") != std::string::npos;
}

static std::string dechunk(const std::string &data);

void Server::onClientReadable(int fd) {
    ClientState *c = clients[fd];
    if (!c || c->closed)
        return;
    char buf[8192];
    ssize_t n = recv(fd, buf, sizeof(buf), 0);
    if (n == 0) { removeClient(fd); return; }
    if (n < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            return;
        removeClient(fd);
        return;
    }
    c->in_buf.append(buf, static_cast<std::size_t>(n));

    if (!c->headers_done) {
        if (!haveFullHeaders(c->in_buf))
            return;
        std::size_t he = c->in_buf.find("\r\n\r\n");
        // try to parse the request line + headers
        Request tmp;
        std::string head = c->in_buf.substr(0, he + 2);
        std::size_t nl = head.find("\r\n");
        std::string rl = (nl == std::string::npos) ? head : head.substr(0, nl);
        std::istringstream iss(rl);
        std::string mp, fullpath, ver;
        iss >> mp >> fullpath >> ver;
        if (mp.empty() || fullpath.empty()) {
            serveError(c, 400, "Bad Request");
            return;
        }
        c->req.method = mp;
        c->req.path = urlDecode(fullpath.substr(0, fullpath.find('?')));
        c->req.query = fullpath.find('?') != std::string::npos
                           ? fullpath.substr(fullpath.find('?') + 1)
                           : "";
        c->req.version = ver;
        // parse headers
        std::size_t pos = nl + 2;
        while (pos < head.size()) {
            std::size_t eol = head.find("\r\n", pos);
            if (eol == std::string::npos) break;
            std::string line = head.substr(pos, eol - pos);
            std::size_t colon = line.find(':');
            if (colon != std::string::npos) {
                std::string key = trim(line.substr(0, colon));
                std::string val = trim(line.substr(colon + 1));
                if (!key.empty())
                    c->req.headers[toLower(key)] = val;
            }
            pos = eol + 2;
        }
        c->req.raw = c->in_buf.substr(0, he + 4);
        c->headers_done = true;
    }

    // determine body_done (re-checked on every recv)
    if (!c->body_done && !c->cgi_running) {
        if (c->req.isChunked()) {
            c->body_done = true;
        } else {
            std::size_t cl = c->req.contentLength();
            std::size_t he = c->in_buf.find("\r\n\r\n");
            if (he != std::string::npos) {
                std::size_t body_start = he + 4;
                if (c->in_buf.size() - body_start >= cl)
                    c->body_done = true;
            }
        }
    }

    if (c->headers_done && c->body_done && !c->cgi_running) {
        // finalize body
        if (c->req.isChunked()) {
            std::size_t he = c->in_buf.find("\r\n\r\n");
            c->req.body = c->in_buf.substr(he + 4);
            c->req.body = dechunk(c->req.body);
        } else {
            std::size_t he = c->in_buf.find("\r\n\r\n");
            std::size_t cl = c->req.contentLength();
            c->req.body = c->in_buf.substr(he + 4, cl);
        }
        parseAndRoute(c);
    }
}

// Simple chunked decoder
std::string dechunk(const std::string &data) {
    std::string out;
    std::size_t pos = 0;
    while (pos < data.size()) {
        std::size_t eol = data.find("\r\n", pos);
        if (eol == std::string::npos) break;
        std::string size_str = data.substr(pos, eol - pos);
        std::size_t chunk_size = static_cast<std::size_t>(
            std::strtoul(size_str.c_str(), NULL, 16));
        if (chunk_size == 0)
            break;
        pos = eol + 2;
        if (pos + chunk_size > data.size())
            chunk_size = data.size() - pos;
        out += data.substr(pos, chunk_size);
        pos += chunk_size + 2;
    }
    return out;
}

void Server::parseAndRoute(ClientState *c) {
    route(c);
}

void Server::route(ClientState *c) {
    // pick the server block bound to the listener that accepted the client
    ServerConfig *sc = NULL;
    if (!listeners.empty()) {
        for (std::size_t i = 0; i < config.servers.size(); ++i) {
            if (config.servers[i].listen_port ==
                    listeners[0].cfg->listen_port) {
                sc = &config.servers[i];
                break;
            }
        }
    }
    if (!sc) sc = config.servers.empty() ? NULL : &config.servers[0];
    if (!sc) { serveError(c, 500, "No server configured"); return; }

    const LocationConfig *loc = sc->findLocation(c->req.path);

    // unknown method -> 501
    if (c->req.method != "GET" && c->req.method != "POST" &&
        c->req.method != "DELETE" && c->req.method != "HEAD") {
        serveError(c, 501, "Not Implemented");
        return;
    }
    // method check
    std::set<std::string> allowed;
    if (loc && !loc->methods.empty())
        allowed = loc->methods;
    else if (loc && !loc->cgi.empty()) {
        allowed.insert("get");
        allowed.insert("head");
        allowed.insert("post");
    } else {
        allowed.insert("get");
        allowed.insert("head");
    }
    if (allowed.find(toLower(c->req.method)) == allowed.end()) {
        serveError(c, 405, "Method Not Allowed");
        return;
    }

    // redirect
    if (loc && loc->redirect_code != 0) {
        serveRedirect(c, loc->redirect_code, loc->redirect_url);
        return;
    }

    // CGI
    if (loc && !loc->cgi.empty()) {
        std::string ext = getExtension(c->req.path);
        if (loc->cgi.count(ext)) {
            std::string script = resolvePath(sc, loc, c->req.path);
            startCGI(c, script, *loc);
            return;
        }
    }

    // upload (POST to upload location)
    if (loc && !loc->upload_store.empty() && c->req.method == "POST") {
        handleUpload(c, resolvePath(sc, loc, c->req.path), *loc,
                     sc->client_max_body_size);
        return;
    }

    // DELETE
    if (c->req.method == "DELETE") {
        std::string full = resolvePath(sc, loc, c->req.path);
        if (unlink(full.c_str()) == 0)
            sendResponse(c, 204, "", "text/plain", false);
        else
            serveError(c, 404, "Not Found");
        return;
    }

    // static
    std::string full = resolvePath(sc, loc, c->req.path);
    if (isDir(full)) {
        bool ai = loc ? loc->autoindex : false;
        serveDirectory(c, full, ai);
    } else if (isFile(full)) {
        serveStatic(c, full);
    } else {
        serveError(c, 404, "Not Found");
    }
}

std::string Server::resolvePath(ServerConfig *sc, const LocationConfig *loc,
                                const std::string &uri) {
    std::string root = sc->root;
    if (loc) {
        if (loc->alias) {
            std::size_t plen = loc->prefix.size();
            std::string sub = (uri.size() >= plen) ? uri.substr(plen) : "";
            root = loc->root;
            return root + (sub.empty() ? "" : sub);
        }
        if (!loc->root.empty())
            root = loc->root;
    }
    return root + uri;
}

void Server::serveStatic(ClientState *c, const std::string &path) {
    std::ifstream f(path.c_str(), std::ios::binary);
    if (!f.is_open()) { serveError(c, 404, "Not Found"); return; }
    std::stringstream ss;
    ss << f.rdbuf();
    std::string content = ss.str();
    std::string ctype = contentTypeFor(path);
    sendResponse(c, 200, content, ctype, c->req.method != "HEAD");
}

void Server::serveDirectory(ClientState *c, const std::string &dir, bool autoindex) {
    std::string index = c->req.headers.empty() ? "" : "";
    // try index files
    const LocationConfig *loc = NULL;
    for (std::size_t i = 0; i < config.servers.size(); ++i) {
        loc = config.servers[i].findLocation(c->req.path);
        if (loc) break;
    }
    std::string idx = (loc && !loc->index.empty()) ? loc->index : "index.html";
    std::vector<std::string> idxs = split(idx, ' ');
    for (std::size_t i = 0; i < idxs.size(); ++i) {
        std::string cand = dir + idxs[i];
        if (isFile(cand)) {
            serveStatic(c, cand);
            return;
        }
    }
    if (autoindex) {
        std::string listing = makeListing(dir, c->req.path);
        sendResponse(c, 200, listing, "text/html", c->req.method != "HEAD");
        return;
    }
    serveError(c, 404, "Not Found");
}

std::string Server::makeListing(const std::string &dir, const std::string &uri) {
    std::string html = "<html><head><title>Index of " + uri +
                       "</title></head><body><h1>Index of " + uri +
                       "</h1><ul>";
    DIR *d = opendir(dir.c_str());
    if (d) {
        struct dirent *e;
        while ((e = readdir(d)) != NULL) {
            if (std::strcmp(e->d_name, ".") == 0 ||
                std::strcmp(e->d_name, "..") == 0)
                continue;
            html += "<li><a href=\"" + std::string(e->d_name) + "\">" +
                    e->d_name + "</a></li>";
        }
        closedir(d);
    }
    html += "</ul></body></html>";
    return html;
}

void Server::serveError(ClientState *c, int status, const std::string &msg) {
    ServerConfig *sc = listeners.empty() ? NULL : listeners[0].cfg;
    std::string body = "<html><head><title>" + to_str(status) +
                       " " + msg + "</title></head><body><h1>" +
                       to_str(status) + " " + msg + "</h1></body></html>";
    // try custom error page
    if (sc) {
        std::map<int, std::string>::iterator it = sc->error_pages.find(status);
        if (it != sc->error_pages.end()) {
            std::string errpath = resolvePath(sc, NULL, it->second);
            std::ifstream f(errpath.c_str(), std::ios::binary);
            if (f.is_open()) {
                std::stringstream ss;
                ss << f.rdbuf();
                body = ss.str();
            }
        }
    }
    sendResponse(c, status, body, "text/html", c->req.method != "HEAD");
    logError("Error " + to_str(status) + " " + msg);
}

void Server::serveRedirect(ClientState *c, int code, const std::string &url) {
    std::ostringstream hs;
    hs << "HTTP/1.1 " << code << " "
       << (code == 301 ? "Moved Permanently" : "Found") << "\r\n";
    std::string dh;
    addDateHeader(dh);
    hs << dh;
    hs << "Location: " << url << "\r\n";
    hs << "Content-Length: 0\r\n\r\n";
    c->out_buf += hs.str();
}

void Server::addDateHeader(std::string &headers) {
    char tbuf[128];
    time_t now = time(NULL);
    struct tm tmv;
    gmtime_r(&now, &tmv);
    strftime(tbuf, sizeof(tbuf), "%a, %d %b %Y %H:%M:%S GMT", &tmv);
    headers += "Date: ";
    headers += tbuf;
    headers += "\r\n";
}

void Server::sendHeaders(ClientState *c, int status, const std::string &ctype,
                         std::size_t body_len) {
    std::ostringstream hs;
    const char *reason = status == 200 ? "OK"
                         : status == 201 ? "Created"
                         : status == 204 ? "No Content"
                         : status == 301 ? "Moved Permanently"
                         : status == 302 ? "Found"
                         : status == 400 ? "Bad Request"
                         : status == 404 ? "Not Found"
                         : status == 405 ? "Method Not Allowed"
                         : status == 413 ? "Payload Too Large"
                         : status == 500 ? "Internal Server Error"
                         : status == 501 ? "Not Implemented"
                         : status == 502 ? "Bad Gateway"
                         : "Unknown";
    hs << "HTTP/1.1 " << status << " " << reason << "\r\n";
    std::string dh;
    addDateHeader(dh);
    hs << dh;
    hs << "Server: webserv/1.0\r\n";
    addCookieHeader(c, hs);
    if (!ctype.empty())
        hs << "Content-Type: " << ctype << "\r\n";
    if (body_len > 0 || status == 204)
        hs << "Content-Length: " << body_len << "\r\n";
    hs << "Connection: " << (c->keep_alive ? "keep-alive" : "close") << "\r\n\r\n";
    c->out_buf += hs.str();
}

void Server::sendResponse(ClientState *c, int status, const std::string &body,
                          const std::string &ctype, bool with_body) {
    sendHeaders(c, status, ctype, body.size());
    if (with_body)
        c->out_buf += body;
    c->response_sent = true;
    logAccess(c->fd, c->req.method, c->req.path, status);
    // determine keep-alive
    std::string conn = c->req.getHeader("connection");
    c->keep_alive = (c->req.version == "HTTP/1.1" &&
                     (conn.empty() || toLower(conn) == "keep-alive")) ||
                    toLower(conn) == "keep-alive";
    if (!c->keep_alive) {
        c->closed = true;
        // will be removed after out_buf drained
    }
}

void Server::handleUpload(ClientState *c, const std::string &path,
                          const LocationConfig &loc,
                          std::size_t max_body) {
    for (std::map<std::string, std::string>::iterator hi = c->req.headers.begin();
         hi != c->req.headers.end(); ++hi)
    // body too large check
    if (c->req.contentLength() > max_body) {
        serveError(c, 413, "Payload Too Large");
        return;
    }
    // multipart parse: extract filename + content
    std::string ct = c->req.getHeader("content-type");
    std::size_t bnd = ct.find("boundary=");
    if (bnd == std::string::npos) {
        serveError(c, 400, "Bad Request");
        return;
    }
    std::string boundary = ct.substr(bnd + 9);
    std::size_t bp = boundary.find(';');
    if (bp != std::string::npos) boundary = boundary.substr(0, bp);
    std::string dash = "--" + boundary;
    std::size_t p1 = c->req.body.find(dash);
    if (p1 == std::string::npos) { serveError(c, 400, "Bad Request"); return; }
    std::size_t p2 = c->req.body.find(dash, p1 + dash.size());
    if (p2 == std::string::npos) { serveError(c, 400, "Bad Request"); return; }
    std::string part = c->req.body.substr(p1 + dash.size() + 2,
                                          p2 - (p1 + dash.size() + 2) - 2);
    // find filename
    std::size_t fn = part.find("filename=\"");
    std::string filename = "upload.bin";
    if (fn != std::string::npos) {
        std::size_t start = fn + 10;
        std::size_t end = part.find('"', start);
        if (end != std::string::npos)
            filename = part.substr(start, end - start);
    }
    std::size_t crlf = part.find("\r\n\r\n");
    if (crlf != std::string::npos)
        part = part.substr(crlf + 4);
    if (!loc.upload_store.empty())
        mkdir(loc.upload_store.c_str(), 0755);
    std::string dest = loc.upload_store.empty() ? path : loc.upload_store + "/" + filename;
    std::ofstream f(dest.c_str(), std::ios::binary);
    if (!f.is_open()) { serveError(c, 500, "Internal Server Error"); return; }
    f << part;
    f.close();
    sendResponse(c, 201, "Upload successful", "text/plain", true);
}

void Server::startCGI(ClientState *c, const std::string &script_path,
                      const LocationConfig &loc) {
    std::string ext = getExtension(script_path);
    std::string interp = "";
    std::map<std::string, std::string>::const_iterator ci = loc.cgi.find(ext);
    if (ci != loc.cgi.end())
        interp = ci->second;
    if (interp.empty()) { serveError(c, 502, "Bad Gateway"); return; }

    int out_pipe[2], in_pipe[2];
    if (pipe(out_pipe) < 0 || pipe(in_pipe) < 0) {
        serveError(c, 500, "Internal Server Error");
        return;
    }
    pid_t pid = fork();
    if (pid == 0) {
        // child
        close(out_pipe[0]);
        close(in_pipe[1]);
        dup2(out_pipe[1], 1);
        dup2(out_pipe[1], 2);
        dup2(in_pipe[0], 0);
        close(out_pipe[1]);
        close(in_pipe[0]);
        // env
        setenv("REQUEST_METHOD", c->req.method.c_str(), 1);
        setenv("REQUEST_URI", (c->req.path + (c->req.query.empty() ? "" : "?" + c->req.query)).c_str(), 1);
        setenv("QUERY_STRING", c->req.query.c_str(), 1);
        setenv("CONTENT_TYPE", c->req.getHeader("content-type").c_str(), 1);
        std::ostringstream cl;
        cl << c->req.body.size();
        setenv("CONTENT_LENGTH", cl.str().c_str(), 1);
        setenv("SCRIPT_FILENAME", script_path.c_str(), 1);
        setenv("SCRIPT_NAME", c->req.path.c_str(), 1);
        setenv("PATH_INFO", c->req.path.c_str(), 1);
        setenv("SERVER_PROTOCOL", "HTTP/1.1", 1);
        setenv("GATEWAY_INTERFACE", "CGI/1.1", 1);
        setenv("SERVER_NAME", "localhost", 1);
        std::ostringstream sp;
        sp << listeners[0].cfg->listen_port;
        setenv("SERVER_PORT", sp.str().c_str(), 1);
        std::string host = c->req.getHeader("host");
        setenv("HTTP_HOST", host.c_str(), 1);
        setenv("HTTP_COOKIE", c->req.getHeader("cookie").c_str(), 1);
        execlp(interp.c_str(), interp.c_str(), script_path.c_str(), (char *)NULL);
        _exit(127);
    }
    // parent
    close(out_pipe[1]);
    close(in_pipe[0]);
    // write body to child stdin
    if (!c->req.body.empty())
        write(in_pipe[1], c->req.body.c_str(), c->req.body.size());
    close(in_pipe[1]);
    c->cgi_running = true;
    c->cgi_start = time(NULL);
    cgi_pipes[out_pipe[0]] = c->fd;
}

void Server::onCgiReadable(int pipe_fd) {
    std::map<int, int>::iterator it = cgi_pipes.find(pipe_fd);
    if (it == cgi_pipes.end())
        return;
    std::map<int, ClientState*>::iterator cit = clients.find(it->second);
    if (cit == clients.end() || cit->second == NULL) {
        cgi_pipes.erase(it);
        close(pipe_fd);
        return;
    }
    ClientState *c = cit->second;
    char buf[8192];
    ssize_t n = read(pipe_fd, buf, sizeof(buf));
    if (n > 0) {
        c->out_buf.append(buf, static_cast<std::size_t>(n));
        return;
    }
    // EOF: CGI finished
    cgi_pipes.erase(it);
    close(pipe_fd);
    c->cgi_running = false;
    // Parse CGI output: either a full HTTP response or CGI headers + body
    std::string out = c->out_buf;
    c->out_buf.clear();
    std::string sep = "\r\n\r\n";
    std::size_t sp = out.find(sep);
    if (sp == std::string::npos) {
        sp = out.find("\n\n");
        if (sp != std::string::npos) sep = "\n\n";
    }
    if (sp == std::string::npos) {
        // no header/body separator: wrap entire output as body
        sendResponse(c, 200, out, "text/html", c->req.method != "HEAD");
    } else {
        std::string head = out.substr(0, sp);
        std::string body = out.substr(sp + sep.size());
        // determine status
        int status = 200;
        if (head.find("HTTP/1.1") != std::string::npos ||
            head.find("HTTP/1.0") != std::string::npos) {
            // full HTTP response: send as-is
            c->out_buf = out;
            c->response_sent = true;
        } else {
            // CGI headers: find Status: line
            std::istringstream hss(head);
            std::string line;
            std::string ctype = "text/html";
            bool has_status = false;
            while (std::getline(hss, line)) {
                std::size_t colon = line.find(':');
                if (colon == std::string::npos)
                    continue;
                std::string key = trim(toLower(line.substr(0, colon)));
                std::string val = trim(line.substr(colon + 1));
                if (key == "status") {
                    std::istringstream ss2(val);
                    ss2 >> status;
                    has_status = true;
                } else if (key == "content-type") {
                    ctype = val;
                }
            }
            if (!has_status) status = 200;
            sendResponse(c, status, body, ctype, c->req.method != "HEAD");
        }
    }
    // determine keep-alive
    std::string conn = c->req.getHeader("connection");
    c->keep_alive = (c->req.version == "HTTP/1.1" &&
                     (conn.empty() || toLower(conn) == "keep-alive")) ||
                    toLower(conn) == "keep-alive";
    if (!c->keep_alive)
        c->closed = true;
}

void Server::writeClient(int fd) {
    std::map<int, ClientState*>::iterator cit = clients.find(fd);
    if (cit == clients.end() || cit->second == NULL)
        return;
    ClientState *c = cit->second;
    if (c->out_buf.empty())
        return;
    ssize_t n = send(fd, c->out_buf.c_str(), c->out_buf.size(), 0);
    if (n <= 0) {
        if (n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK))
            return;
        removeClient(fd);
        return;
    }
    c->out_buf.erase(0, static_cast<std::size_t>(n));
    if (c->out_buf.empty() && c->closed)
        removeClient(fd);
    else if (c->out_buf.empty() && !c->cgi_running && c->response_sent) {
        // keep-alive: reset for next request
        std::size_t he = c->in_buf.find("\r\n\r\n");
        std::size_t consumed = he + 4;
        if (c->req.contentLength() > 0)
            consumed += c->req.contentLength();
        if (consumed <= c->in_buf.size())
            c->in_buf.erase(0, consumed);
        c->headers_done = false;
        c->body_done = false;
        c->response_sent = false;
        c->req = Request();
    }
}

void Server::removeClient(int fd) {
    std::map<int, ClientState*>::iterator it = clients.find(fd);
    if (it == clients.end())
        return;
    delete it->second;
    clients.erase(it);
    close(fd);
}

void Server::run() {
    if (listeners.empty()) {
        std::cerr << "webserv: no listening sockets" << std::endl;
        return;
    }
    while (true) {
        // CGI timeout check (10s -> 504)
        for (std::map<int, int>::iterator it = cgi_pipes.begin();
             it != cgi_pipes.end(); ) {
            ClientState *cc = NULL;
            std::map<int, ClientState*>::iterator cit = clients.find(it->second);
            if (cit != clients.end()) cc = cit->second;
            if (cc && cc->cgi_start && time(NULL) - cc->cgi_start > 10) {
                int pipe_fd = it->first;
                int client_fd = it->second;
                (void)client_fd;
                ++it;
                cgi_pipes.erase(pipe_fd);
                close(pipe_fd);
                // kill child (find pid via process tracking: skip, just close)
                if (cc) {
                    cc->cgi_running = false;
                    cc->out_buf.clear();
                    serveError(cc, 504, "Gateway Timeout");
                }
                continue;
            }
            ++it;
        }
        fd_set readset, writeset;
        int maxfd;
        buildFdSet(readset, writeset, maxfd);
        if (maxfd < 0) { usleep(10000); continue; }
        struct timeval tv;
        tv.tv_sec = 1;
        tv.tv_usec = 0;
        int n = select(maxfd + 1, &readset, &writeset, NULL, &tv);
        if (n < 0) {
            if (errno == EINTR)
                continue;
            break;
        }
        if (n == 0)
            continue;
        acceptClients(readset);
        // client reads
        for (std::map<int, ClientState*>::iterator it = clients.begin();
             it != clients.end(); ) {
            int fd = it->first;
            std::map<int, ClientState*>::iterator next = it;
            ++next;
            if (FD_ISSET(fd, &readset))
                onClientReadable(fd);
            if (FD_ISSET(fd, &writeset))
                writeClient(fd);
            // if removed, it is no longer in map
            if (clients.find(fd) == clients.end()) {
                it = next;
                continue;
            }
            it = next;
        }
        // CGI pipes
        std::vector<int> pipes_to_check;
        for (std::map<int, int>::iterator it = cgi_pipes.begin();
             it != cgi_pipes.end(); ++it)
            if (FD_ISSET(it->first, &readset))
                pipes_to_check.push_back(it->first);
        for (std::size_t i = 0; i < pipes_to_check.size(); ++i)
            onCgiReadable(pipes_to_check[i]);
    }
}

} // namespace webserv
