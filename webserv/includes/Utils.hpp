#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>
#include <vector>
#include <map>

namespace webserv {

std::string trim(const std::string &s);
std::string toLower(const std::string &s);
std::vector<std::string> split(const std::string &s, char delim);
std::string getExtension(const std::string &path);
std::string contentTypeFor(const std::string &path);
bool isDir(const std::string &path);
bool isFile(const std::string &path);
std::string urlDecode(const std::string &s);
// Split a path into directory + basename.
std::string dirName(const std::string &path);

} // namespace webserv

#endif
