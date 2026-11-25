#include "Channel.hpp"

namespace ft_irc {

Channel::Channel(const std::string &name_)
    : name(name_), key(), limit(0),
      mode_i(false), mode_t(false), mode_k(false), mode_l(false),
      mode_p(false), mode_s(false), mode_n(false), mode_m(false),
      mode_b(false), mode_v(false) {}

Channel::~Channel() {}

std::string Channel::modeString() const {
    std::string m = "+";
    if (mode_i) m += "i";
    if (mode_t) m += "t";
    if (mode_k) m += "k";
    if (mode_l) m += "l";
    if (mode_p) m += "p";
    if (mode_s) m += "s";
    if (mode_n) m += "n";
    if (mode_m) m += "m";
    if (mode_b) m += "b";
    if (mode_v) m += "v";
    return m;
}

} // namespace ft_irc
