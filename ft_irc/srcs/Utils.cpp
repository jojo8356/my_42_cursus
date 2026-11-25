#include "Utils.hpp"
#include <sys/socket.h>
#include <cstring>
#include <cctype>

namespace ft_irc {

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

std::string trim(const std::string &s) {
    std::size_t a = 0, b = s.size();
    while (a < b && (s[a] == ' ' || s[a] == '\t' || s[a] == '\r' || s[a] == '\n'))
        ++a;
    while (b > a && (s[b - 1] == ' ' || s[b - 1] == '\t' || s[b - 1] == '\r' ||
                     s[b - 1] == '\n'))
        --b;
    return s.substr(a, b - a);
}

std::string toLowerIrc(const std::string &s) {
    std::string out = s;
    for (std::size_t i = 0; i < out.size(); ++i) {
        char c = out[i];
        if (c >= 'A' && c <= 'Z')
            out[i] = static_cast<char>(c - 'A' + 'a');
        else if (c == '[')      out[i] = '~';
        else if (c == '\\')    out[i] = ']';
        else if (c == ']')     out[i] = '}';
        else if (c == '^')     out[i] = '^';
        else if (c == '`')     out[i] = '_';
        else if (c == '{')     out[i] = '|';
        else if (c == '|')     out[i] = '}';
    }
    return out;
}

bool isValidNick(const std::string &nick) {
    if (nick.empty() || nick.size() > 9)
        return false;
    for (std::size_t i = 0; i < nick.size(); ++i) {
        char c = nick[i];
        if (c == ' ' || c == ',' || c == '\r' || c == '\n')
            return false;
        // allowed: letters, digits, and a set of specials
        if (!(std::isalnum(static_cast<unsigned char>(c)) ||
              c == '-' || c == '_' || c == '[' || c == ']' ||
              c == '\\' || c == '^' || c == '`' || c == '{' ||
              c == '|' || c == '}'))
            return false;
    }
    return true;
}

bool isValidChannel(const std::string &name) {
    if (name.empty() || name.size() > 50)
        return false;
    if (name[0] != '#' && name[0] != '&')
        return false;
    return true;
}

int sendLine(int fd, const std::string &line) {
    std::string data = line + "\r\n";
    std::size_t total = 0;
    while (total < data.size()) {
        ssize_t n = send(fd, data.c_str() + total, data.size() - total, 0);
        if (n <= 0)
            return static_cast<int>(n);
        total += static_cast<std::size_t>(n);
    }
    return static_cast<int>(total);
}

std::string makePrefix(const std::string &nick, const std::string &user,
                       const std::string &host) {
    return nick + "!" + user + "@" + host;
}

std::string makeMsg(const std::string &prefix, const std::string &cmd,
                    const std::string &params) {
    std::string m = cmd;
    if (!prefix.empty())
        m = prefix + " " + m;
    if (!params.empty())
        m += " " + params;
    return m;
}

} // namespace ft_irc
