#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <string>
#include <map>

namespace webserv {

class Request {
public:
    std::string             method;
    std::string             path;
    std::string             query;
    std::string             version;
    std::map<std::string, std::string> headers; // lowercased keys
    std::string             body;
    std::string             raw;                  // full raw request

    Request();

    // Parse a full request (headers + body). Returns true on success.
    bool parse(const std::string &data);

    bool hasHeader(const std::string &name) const;
    std::string getHeader(const std::string &name) const;
    std::size_t contentLength() const;
    bool isChunked() const;
};

} // namespace webserv

#endif
