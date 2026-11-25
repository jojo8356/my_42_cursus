#include "Request.hpp"
#include "Utils.hpp"
#include <sstream>

namespace webserv {

Request::Request() {}

bool Request::hasHeader(const std::string &name) const {
    return headers.find(toLower(name)) != headers.end();
}

std::string Request::getHeader(const std::string &name) const {
    std::map<std::string, std::string>::const_iterator it =
        headers.find(toLower(name));
    return it == headers.end() ? std::string() : it->second;
}

std::size_t Request::contentLength() const {
    std::string cl = getHeader("content-length");
    if (cl.empty())
        return 0;
    std::istringstream iss(cl);
    std::size_t v = 0;
    iss >> v;
    return v;
}

bool Request::isChunked() const {
    std::string te = getHeader("transfer-encoding");
    return toLower(te).find("chunked") != std::string::npos;
}

bool Request::parse(const std::string &data) {
    raw = data;
    std::size_t header_end = data.find("\r\n\r\n");
    if (header_end == std::string::npos)
        return false;

    std::string head = data.substr(0, header_end);
    std::string body_part = data.substr(header_end + 4);

    // request line
    std::size_t nl = head.find("\r\n");
    std::string request_line = (nl == std::string::npos) ? head : head.substr(0, nl);
    std::istringstream rl(request_line);
    std::string full_path;
    rl >> method >> full_path >> version;
    if (method.empty() || full_path.empty())
        return false;

    std::size_t q = full_path.find('?');
    if (q == std::string::npos) {
        path = full_path;
    } else {
        path = full_path.substr(0, q);
        query = full_path.substr(q + 1);
    }
    // URL-decode path
    path = urlDecode(path);

    // headers
    std::size_t pos = (nl == std::string::npos) ? head.size() : nl + 2;
    while (pos < head.size()) {
        std::size_t eol = head.find("\r\n", pos);
        if (eol == std::string::npos)
            break;
        std::string line = head.substr(pos, eol - pos);
        std::size_t colon = line.find(':');
        if (colon != std::string::npos) {
            std::string key = trim(line.substr(0, colon));
            std::string val = trim(line.substr(colon + 1));
            if (!key.empty())
                headers[toLower(key)] = val;
        }
        pos = eol + 2;
    }

    body = body_part;
    return true;
}

} // namespace webserv
