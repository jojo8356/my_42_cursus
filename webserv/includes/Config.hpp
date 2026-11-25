#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <string>
#include <vector>
#include <map>
#include <set>
#include <fstream>

namespace webserv {

struct LocationConfig {
    std::string              prefix;            // e.g. "/cgi-bin/"
    std::string              root;               // root dir (may be empty -> server root)
    bool                     alias;              // alias vs root
    std::set<std::string>    methods;            // allowed methods (empty = GET,HEAD)
    bool                     autoindex;
    std::map<std::string, std::string> cgi;      // ext -> interpreter path
    std::string              upload_store;      // upload dir
    std::string              index;             // index file(s), space separated
    int                      redirect_code;     // 0 = none
    std::string              redirect_url;

    LocationConfig() : alias(false), autoindex(false),
                       redirect_code(0) {}
};

struct ServerConfig {
    std::string              listen_host;       // "127.0.0.1" or "" (all)
    int                      listen_port;
    std::string              server_name;
    std::string              root;
    std::string              index;              // default index
    std::map<int, std::string> error_pages;     // status -> path
    std::size_t              client_max_body_size;
    std::vector<LocationConfig> locations;

    ServerConfig() : listen_port(0), client_max_body_size(10485760) {}

    const LocationConfig *findLocation(const std::string &path) const;
};

class Config {
public:
    std::vector<ServerConfig> servers;

    bool load(const std::string &filename);
    const ServerConfig *findServer(const std::string &host, int port) const;

private:
    void parse(const std::string &content);
};

} // namespace webserv

#endif
