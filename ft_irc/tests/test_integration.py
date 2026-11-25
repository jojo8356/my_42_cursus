# ============================================================================ #
#  Tests — integration: real server + real socket clients                    #
# ============================================================================ #
import time
from lib import (check, expect, IrcClient, start_server, stop_server)

PW = "secret123"
OP = "oppass"


def _reg(port, nick, user=None, pw=PW):
    """Register a client and return it (drained)."""
    c = IrcClient(port, password=pw, nick=nick,
                  user=user if user else nick)
    return c


def test_registration_replies():
    proc = start_server(17001, PW, OP)
    try:
        c = _reg(17001, "alice")
        out = c.buf.decode(errors="replace")
        for num in ("001", "002", "003", "004", "005"):
            check(num in out, f"réponse de registration '{num}' reçue", out)
    finally:
        c.close(); stop_server(proc)


def test_wrong_password_464():
    proc = start_server(17002, PW, OP)
    try:
        c = IrcClient(17002, password="wrongpass", nick="bob", user="bob")
        out = c.buf.decode(errors="replace")
        check("464" in out, "mot de passe incorrect -> 464", out)
    finally:
        c.close(); stop_server(proc)


def test_nickname_taken_433():
    proc = start_server(17003, PW, OP)
    try:
        a = _reg(17003, "alice")
        b = IrcClient(17003, password=PW, nick="alice", user="bob")
        out = b.buf.decode(errors="replace")
        check("433" in out, "nickname déjà pris -> 433", out)
    finally:
        a.close(); b.close(); stop_server(proc)


def test_invalid_nick_432():
    proc = start_server(17004, PW, OP)
    try:
        c = IrcClient(17004, password=PW, nick="bad*nick", user="bad")
        out = c.buf.decode(errors="replace")
        check("432" in out, "nickname invalide -> 432", out)
    finally:
        c.close(); stop_server(proc)


def test_not_registered_461():
    proc = start_server(17005, PW, OP)
    try:
        c = IrcClient(17005)  # no registration
        c.send("JOIN #test")
        out = c.recv_until("461")
        check("461" in out, "commande hors registration -> 461", out)
    finally:
        c.close(); stop_server(proc)


def test_ping_pong():
    proc = start_server(17006, PW, OP)
    try:
        c = _reg(17006, "alice")
        c.send("PING abc123")
        out = c.recv_until("PONG")
        check("PONG" in out and "abc123" in out, "PING -> PONG :token", out)
    finally:
        c.close(); stop_server(proc)


def test_join_creates_channel_operator():
    proc = start_server(17007, PW, OP)
    try:
        c = _reg(17007, "alice")
        c.send("JOIN #general")
        out = c.recv_until("366")
        check("353" in out, "JOIN -> 353 (NAMES)", out)
        check("@alice" in out, "créateur est opérateur (@alice)", out)
        check("366" in out, "JOIN -> 366 (fin NAMES)", out)
    finally:
        c.close(); stop_server(proc)


def test_two_clients_see_each_other():
    proc = start_server(17008, PW, OP)
    try:
        a = _reg(17008, "alice")
        a.send("JOIN #lobby"); a.recv_until("366")
        b = _reg(17008, "bob")
        b.send("JOIN #lobby"); b.recv_until("366")
        # a should have received bob's JOIN broadcast + NAMES
        out = a.recv_all(0.5)
        check("bob" in out, "client A voit client B dans le canal", out)
        check("JOIN #lobby" in out or "353" in out, "diffusion JOIN/NAMES reçue", out)
    finally:
        a.close(); b.close(); stop_server(proc)


def test_privmsg_channel_broadcast():
    proc = start_server(17009, PW, OP)
    try:
        a = _reg(17009, "alice"); a.send("JOIN #chat"); a.recv_until("366")
        b = _reg(17009, "bob"); b.send("JOIN #chat"); b.recv_until("366")
        a.send("PRIVMSG #chat :hello everyone")
        out = b.recv_until("hello everyone")
        check("hello everyone" in out, "PRIVMSG canal diffusé à B", out)
        check("alice" in out and "PRIVMSG" in out, "message avec préfixe nick", out)
    finally:
        a.close(); b.close(); stop_server(proc)


def test_privmsg_private():
    proc = start_server(17010, PW, OP)
    try:
        a = _reg(17010, "alice")
        b = _reg(17010, "bob")
        a.send("PRIVMSG bob :secret message")
        out = b.recv_until("secret message")
        check("secret message" in out, "PRIVMSG privé reçu par B", out)
        check("alice" in out, "PRIVMSG privé avec préfixe", out)
    finally:
        a.close(); b.close(); stop_server(proc)


def test_privmsg_no_such_user_401():
    proc = start_server(17011, PW, OP)
    try:
        c = _reg(17011, "alice")
        c.send("PRIVMSG nobody :hi")
        out = c.recv_until("401")
        check("401" in out, "PRIVMSG utilisateur inexistant -> 401", out)
    finally:
        c.close(); stop_server(proc)


def test_mode_invite_only():
    proc = start_server(17012, PW, OP)
    try:
        a = _reg(17012, "alice"); a.send("JOIN #inc"); a.recv_until("366")
        a.send("MODE #inc +i"); a.recv_until("351")
        b = _reg(17012, "bob")
        b.send("JOIN #inc")
        out = b.recv_until("473")
        check("473" in out, "JOIN canal +i sans invitation -> 473", out)
        # operator invites bob
        a.send("INVITE bob #inc"); a.recv_until("341")
        b.recv_all(0.3)
        b.send("JOIN #inc")
        out2 = b.recv_until("353")
        check("353" in out2, "JOIN canal +i après invitation -> 353", out2)
    finally:
        a.close(); b.close(); stop_server(proc)


def test_mode_key():
    proc = start_server(17013, PW, OP)
    try:
        a = _reg(17013, "alice"); a.send("JOIN #key"); a.recv_until("366")
        a.send("MODE #key +k mykey"); a.recv_until("351")
        b = _reg(17013, "bob")
        b.send("JOIN #key wrong")
        out = b.recv_until("475")
        check("475" in out, "JOIN avec mauvaise clé -> 475", out)
        b.recv_all(0.2)
        b.send("JOIN #key mykey")
        out2 = b.recv_until("353")
        check("353" in out2, "JOIN avec bonne clé -> 353", out2)
    finally:
        a.close(); b.close(); stop_server(proc)


def test_mode_limit():
    proc = start_server(17014, PW, OP)
    try:
        a = _reg(17014, "alice"); a.send("JOIN #lim"); a.recv_until("366")
        a.send("MODE #lim +l 1"); a.recv_until("351")
        b = _reg(17014, "bob")
        b.send("JOIN #lim")
        out = b.recv_until("471")
        check("471" in out, "JOIN dépassement limite -> 471", out)
    finally:
        a.close(); b.close(); stop_server(proc)


def test_mode_operator_and_kick():
    proc = start_server(17015, PW, OP)
    try:
        a = _reg(17015, "alice"); a.send("JOIN #op"); a.recv_until("366")
        b = _reg(17015, "bob"); b.send("JOIN #op"); b.recv_until("366")
        # bob is not operator -> kick fails
        b.clear()
        b.send("KICK #op alice :nope")
        out = b.recv_until("481")
        check("481" in out, "KICK par non-opérateur -> 481", out)
        # alice (operator) kicks bob
        a.clear()
        a.send("KICK #op bob :bye")
        out2 = a.recv_until("KICK")
        check("KICK" in out2 and "bob" in out2, "KICK par opérateur réussi", out2)
    finally:
        a.close(); b.close(); stop_server(proc)


def test_topic_view_set_and_restrict():
    proc = start_server(17016, PW, OP)
    try:
        a = _reg(17016, "alice"); a.send("JOIN #top"); a.recv_until("366")
        a.send("TOPIC #top :hello topic")
        a.recv_until("TOPIC")
        a.send("TOPIC #top")
        out = a.recv_until("332")
        check("332" in out and "hello topic" in out, "TOPIC visualisé (332)", out)
        # set +t, then bob (non-op) cannot change topic
        a.send("MODE #top +t"); a.recv_until("351")
        b = _reg(17016, "bob"); b.send("JOIN #top"); b.recv_until("366")
        b.send("TOPIC #top :hacked")
        out2 = b.recv_until("482")
        check("482" in out2, "TOPIC par non-opérateur en +t -> 482", out2)
    finally:
        a.close(); b.close(); stop_server(proc)


def test_part_destroys_channel():
    proc = start_server(17017, PW, OP)
    try:
        a = _reg(17017, "alice"); a.send("JOIN #temp"); a.recv_until("366")
        b = _reg(17017, "bob"); b.send("JOIN #temp"); b.recv_until("366")
        b.clear()
        a.send("PART #temp :leaving")
        out = b.recv_until("PART")
        check("PART" in out, "PART annoncé aux autres membres", out)
        # channel should be gone (a re-joins alone -> recreated as new channel)
        a.clear()
        a.send("JOIN #temp")
        out2 = a.recv_until("353")
        check("353" in out2, "canal recréé après PART (dernier membre parti)", out2)
    finally:
        a.close(); b.close(); stop_server(proc)


def test_names_command():
    proc = start_server(17018, PW, OP)
    try:
        a = _reg(17018, "alice"); a.send("JOIN #n"); a.recv_until("366")
        b = _reg(17018, "bob"); b.send("JOIN #n"); b.recv_until("366")
        a.clear()
        a.send("NAMES #n")
        out = a.recv_until("366")
        check("353" in out and "alice" in out and "bob" in out,
              "NAMES liste les membres (353)", out)
    finally:
        a.close(); b.close(); stop_server(proc)


def test_list_command():
    proc = start_server(17019, PW, OP)
    try:
        a = _reg(17019, "alice"); a.send("JOIN #l1"); a.recv_until("366")
        a.send("JOIN #l2"); a.recv_until("366")
        a.send("LIST")
        out = a.recv_until("323")
        check("321" in out, "LIST -> 321 (entête)", out)
        check("322" in out and "#l1" in out, "LIST -> 322 (canal)", out)
        check("323" in out, "LIST -> 323 (fin)", out)
    finally:
        a.close(); stop_server(proc)


def test_quit_announced():
    proc = start_server(17020, PW, OP)
    try:
        a = _reg(17020, "alice"); a.send("JOIN #q"); a.recv_until("366")
        b = _reg(17020, "bob"); b.send("JOIN #q"); b.recv_until("366")
        a.close()  # sends QUIT
        out = b.recv_until("QUIT")
        check("QUIT" in out and "alice" in out, "QUIT annoncé au canal", out)
    finally:
        b.close(); stop_server(proc)


def test_oper_bonus():
    proc = start_server(17021, PW, OP)
    try:
        c = _reg(17021, "alice")
        c.send("OPER root wrongpass")
        out = c.recv_until("481")
        check("481" in out, "OPER mauvais mot de passe -> 481", out)
        c.recv_all(0.2)
        c.send("OPER root oppass")
        out2 = c.recv_until("221")
        check("221" in out2, "OPER bon mot de passe -> 221", out2)
    finally:
        c.close(); stop_server(proc)


def test_whois_bonus():
    proc = start_server(17022, PW, OP)
    try:
        a = _reg(17022, "alice")
        b = _reg(17022, "bob")
        a.send("WHOIS bob")
        out = a.recv_until("371")
        check("251" in out, "WHOIS -> 251", out)
        check("311" in out, "WHOIS -> 311 (identité)", out)
        check("371" in out, "WHOIS -> 371 (fin)", out)
    finally:
        a.close(); b.close(); stop_server(proc)


def test_who_bonus():
    proc = start_server(17023, PW, OP)
    try:
        a = _reg(17023, "alice"); a.send("JOIN #w"); a.recv_until("366")
        b = _reg(17023, "bob"); b.send("JOIN #w"); b.recv_until("366")
        a.send("WHO #w")
        out = a.recv_until("315")
        check("352" in out, "WHO -> 352 (utilisateur)", out)
        check("315" in out, "WHO -> 315 (fin)", out)
    finally:
        a.close(); b.close(); stop_server(proc)


def test_citebot_bonus():
    proc = start_server(17024, PW, OP)
    try:
        c = _reg(17024, "alice")
        c.send("CITEBOT")
        out = c.recv_until("CITEBOT")
        check("CITEBOT" in out, "CITEBOT répond (bonus)", out)
    finally:
        c.close(); stop_server(proc)


def test_cap_ls_bonus():
    proc = start_server(17025, PW, OP)
    try:
        c = IrcClient(17025)
        c.send("CAP LS 302")
        out = c.recv_until("CAP LS")
        check("CAP LS" in out, "CAP LS 302 répond (bonus)", out)
    finally:
        c.close(); stop_server(proc)


def test_join_zero_leaves_all():
    proc = start_server(17026, PW, OP)
    try:
        a = _reg(17026, "alice")
        a.send("JOIN #a"); a.recv_until("366")
        a.send("JOIN #b"); a.recv_until("366")
        a.send("JOIN 0")
        a.recv_all(0.4)
        a.send("NAMES #a")
        out = a.recv_until("403")
        check("403" in out or "353" not in out,
              "JOIN 0 quitte tous les canaux (bonus)", out)
    finally:
        a.close(); stop_server(proc)


def test_multi_channel_join_part():
    proc = start_server(17027, PW, OP)
    try:
        a = _reg(17027, "alice")
        b = _reg(17027, "bob")
        a.send("JOIN #m1,#m2")
        a.recv_until("366")
        b.send("JOIN #m1,#m2"); b.recv_until("366")
        b.clear()
        a.send("PART #m1,#m2 :done")
        out2 = b.recv_until("PART")
        check("PART" in out2, "PART multi-canaux annoncé aux membres", out2)
    finally:
        a.close(); b.close(); stop_server(proc)


def test_case_insensitive_nick():
    proc = start_server(17028, PW, OP)
    try:
        a = _reg(17028, "Alice")
        b = IrcClient(17028, password=PW, nick="alice", user="bob2")
        out = b.buf.decode(errors="replace")
        check("433" in out, "nickname insensible à la casse -> 433", out)
    finally:
        a.close(); b.close(); stop_server(proc)


def test_unknown_command_421():
    proc = start_server(17029, PW, OP)
    try:
        c = _reg(17029, "alice")
        c.send("FOOBAR xyz")
        out = c.recv_until("421")
        check("421" in out, "commande inconnue -> 421", out)
    finally:
        c.close(); stop_server(proc)


def test_multiple_clients_simultaneous():
    proc = start_server(17030, PW, OP)
    try:
        clients = []
        for i, nick in enumerate(["u1", "u2", "u3", "u4"]):
            c = _reg(17030, nick)
            c.send("JOIN #multi")
            c.recv_until("366")
            clients.append(c)
        # each should see the others
        out = clients[0].recv_all(0.6)
        for nick in ("u2", "u3", "u4"):
            check(nick in out, f"client multi vu : {nick}", out)
    finally:
        for c in clients:
            c.close()
        stop_server(proc)
