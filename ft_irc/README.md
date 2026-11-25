# ft_irc — 42 School

**Serveur IRC** en **C++98**, conforme au sujet ft_irc. Gère **plusieurs
clients simultanément** via **un seul `poll()`** (sockets TCP/IPv4 non-bloquants,
pas de threads, pas de fork).

## Caractéristiques
- ✅ **Un seul `poll()`** pour toutes les opérations (read/write/accept).
- ✅ Sockets **TCP/IP v4** non-bloquants.
- ✅ **Buffers d'entrée/sortie** (gestion des lignes partiellement reçues et
  des écritures partielles).
- ✅ **Multiples clients** simultanés, sans bloquage.

## Commandes implémentées
**Obligatoires** : PASS, NICK, USER, JOIN, PART, MODE, TOPIC, NAMES, LIST,
INVITE, KICK, PRIVMSG, NOTICE, PING, QUIT, CAP LS 302.

**Bonus** : OPER, WHOIS, WHO, CAP (LS/REQ/END), JOIN 0, JOIN/PART multi-canaux,
CITEBOT, MARVINBOT, MOTD complet (001-005), arrêt propre sur SIGINT/SIGQUIT.

## Modes canaux
| Mode | Effet |
|------|-------|
| `+i` | Canal réservé aux invités |
| `+t` | Topic réservé aux opérateurs |
| `+k <key>` | Clé de canal |
| `+o <nick>` | Donner/retirer le statut opérateur |
| `+l <n>` | Limite de membres |

(les modes `p,s,n,m,b,v` sont acceptés en bonus.)

## Compilation
```bash
cd ft_irc
make          # -> ./ircserv
make clean    # nettoyage
```

## Utilisation
```bash
./ircserv <port> <password> [operator_password]
# ex.
./ircserv 6667 secret123 oppass
```

## Connexion (nc)
```bash
nc -C 127.0.0.1 6667
PASS secret123
NICK alice
USER alice 0 * :Alice
JOIN #general
PRIVMSG #general :hello everyone
PING 1
QUIT :bye
```

## Structure
```
ft_irc/
├── Makefile
├── README.md
├── subject.md
├── includes/
│   ├── Server.hpp      # socket, poll loop, clients, canaux
│   ├── Client.hpp      # état client, buffers
│   ├── Channel.hpp     # membres, opérateurs, modes
│   ├── Commands.hpp    # dispatch des commandes
│   └── Utils.hpp       # helpers (split, casefold, validation)
├── srcs/
│   ├── main.cpp        # arguments, signaux
│   ├── Server.cpp      # poll loop, accept/read/write, helpers
│   ├── Client.cpp
│   ├── Channel.cpp
│   ├── Utils.cpp
│   └── commands/Commands.cpp   # tous les handlers
└── tests/              # suite de tests (statique + intégration)
```

## Tests
Suite complète **statique + intégration réelle** :
```bash
python3 tests/run_tests.py
```
- **Statique** : Makefile (C++98 strict), structure, un seul `poll()`, pas de
  threads/fork, buffers, tous les handlers, numerics.
- **Intégration** : le serveur est **compilé et démarré** ; des vrais clients
  socket testent **chaque commande** et **chaque mode** (i/t/k/o/l), les
  messages privés/canal, KICK/INVITE, TOPIC, NAMES, LIST, QUIT, et tous les
  bonus (OPER, WHOIS, WHO, CITEBOT, CAP, JOIN 0, multi-canaux, casse).
