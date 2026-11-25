#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <string>
#include <set>

namespace ft_irc {

class Channel {
public:
    std::string     name;
    std::string     topic;
    std::string     key;          // +k channel key
    int             limit;        // +l user limit
    bool            mode_i;       // invite-only
    bool            mode_t;       // topic restricted to operators
    bool            mode_k;       // key required
    bool            mode_l;       // user limit
    // Extended modes accepted (bonus) — stored, behavior for core set only.
    bool            mode_p, mode_s, mode_n, mode_m, mode_b, mode_v;
    std::set<int>   members;
    std::set<int>   operators;
    std::set<int>   invited;      // +i invite list

    explicit Channel(const std::string &name_);
    ~Channel();

    bool isMember(int fd) const { return members.find(fd) != members.end(); }
    bool isOperator(int fd) const { return operators.find(fd) != operators.end(); }
    bool isInvited(int fd) const { return invited.find(fd) != invited.end(); }

    // Human-readable mode string (e.g. "+itkl").
    std::string modeString() const;

    // True when the channel has no members left (should be destroyed).
    bool isEmpty() const { return members.empty(); }
};

} // namespace ft_irc

#endif
