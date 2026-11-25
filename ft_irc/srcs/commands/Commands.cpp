#include "Commands.hpp"
#include "Utils.hpp"
#include <cctype>
#include <cstdlib>

namespace ft_irc {

static std::string toUpper(std::string s) {
    for (std::size_t i = 0; i < s.size(); ++i)
        if (s[i] >= 'a' && s[i] <= 'z')
            s[i] = static_cast<char>(s[i] - 'a' + 'A');
    return s;
}

static std::string itos(long n) {
    if (n == 0)
        return "0";
    std::string s;
    bool neg = n < 0;
    if (neg)
        n = -n;
    while (n > 0) {
        s = static_cast<char>('0' + (n % 10)) + s;
        n /= 10;
    }
    if (neg)
        s = "-" + s;
    return s;
}

static void reply(Server &srv, int fd, const std::string &msg) {
    srv.sendTo(fd, msg);
}

static void numeric(Server &srv, int fd, const std::string &num,
                    const std::string &params) {
    srv.sendTo(fd, ":ircserv " + num + " " + params);
}

// Parse a line into command + params (last param may be trailing text after " :").
static void parseLine(const std::string &line, std::string &cmd,
                      std::vector<std::string> &params) {
    std::size_t sp = line.find(' ');
    if (sp == std::string::npos) {
        cmd = line;
        params.clear();
        return;
    }
    cmd = line.substr(0, sp);
    std::string rest = line.substr(sp + 1);
    std::size_t colon = rest.find(" :");
    if (colon != std::string::npos) {
        std::string head = rest.substr(0, colon);
        std::string trailing = rest.substr(colon + 2);
        std::vector<std::string> head_parts = split(head, ' ');
        for (std::size_t i = 0; i < head_parts.size(); ++i)
            if (!head_parts[i].empty())
                params.push_back(head_parts[i]);
        if (!trailing.empty())
            params.push_back(trailing);
    } else {
        std::vector<std::string> parts = split(rest, ' ');
        for (std::size_t i = 0; i < parts.size(); ++i)
            if (!parts[i].empty())
                params.push_back(parts[i]);
    }
}

// --- PASS ---
static void cmdPass(Server &srv, int fd, const std::vector<std::string> &p) {
    Client *c = srv.getClient(fd);
    if (p.empty())
        return;
    c->pass_ok = (p[0] == srv.password);
    c->pass_received = true;
    srv.tryRegister(fd);
}

// --- NICK ---
static void cmdNick(Server &srv, int fd, const std::vector<std::string> &p) {
    Client *c = srv.getClient(fd);
    if (p.empty()) {
        numeric(srv, fd, "431", ":No nickname given");
        return;
    }
    if (!isValidNick(p[0])) {
        numeric(srv, fd, "432", p[0] + " :Erroneous nickname");
        return;
    }
    std::string folded = toLowerIrc(p[0]);
    for (std::map<int, Client*>::iterator it = srv.clients.begin();
         it != srv.clients.end(); ++it) {
        if (it->first != fd && it->second->nickname.size() == p[0].size() &&
            toLowerIrc(it->second->nickname) == folded) {
            numeric(srv, fd, "433", "* " + p[0] + " :Nickname is already in use");
            return;
        }
    }
    std::string old = c->nickname;
    c->nickname = p[0];
    c->has_nick = true;
    if (!old.empty() && c->isRegistered()) {
        // broadcast nickname change to channels
        for (std::map<std::string, Channel*>::iterator it = srv.channels.begin();
             it != srv.channels.end(); ++it) {
            if (it->second->isMember(fd)) {
                std::string msg = makeMsg(
                    makePrefix(c->nickname, c->username, c->hostname),
                    "NICK", c->nickname);
                srv.broadcastChannel(it->second, msg, fd);
            }
        }
    }
    srv.tryRegister(fd);
}

// --- USER ---
static void cmdUser(Server &srv, int fd, const std::vector<std::string> &p) {
    Client *c = srv.getClient(fd);
    if (p.size() < 2) {
        numeric(srv, fd, "461", "USER :Not enough parameters");
        return;
    }
    c->username = p[0];
    if (p.size() >= 4)
        c->realname = p[3];
    c->has_user = true;
    srv.tryRegister(fd);
}

// --- CAP ---
static void cmdCap(Server &srv, int fd, const std::vector<std::string> &p) {
    if (p.size() >= 1 && toUpper(p[0]) == "LS") {
        reply(srv, fd, ":ircserv CAP LS :multi-prefix");
    } else if (p.size() >= 1 && toUpper(p[0]) == "REQ") {
        reply(srv, fd, ":ircserv CAP NAK :" + p[1]);
    }
    // CAP END / CAP END 302 -> no reply, proceed
}

// --- PING ---
static void cmdPing(Server &srv, int fd, const std::vector<std::string> &p) {
    std::string token = p.empty() ? "" : p[0];
    reply(srv, fd, "PONG :" + token);
}

// --- QUIT ---
static void cmdQuit(Server &srv, int fd, const std::vector<std::string> &p) {
    std::string reason = p.empty() ? "Goodbye" : p[0];
    srv.clientQuit(fd, reason);
}

// --- JOIN ---
static void cmdPart(Server &srv, int fd, const std::vector<std::string> &p);
static void cmdJoin(Server &srv, int fd, const std::vector<std::string> &p) {
    if (p.empty()) {
        numeric(srv, fd, "461", "JOIN :Not enough parameters");
        return;
    }
    // JOIN 0 -> leave all channels
    if (p[0] == "0") {
        std::vector<std::string> mine;
        for (std::map<std::string, Channel*>::iterator it = srv.channels.begin();
             it != srv.channels.end(); ++it) {
            if (it->second->isMember(fd))
                mine.push_back(it->first);
        }
        for (std::size_t i = 0; i < mine.size(); ++i)
            cmdPart(srv, fd, std::vector<std::string>(1, mine[i]));
        return;
    }

    std::vector<std::string> chans = split(p[0], ',');
    std::vector<std::string> keys;
    if (p.size() >= 2)
        keys = split(p[1], ',');

    for (std::size_t i = 0; i < chans.size(); ++i) {
        std::string name = chans[i];
        if (!isValidChannel(name)) {
            numeric(srv, fd, "403", name + " :No such channel");
            continue;
        }
        Channel *ch = srv.getChannel(name);
        std::string key = (i < keys.size()) ? keys[i] : "";

        if (ch) {
            if (ch->mode_i && !ch->isInvited(fd) && !ch->isMember(fd)) {
                numeric(srv, fd, "473", name + " :Cannot join channel (+i)");
                continue;
            }
            if (ch->mode_k && key != ch->key) {
                numeric(srv, fd, "475", name + " :Wrong channel key");
                continue;
            }
            if (ch->mode_l && static_cast<int>(ch->members.size()) >= ch->limit) {
                numeric(srv, fd, "471", name + " :Channel is full");
                continue;
            }
            if (ch->isMember(fd)) {
                numeric(srv, fd, "442", name + " :You are already on that channel");
                continue;
            }
            srv.addClientToChannel(fd, ch, false);
        } else {
            ch = new Channel(name);
            ch->mode_k = false;
            if (!key.empty()) {
                ch->mode_k = true;
                ch->key = key;
            }
            srv.channels[name] = ch;
            srv.addClientToChannel(fd, ch, true); // creator = operator
        }
        // Send topic + names
        if (ch->topic.empty())
            numeric(srv, fd, "331", name + " :No topic is set");
        else
            numeric(srv, fd, "332", name + " :" + ch->topic);
        std::string names;
        for (std::set<int>::iterator it = ch->members.begin();
             it != ch->members.end(); ++it) {
            if (ch->isOperator(*it))
                names += "@";
            names += srv.nickOf(*it);
            names += " ";
        }
        numeric(srv, fd, "353", "=" + name + " :" + names);
        numeric(srv, fd, "366", name + " :" + "End of NAMES list");
        // Broadcast the JOIN event to the other channel members
        Client *cl = srv.getClient(fd);
        if (cl) {
            std::string jm = makeMsg(
                makePrefix(cl->nickname, cl->username, cl->hostname),
                "JOIN", name);
            srv.broadcastChannel(ch, jm, fd);
        }
    }
}

// --- PART ---
static void cmdPart(Server &srv, int fd, const std::vector<std::string> &p) {
    Client *c = srv.getClient(fd);
    if (p.empty()) {
        numeric(srv, fd, "461", "PART :Not enough parameters");
        return;
    }
    std::string reason = (p.size() >= 2) ? p[1] : "Leaving";
    std::vector<std::string> chans = split(p[0], ',');
    for (std::size_t i = 0; i < chans.size(); ++i) {
        std::string name = chans[i];
        Channel *ch = srv.getChannel(name);
        if (!ch) {
            numeric(srv, fd, "403", name + " :No such channel");
            continue;
        }
        if (!ch->isMember(fd)) {
            numeric(srv, fd, "442", name + " :You're not on that channel");
            continue;
        }
        std::string msg = makeMsg(
            makePrefix(c->nickname, c->username, c->hostname),
            "PART", name + " :" + reason);
        srv.broadcastChannel(ch, msg, fd);
        srv.removeClientFromChannel(fd, ch);
        srv.removeChannelIfEmpty(name);
    }
}

// --- TOPIC ---
static void cmdTopic(Server &srv, int fd, const std::vector<std::string> &p) {
    Client *c = srv.getClient(fd);
    if (p.empty()) {
        numeric(srv, fd, "461", "TOPIC :Not enough parameters");
        return;
    }
    std::string name = p[0];
    Channel *ch = srv.getChannel(name);
    if (!ch) {
        numeric(srv, fd, "403", name + " :No such channel");
        return;
    }
    if (p.size() < 2) {
        // view
        if (ch->topic.empty())
            numeric(srv, fd, "331", name + " :No topic is set");
        else
            numeric(srv, fd, "332", name + " :" + ch->topic);
        return;
    }
    if (ch->mode_t && !ch->isOperator(fd)) {
        numeric(srv, fd, "482", name + " :You don't have operator privileges");
        return;
    }
    ch->topic = p[1];
    std::string msg = makeMsg(
        makePrefix(c->nickname, c->username, c->hostname),
        "TOPIC", name + " :" + ch->topic);
    srv.broadcastChannel(ch, msg, fd);
}

// --- NAMES ---
static void cmdNames(Server &srv, int fd, const std::vector<std::string> &p) {
    if (p.empty()) {
        // list all channels
        for (std::map<std::string, Channel*>::iterator it = srv.channels.begin();
             it != srv.channels.end(); ++it) {
            std::string names;
            for (std::set<int>::iterator m = it->second->members.begin();
                 m != it->second->members.end(); ++m) {
                if (it->second->isOperator(*m))
                    names += "@";
                names += srv.nickOf(*m);
                names += " ";
            }
            numeric(srv, fd, "353", "=" + it->first + " :" + names);
        }
        numeric(srv, fd, "366", std::string("* :") + "End of NAMES list");
        return;
    }
    std::vector<std::string> chans = split(p[0], ',');
    for (std::size_t i = 0; i < chans.size(); ++i) {
        std::string name = chans[i];
        Channel *ch = srv.getChannel(name);
        if (!ch) {
            numeric(srv, fd, "403", name + " :No such channel");
            continue;
        }
        std::string names;
        for (std::set<int>::iterator m = ch->members.begin();
             m != ch->members.end(); ++m) {
            if (ch->isOperator(*m))
                names += "@";
            names += srv.nickOf(*m);
            names += " ";
        }
        numeric(srv, fd, "353", "=" + name + " :" + names);
    }
    numeric(srv, fd, "366", chans[0] + " :" + "End of NAMES list");
}

// --- LIST ---
static void cmdList(Server &srv, int fd, const std::vector<std::string> &p) {
    (void)p;
    numeric(srv, fd, "321", ":Channel :Users Topic");
    for (std::map<std::string, Channel*>::iterator it = srv.channels.begin();
         it != srv.channels.end(); ++it) {
        numeric(srv, fd, "322",
                it->first + " " + itos(it->second->members.size()) +
                " :" + it->second->topic);
    }
    numeric(srv, fd, "323", ":End of LIST");
}

// --- INVITE ---
static void cmdInvite(Server &srv, int fd, const std::vector<std::string> &p) {
    Client *c = srv.getClient(fd);
    if (p.size() < 2) {
        numeric(srv, fd, "461", "INVITE :Not enough parameters");
        return;
    }
    std::string nick = p[0];
    std::string name = p[1];
    Channel *ch = srv.getChannel(name);
    if (!ch) {
        numeric(srv, fd, "403", name + " :No such channel");
        return;
    }
    if (!ch->isOperator(fd)) {
        numeric(srv, fd, "481", name + " :You're not a channel operator");
        return;
    }
    // find target client by nickname
    int target_fd = -1;
    for (std::map<int, Client*>::iterator it = srv.clients.begin();
         it != srv.clients.end(); ++it) {
        if (it->second->nickname.size() == nick.size() &&
            toLowerIrc(it->second->nickname) == toLowerIrc(nick)) {
            target_fd = it->first;
            break;
        }
    }
    if (target_fd < 0) {
        numeric(srv, fd, "401", nick + " :No such nick");
        return;
    }
    if (ch->isMember(target_fd)) {
        numeric(srv, fd, "443", nick + " " + name + " :User is already on channel");
        return;
    }
    ch->invited.insert(target_fd);
    numeric(srv, fd, "341", nick + " " + name);
    numeric(srv, target_fd, "342", c->nickname + " " + name);
}

// --- KICK ---
static void cmdKick(Server &srv, int fd, const std::vector<std::string> &p) {
    Client *c = srv.getClient(fd);
    if (p.size() < 2) {
        numeric(srv, fd, "461", "KICK :Not enough parameters");
        return;
    }
    std::string name = p[0];
    std::string nick = p[1];
    std::string reason = (p.size() >= 3) ? p[2] : "Kicked";
    Channel *ch = srv.getChannel(name);
    if (!ch) {
        numeric(srv, fd, "403", name + " :No such channel");
        return;
    }
    if (!ch->isOperator(fd)) {
        numeric(srv, fd, "481", name + " :You're not a channel operator");
        return;
    }
    int target_fd = -1;
    for (std::map<int, Client*>::iterator it = srv.clients.begin();
         it != srv.clients.end(); ++it) {
        if (it->second->nickname.size() == nick.size() &&
            toLowerIrc(it->second->nickname) == toLowerIrc(nick)) {
            target_fd = it->first;
            break;
        }
    }
    if (target_fd < 0 || !ch->isMember(target_fd)) {
        numeric(srv, fd, "441", nick + " " + name + " :They aren't on that channel");
        return;
    }
    std::string msg = makeMsg(
        makePrefix(c->nickname, c->username, c->hostname),
        "KICK", name + " " + nick + " :" + reason);
    srv.broadcastChannel(ch, msg, -1);
    reply(srv, target_fd, msg);
    srv.removeClientFromChannel(target_fd, ch);
    srv.removeChannelIfEmpty(name);
}

// --- MODE ---
static void cmdMode(Server &srv, int fd, const std::vector<std::string> &p) {
    if (p.size() < 2) {
        numeric(srv, fd, "461", "MODE :Not enough parameters");
        return;
    }
    std::string name = p[0];
    Channel *ch = srv.getChannel(name);
    if (!ch) {
        numeric(srv, fd, "403", name + " :No such channel");
        return;
    }
    if (!ch->isOperator(fd)) {
        numeric(srv, fd, "481", name + " :You're not a channel operator");
        return;
    }
    std::string flags = p[1];
    std::size_t pos = 0;
    std::size_t arg_idx = 2;
    std::string added, removed;
    while (pos < flags.size()) {
        char sign = flags[pos++];
        bool on = (sign == '+');
        if (pos >= flags.size())
            break;
        char m = flags[pos++];
        std::string arg;
        if (m == 'k') {
            if (arg_idx >= p.size())
                break;
            arg = p[arg_idx++];
            if (on) { ch->mode_k = true; ch->key = arg; added += "k "; }
            else { ch->mode_k = false; ch->key.clear(); removed += "k "; }
        } else if (m == 'l') {
            if (arg_idx >= p.size())
                break;
            arg = p[arg_idx++];
            if (on) { ch->mode_l = true; ch->limit = std::atoi(arg.c_str()); added += "l "; }
            else { ch->mode_l = false; ch->limit = 0; removed += "l "; }
        } else if (m == 'o') {
            if (arg_idx >= p.size())
                break;
            arg = p[arg_idx++];
            int target_fd = -1;
            for (std::map<int, Client*>::iterator it = srv.clients.begin();
                 it != srv.clients.end(); ++it) {
                if (it->second->nickname.size() == arg.size() &&
                    toLowerIrc(it->second->nickname) == toLowerIrc(arg)) {
                    target_fd = it->first;
                    break;
                }
            }
            if (target_fd >= 0 && ch->isMember(target_fd)) {
                if (on) { ch->operators.insert(target_fd); added += "o " + arg + " "; }
                else { ch->operators.erase(target_fd); removed += "o " + arg + " "; }
            }
        } else {
            // i, t, p, s, n, m, b, v
            if (m == 'i') { ch->mode_i = on; on ? added += "i " : removed += "i "; }
            else if (m == 't') { ch->mode_t = on; on ? added += "t " : removed += "t "; }
            else if (m == 'p') { ch->mode_p = on; }
            else if (m == 's') { ch->mode_s = on; }
            else if (m == 'n') { ch->mode_n = on; }
            else if (m == 'm') { ch->mode_m = on; }
            else if (m == 'b') { ch->mode_b = on; }
            else if (m == 'v') { ch->mode_v = on; }
        }
    }
    std::string resp = ch->modeString();
    if (!ch->key.empty()) resp += " " + ch->key;
    if (ch->mode_l) resp += " " + itos(ch->limit);
    numeric(srv, fd, "351", name + " " + resp);
}

// --- PRIVMSG / NOTICE ---
static void sendMessage(Server &srv, int fd, const std::string &cmd,
                        const std::vector<std::string> &p) {
    Client *c = srv.getClient(fd);
    if (p.size() < 2) {
        if (cmd == "PRIVMSG")
            numeric(srv, fd, "461", cmd + " :Not enough parameters");
        return;
    }
    std::string target = p[0];
    std::string text = p[1];
    std::string prefix = makePrefix(c->nickname, c->username, c->hostname);
    std::string msg = makeMsg(prefix, cmd, target + " :" + text);

    Channel *ch = srv.getChannel(target);
    if (ch) {
        srv.broadcastChannel(ch, msg, fd);
        return;
    }
    // private message to a user
    int target_fd = -1;
    for (std::map<int, Client*>::iterator it = srv.clients.begin();
         it != srv.clients.end(); ++it) {
        if (it->second->nickname.size() == target.size() &&
            toLowerIrc(it->second->nickname) == toLowerIrc(target)) {
            target_fd = it->first;
            break;
        }
    }
    if (target_fd < 0) {
        if (cmd == "PRIVMSG")
            numeric(srv, fd, "401", target + " :No such nick/channel");
        return;
    }
    srv.sendTo(target_fd, msg);
}

// --- OPER (bonus) ---
static void cmdOper(Server &srv, int fd, const std::vector<std::string> &p) {
    Client *c = srv.getClient(fd);
    if (p.size() < 2) {
        numeric(srv, fd, "461", "OPER :Not enough parameters");
        return;
    }
    if (p[0] == "root" && p[1] == srv.op_password && !srv.op_password.empty()) {
        c->is_oper = true;
        numeric(srv, fd, "221", c->nickname + " :You are now an IRC operator");
    } else {
        numeric(srv, fd, "481", ":Permission denied");
    }
}

// --- WHOIS (bonus) ---
static void cmdWhois(Server &srv, int fd, const std::vector<std::string> &p) {
    if (p.empty()) {
        numeric(srv, fd, "461", "WHOIS :Not enough parameters");
        return;
    }
    std::string nick = p[0];
    int target_fd = -1;
    for (std::map<int, Client*>::iterator it = srv.clients.begin();
         it != srv.clients.end(); ++it) {
        if (it->second->nickname.size() == nick.size() &&
            toLowerIrc(it->second->nickname) == toLowerIrc(nick)) {
            target_fd = it->first;
            break;
        }
    }
    if (target_fd < 0) {
        numeric(srv, fd, "401", nick + " :No such nick");
        return;
    }
    Client *t = srv.getClient(target_fd);
    numeric(srv, fd, "251", nick + " :is connected");
    numeric(srv, fd, "311", nick + " " + t->username + " " + t->hostname +
            " :*");
    numeric(srv, fd, "319", nick + " :is on");
    numeric(srv, fd, "371", nick + " :is a 42 student");
}

// --- WHO (bonus) ---
static void cmdWho(Server &srv, int fd, const std::vector<std::string> &p) {
    if (!p.empty()) {
        Channel *ch = srv.getChannel(p[0]);
        if (!ch) {
            numeric(srv, fd, "403", p[0] + " :No such channel");
            return;
        }
        numeric(srv, fd, "312", ":user host server :realname");
        for (std::set<int>::iterator it = ch->members.begin();
             it != ch->members.end(); ++it) {
            Client *c = srv.getClient(*it);
            if (c)
                numeric(srv, fd, "352",
                        p[0] + " " + c->nickname + " " + c->username +
                        " " + c->hostname + " ircserv * :" + c->realname);
        }
        numeric(srv, fd, "315", p[0] + " :End of WHO list");
    } else {
        numeric(srv, fd, "312", ":user host server :realname");
        for (std::map<int, Client*>::iterator it = srv.clients.begin();
             it != srv.clients.end(); ++it) {
            if (!it->second->isRegistered())
                continue;
            Client *c = it->second;
            numeric(srv, fd, "352",
                    "* " + c->nickname + " " + c->username + " " +
                    c->hostname + " ircserv * :" + c->realname);
        }
        numeric(srv, fd, "315", "* :End of WHO list");
    }
}

// --- CITEBOT / MARVINBOT (bonus) ---
static const char *QUOTES[] = {
    "Simplicity is the ultimate sophistication.",
    "Talk is cheap. Show me the code.",
    "Programs must be written for people to read.",
    "Premature optimization is the root of all evil."
};
static int quote_idx = 0;

static void cmdCitebot(Server &srv, int fd, const std::vector<std::string> &) {
    std::string q = QUOTES[quote_idx % 4];
    quote_idx++;
    reply(srv, fd, ":CITEBOT " + srv.nickOf(fd) + " :" + q);
}

static void cmdMarvinbot(Server &srv, int fd, const std::vector<std::string> &) {
    reply(srv, fd, ":MARVINBOT " + srv.nickOf(fd) + " :I am so depressed.");
}

void handleLine(Server &srv, int fd, const std::string &line) {
    std::string cmd;
    std::vector<std::string> params;
    parseLine(line, cmd, params);
    cmd = toUpper(cmd);

    Client *c = srv.getClient(fd);
    if (!c)
        return;

    // Commands allowed before registration
    if (cmd == "PASS") { cmdPass(srv, fd, params); return; }
    if (cmd == "NICK") { cmdNick(srv, fd, params); return; }
    if (cmd == "USER") { cmdUser(srv, fd, params); return; }
    if (cmd == "CAP")   { cmdCap(srv, fd, params); return; }
    if (cmd == "PING")  { cmdPing(srv, fd, params); return; }
    if (cmd == "QUIT")  { cmdQuit(srv, fd, params); return; }

    // Everything else requires registration
    if (!c->isRegistered()) {
        numeric(srv, fd, "461", cmd + " :You have not registered");
        return;
    }

    if (cmd == "JOIN")       cmdJoin(srv, fd, params);
    else if (cmd == "PART")  cmdPart(srv, fd, params);
    else if (cmd == "TOPIC") cmdTopic(srv, fd, params);
    else if (cmd == "NAMES") cmdNames(srv, fd, params);
    else if (cmd == "LIST")  cmdList(srv, fd, params);
    else if (cmd == "INVITE") cmdInvite(srv, fd, params);
    else if (cmd == "KICK")  cmdKick(srv, fd, params);
    else if (cmd == "MODE")  cmdMode(srv, fd, params);
    else if (cmd == "PRIVMSG") sendMessage(srv, fd, "PRIVMSG", params);
    else if (cmd == "NOTICE")  sendMessage(srv, fd, "NOTICE", params);
    else if (cmd == "OPER")  cmdOper(srv, fd, params);
    else if (cmd == "WHOIS") cmdWhois(srv, fd, params);
    else if (cmd == "WHO")   cmdWho(srv, fd, params);
    else if (cmd == "CITEBOT") cmdCitebot(srv, fd, params);
    else if (cmd == "MARVINBOT") cmdMarvinbot(srv, fd, params);
    else
        numeric(srv, fd, "421", cmd + " :Unknown command");
}

} // namespace ft_irc
