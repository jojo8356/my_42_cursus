#include "Utils.hpp"
#include <sys/stat.h>
#include <cctype>
#include <cstring>

namespace webserv {

std::string trim(const std::string &s) {
    std::size_t a = 0, b = s.size();
    while (a < b && (s[a] == ' ' || s[a] == '\t' || s[a] == '\r' || s[a] == '\n'))
        ++a;
    while (b > a && (s[b - 1] == ' ' || s[b - 1] == '\t' || s[b - 1] == '\r' ||
                     s[b - 1] == '\n'))
        --b;
    return s.substr(a, b - a);
}

std::string toLower(const std::string &s) {
    std::string out = s;
    for (std::size_t i = 0; i < out.size(); ++i)
        if (out[i] >= 'A' && out[i] <= 'Z')
            out[i] = static_cast<char>(out[i] - 'A' + 'a');
    return out;
}

std::vector<std::string> split(const std::string &s, char delim) {
    std::vector<std::string> tokens;
    std::string cur;
    for (std::size_t i = 0; i < s.size(); ++i) {
        if (s[i] == delim) {
            tokens.push_back(cur);
            cur.clear();
        } else {
            cur += s[i];
        }
    }
    tokens.push_back(cur);
    return tokens;
}

std::string getExtension(const std::string &path) {
    std::size_t slash = path.find_last_of('/');
    std::size_t dot = path.find_last_of('.');
    if (dot == std::string::npos)
        return "";
    if (slash != std::string::npos && dot < slash)
        return "";
    return toLower(path.substr(dot + 1));
}

std::string contentTypeFor(const std::string &path) {
    std::string ext = getExtension(path);
    if (ext == "html" || ext == "htm") return "text/html";
    if (ext == "css") return "text/css";
    if (ext == "js") return "application/javascript";
    if (ext == "jpg" || ext == "jpeg") return "image/jpeg";
    if (ext == "png") return "image/png";
    if (ext == "gif") return "image/gif";
    if (ext == "svg") return "image/svg+xml";
    if (ext == "ico") return "image/x-icon";
    if (ext == "txt") return "text/plain";
    if (ext == "json") return "application/json";
    if (ext == "pdf") return "application/pdf";
    if (ext == "xml") return "text/xml";
    if (ext == "py") return "text/plain";
    if (ext == "pl") return "text/plain";
    if (ext == "php") return "text/html";
    return "application/octet-stream";
}

bool isDir(const std::string &path) {
    struct stat st;
    if (stat(path.c_str(), &st) != 0)
        return false;
    return (st.st_mode & S_IFMT) == S_IFDIR;
}

bool isFile(const std::string &path) {
    struct stat st;
    if (stat(path.c_str(), &st) != 0)
        return false;
    return (st.st_mode & S_IFMT) == S_IFREG;
}

std::string urlDecode(const std::string &s) {
    std::string out;
    for (std::size_t i = 0; i < s.size(); ++i) {
        if (s[i] == '%' && i + 2 < s.size()) {
            int hi = s[i + 1] >= '0' && s[i + 1] <= '9' ? s[i + 1] - '0'
                        : s[i + 1] >= 'a' && s[i + 1] <= 'f' ? s[i + 1] - 'a' + 10
                        : s[i + 1] - 'A' + 10;
            int lo = s[i + 2] >= '0' && s[i + 2] <= '9' ? s[i + 2] - '0'
                        : s[i + 2] >= 'a' && s[i + 2] <= 'f' ? s[i + 2] - 'a' + 10
                        : s[i + 2] - 'A' + 10;
            out += static_cast<char>((hi << 4) | lo);
            i += 2;
        } else if (s[i] == '+') {
            out += ' ';
        } else {
            out += s[i];
        }
    }
    return out;
}

std::string dirName(const std::string &path) {
    std::size_t slash = path.find_last_of('/');
    if (slash == std::string::npos)
        return "";
    if (slash == 0)
        return "/";
    return path.substr(0, slash + 1);
}

} // namespace webserv
