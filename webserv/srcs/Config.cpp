#include "Config.hpp"
#include "Utils.hpp"
#include <sstream>
#include <iostream>

namespace webserv {

static std::vector<std::string> tokenize(const std::string &s) {
    std::vector<std::string> toks;
    std::string cur;
    for (std::size_t i = 0; i < s.size(); ++i) {
        char c = s[i];
        if (c == ';' || c == '{' || c == '}') {
            if (!cur.empty()) { toks.push_back(cur); cur.clear(); }
            toks.push_back(std::string(1, c));
        } else if (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
            if (!cur.empty()) { toks.push_back(cur); cur.clear(); }
        } else {
            cur += c;
        }
    }
    if (!cur.empty())
        toks.push_back(cur);
    return toks;
}

static std::size_t parseSize(const std::string &v) {
    std::string s = toLower(trim(v));
    std::size_t mult = 1;
    if (!s.empty() && (s[s.size() - 1] == 'k' || s[s.size() - 1] == 'K')) {
        mult = 1024; s.erase(s.size() - 1);
    } else if (!s.empty() && (s[s.size() - 1] == 'm' || s[s.size() - 1] == 'M')) {
        mult = 1024 * 1024; s.erase(s.size() - 1);
    } else if (!s.empty() && (s[s.size() - 1] == 'g' || s[s.size() - 1] == 'G')) {
        mult = 1024 * 1024 * 1024; s.erase(s.size() - 1);
    }
    std::istringstream iss(s);
    std::size_t n = 0;
    iss >> n;
    return n * mult;
}

static int toInt(const std::string &v) {
    std::istringstream iss(v);
    int n = 0;
    iss >> n;
    return n;
}

const LocationConfig *ServerConfig::findLocation(const std::string &path) const {
    const LocationConfig *best = NULL;
    std::size_t best_len = 0;
    for (std::size_t i = 0; i < locations.size(); ++i) {
        const std::string &prefix = locations[i].prefix;
        if (prefix.empty())
            continue;
        if (path.compare(0, prefix.size(), prefix) == 0 &&
            prefix.size() > best_len) {
            best = &locations[i];
            best_len = prefix.size();
        }
    }
    return best;
}

class Parser {
public:
    Parser(const std::vector<std::string> &toks) : toks(toks), i(0) {}

    bool parse(Config &cfg) {
        while (i < toks.size()) {
            if (toks[i] == "server") {
                ++i;
                ServerConfig sc;
                if (i >= toks.size() || toks[i] != "{")
                    return false;
                ++i;
                if (!parseServerBlock(sc))
                    return false;
                cfg.servers.push_back(sc);
            } else {
                ++i; // skip unexpected
            }
        }
        return !cfg.servers.empty();
    }

private:
    const std::vector<std::string> &toks;
    std::size_t i;

    bool parseServerBlock(ServerConfig &sc) {
        while (i < toks.size() && toks[i] != "}") {
            std::string dir = toks[i];
            ++i;
            if (dir == "location") {
                if (i >= toks.size())
                    return false;
                LocationConfig lc;
                lc.prefix = toks[i];
                ++i;
                if (i >= toks.size() || toks[i] != "{")
                    return false;
                ++i;
                if (!parseLocationBlock(lc))
                    return false;
                sc.locations.push_back(lc);
            } else if (i < toks.size() && toks[i] == ";") {
                ++i;
            } else {
                // server-level directive: name then values until ;
                std::vector<std::string> vals;
                while (i < toks.size() && toks[i] != ";" && toks[i] != "}") {
                    vals.push_back(toks[i]);
                    ++i;
                }
                if (i < toks.size() && toks[i] == ";")
                    ++i;
                applyServerDirective(sc, dir, vals);
            }
        }
        if (i < toks.size() && toks[i] == "}")
            ++i;
        return true;
    }

    bool parseLocationBlock(LocationConfig &lc) {
        while (i < toks.size() && toks[i] != "}") {
            std::string dir = toks[i];
            ++i;
            if (i < toks.size() && toks[i] == ";") {
                ++i;
                continue;
            }
            std::vector<std::string> vals;
            while (i < toks.size() && toks[i] != ";" && toks[i] != "}") {
                vals.push_back(toks[i]);
                ++i;
            }
            if (i < toks.size() && toks[i] == ";")
                ++i;
            applyLocationDirective(lc, dir, vals);
        }
        if (i < toks.size() && toks[i] == "}")
            ++i;
        return true;
    }

    void applyServerDirective(ServerConfig &sc, const std::string &dir,
                              const std::vector<std::string> &v) {
        if (dir == "listen") {
            std::string spec = v.empty() ? "" : v[0];
            std::size_t colon = spec.find(':');
            if (colon == std::string::npos) {
                sc.listen_host = "";
                sc.listen_port = toInt(spec);
            } else {
                sc.listen_host = spec.substr(0, colon);
                sc.listen_port = toInt(spec.substr(colon + 1));
            }
        } else if (dir == "server_name") {
            sc.server_name = v.empty() ? "" : v[0];
        } else if (dir == "root") {
            sc.root = v.empty() ? "" : v[0];
        } else if (dir == "index") {
            for (std::size_t k = 0; k < v.size(); ++k)
                sc.index += (k ? " " : "") + v[k];
        } else if (dir == "error_page") {
            if (v.size() >= 2)
                sc.error_pages[toInt(v[0])] = v[1];
        } else if (dir == "client_max_body_size") {
            if (!v.empty())
                sc.client_max_body_size = parseSize(v[0]);
        }
    }

    void applyLocationDirective(LocationConfig &lc, const std::string &dir,
                                const std::vector<std::string> &v) {
        if (dir == "root") {
            if (!v.empty()) lc.root = v[0];
        } else if (dir == "alias") {
            if (!v.empty()) { lc.alias = true; lc.root = v[0]; }
        } else if (dir == "allow_methods" || dir == "methods" ||
                   dir == "limit_except") {
            for (std::size_t k = 0; k < v.size(); ++k)
                lc.methods.insert(toLower(v[k]));
        } else if (dir == "autoindex") {
            lc.autoindex = !v.empty() && (v[0] == "on" || v[0] == "1");
        } else if (dir == "cgi" || dir == "cgi_pass" || dir == "cgi_ext") {
            if (v.size() >= 2) {
                std::string ext = v[0];
                if (!ext.empty() && ext[0] == '.')
                    ext = ext.substr(1);
                lc.cgi[ext] = v[1];
            }
        } else if (dir == "upload_store" || dir == "upload_path" ||
                   dir == "upload_dir") {
            if (!v.empty()) lc.upload_store = v[0];
        } else if (dir == "index") {
            for (std::size_t k = 0; k < v.size(); ++k)
                lc.index += (k ? " " : "") + v[k];
        } else if (dir == "return") {
            if (v.size() >= 2) {
                lc.redirect_code = toInt(v[0]);
                lc.redirect_url = v[1];
            } else if (v.size() == 1) {
                lc.redirect_code = 302;
                lc.redirect_url = v[0];
            }
        }
    }
};

bool Config::load(const std::string &filename) {
    std::ifstream in(filename.c_str());
    if (!in.is_open()) {
        std::cerr << "webserv: cannot open config: " << filename << std::endl;
        return false;
    }
    std::string content((std::istreambuf_iterator<char>(in)),
                         std::istreambuf_iterator<char>());
    std::vector<std::string> toks = tokenize(content);
    Parser p(toks);
    return p.parse(*this);
}

const ServerConfig *Config::findServer(const std::string &host, int port) const {
    for (std::size_t i = 0; i < servers.size(); ++i) {
        if (servers[i].listen_port == port) {
            if (servers[i].listen_host.empty() ||
                servers[i].listen_host == host)
                return &servers[i];
        }
    }
    return servers.empty() ? NULL : &servers[0];
}

} // namespace webserv
