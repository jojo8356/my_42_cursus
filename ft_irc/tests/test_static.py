# ============================================================================ #
#  Tests — static: Makefile, structure, C++98, subject rules                 #
# ============================================================================ #
import os
from lib import check, expect, read_file, path_exists, PROJECT_ROOT


def test_makefile_exists_and_has_targets():
    mk = read_file("Makefile")
    for tgt in ("all", "clean", "re", "fclean"):
        check(tgt + ":" in mk, f"Makefile: cible '{tgt}:'")
    check("ircserv" in mk, "Makefile: binary 'ircserv'")


def test_makefile_cxx98_strict():
    mk = read_file("Makefile")
    check("-std=c++98" in mk, "Makefile: compile en C++98")
    check("-Wall" in mk and "-Wextra" in mk and "-Werror" in mk,
          "Makefile: flags stricts (-Wall -Wextra -Werror)")


def test_source_structure():
    for rel in (
        "srcs/main.cpp",
        "srcs/Server.cpp",
        "srcs/Client.cpp",
        "srcs/Channel.cpp",
        "srcs/Utils.cpp",
        "srcs/commands/Commands.cpp",
        "includes/Server.hpp",
        "includes/Client.hpp",
        "includes/Channel.hpp",
        "includes/Utils.hpp",
        "includes/Commands.hpp",
    ):
        check(path_exists(rel), f"structure: {rel} présent")


def test_single_poll_loop():
    """The server must use exactly one poll() for all I/O (subject rule)."""
    src = read_file("srcs/Server.cpp")
    check("poll(" in src, "Server.cpp: utilise poll()")
    # exactly one poll() call site in the run loop
    check(src.count("poll(") == 1,
          "Server.cpp: un seul poll() (multiplexage unique)",
          f"count={src.count('poll(')}")


def test_no_threads_no_fork():
    """No threads or fork — single process, single poll."""
    for rel in ("srcs/Server.cpp", "srcs/main.cpp"):
        txt = read_file(rel)
        expect("pthread" not in txt and "std::thread" not in txt,
               f"{rel}: pas de threads")
        expect("fork(" not in txt, f"{rel}: pas de fork()")


def test_tcp_ipv4():
    src = read_file("srcs/Server.cpp")
    check("AF_INET" in src, "Server.cpp: TCP/IPv4 (AF_INET)")
    check("SOCK_STREAM" in src, "Server.cpp: socket TCP (SOCK_STREAM)")


def test_non_blocking_sockets():
    src = read_file("srcs/Server.cpp")
    check("O_NONBLOCK" in src, "Server.cpp: sockets non-bloquants (O_NONBLOCK)")


def test_input_output_buffering():
    """Input buffer (partial lines) and output buffer (partial writes)."""
    src = read_file("srcs/Server.cpp")
    check("in_buffer" in src, "Server.cpp: buffer d'entrée (in_buffer)")
    check("out_buffer" in src, "Server.cpp: buffer de sortie (out_buffer)")
    check("find('\\n'" in src or "find('\\n'" in src or "'\\n'" in src,
          "Server.cpp: extraction des lignes complètes (\\n)")


def test_command_handlers_present():
    """All mandatory + bonus command handlers must be implemented."""
    src = read_file("srcs/commands/Commands.cpp")
    mandatory = ["PASS", "NICK", "USER", "JOIN", "PART", "MODE", "TOPIC",
                 "NAMES", "LIST", "INVITE", "KICK", "PRIVMSG", "NOTICE",
                 "PING", "QUIT", "CAP"]
    for cmd in mandatory:
        check(cmd in src, f"Commands.cpp: handler '{cmd}'")
    bonus = ["OPER", "WHOIS", "WHO", "CITEBOT", "MARVINBOT"]
    for cmd in bonus:
        check(cmd in src, f"Commands.cpp: bonus '{cmd}'")


def test_channel_modes_implemented():
    src = read_file("srcs/commands/Commands.cpp")
    for m in ("mode_i", "mode_t", "mode_k", "mode_l"):
        check(m in src, f"Commands.cpp: mode '{m}' implémenté")


def test_registration_numerics():
    src = read_file("srcs/commands/Commands.cpp")
    src2 = read_file("srcs/Server.cpp")
    combined = src + src2
    for num in ("001", "002", "003", "004", "005", "464", "433", "461"):
        check(num in combined, f"réponse numérique '{num}' implémentée")


def test_main_arg_parsing():
    src = read_file("srcs/main.cpp")
    check("argv" in src, "main.cpp: parse les arguments (port + password)")
    check("SIGINT" in src and "SIGQUIT" in src,
          "main.cpp: gère SIGINT/SIGQUIT (arrêt propre)")


def test_readme_and_subject():
    check(path_exists("README.md"), "README.md présent")
    check(path_exists("subject.md"), "subject.md présent")
