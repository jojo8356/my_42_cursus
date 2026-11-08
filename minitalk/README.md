# minitalk — 42

Petit programme d'échange de données entre un **client** et un **serveur** en
utilisant **uniquement les signaux UNIX** `SIGUSR1` et `SIGUSR2` (projet du
rang 02 du common core de l'École 42).

Énoncé : [`docs/fr.subject.md`](docs/fr.subject.md) ·
[`docs/en.subject.md`](docs/en.subject.md) · PDF officiels dans [`docs/`](docs/)

---

## Compilation et utilisation

```sh
make            # compile ./client et ./server (partie obligatoire)
make bonus      # compile ./client_bonus et ./server_bonus
make clean      # supprime les *.o
make fclean     # supprime *.o + exécutables
make re         # fclean + all
```

Flags : `cc -Wall -Wextra -Werror` — le Makefile ne relinke pas.

```sh
# terminal 1                        # terminal 2
./server                            ./client <PID_DU_SERVEUR> "Bonjour !"
# Server PID: 12345                 # Message sent to server.
# Bonjour !
```

Bonus :

```sh
./server_bonus                      ./client_bonus <PID> "hello 🦊 café"
# Server PID: 12345                 # Message received by the server!
# hello 🦊 café
```

---

## Comment ça marche

Un signal ne transporte aucune donnée : on encode donc le message **bit par
bit**. Chaque octet est envoyé en 8 signaux, du bit de poids fort au bit de
poids faible :

| Signal   | Bit |
|----------|-----|
| SIGUSR1  | 1   |
| SIGUSR2  | 0   |

La fin de message est marquée par un octet `\0`.

```
   CLIENT                                SERVEUR
     │  bit = 1  ──── SIGUSR1 ──────────▶  │ reconstruit l'octet
     │  ◀──────────── SIGUSR1 (ACK) ────   │ (8 bits -> 1 caractère)
     │  bit = 0  ──── SIGUSR2 ──────────▶  │ puis l'affiche avec write()
     │  ◀──────────── SIGUSR1 (ACK) ────   │
     │              ...  ×8 ...            │
     │  '\0'      ──── ... ─────────────▶  │ fin de message : '\n'
     │  ◀──────────── SIGUSR2 (ACK msg) ─  │  [bonus]
```

### Pourquoi un accusé de réception à chaque bit ?

Le sujet prévient : *« Linux ne met PAS les signaux en file d'attente
lorsqu'il y a déjà un signal de ce type en attente »*. Envoyer les signaux à
la chaîne sans synchronisation fait donc perdre des bits (surtout avec des
messages longs ou la charge machine).

Ici, le client **attend l'ACK du serveur après chaque bit** avant d'envoyer
le suivant : il n'y a donc jamais plus d'un signal en vol, aucun signal n'est
perdu, et le flux est auto-régulé. Le client abandonne avec une erreur après
~2 s sans réponse (serveur mort : pas de blocage infini).

### Vitesse

L'ACK coûte un aller-retour de signaux (~30 µs) par bit :

| Test                          | Résultat mesuré |
|-------------------------------|-----------------|
| 100 caractères (limite sujet) | **~30 ms** (limite : « 1 s c'est colossal ») |
| 1 000 caractères              | ~0,26 s         |
| 20 000 caractères             | ~5,4 s          |

---

## Partie bonus

1. **Accusé de réception** : `server_bonus` renvoie un signal au client pour
   chaque message reçu (le `\0` final déclenche un `SIGUSR2` de confirmation
   en plus des ACK de flux `SIGUSR1`). `client_bonus` affiche alors
   `Message received by the server!` — la preuve visible que le serveur a
   bien répondu.
2. **Unicode** : les octets sont transmis tels quels (transparent au
   charset) : accents, UTF-8 multioctets et emojis passent sans altération.

Les fichiers bonus sont bien séparés (`_bonus.{c/h}`) comme l'exige le sujet.

---

## Variable globale — justification (exigée par le sujet)

> « Vous pouvez utiliser une variable globale par programme (une pour le
> client et une pour le serveur) mais leur usage doit être justifié. »

Chaque programme possède **exactement une** variable globale,
`volatile sig_atomic_t g_protocol_state` :

- **client** : elle contient le code d'accusé de réception renvoyé par le
  serveur (`ACK_NONE` = en attente, `ACK_BIT_RECEIVED` = ACK de flux,
  `ACK_MESSAGE_RECEIVED` = ACK de message).
- **serveur** : elle contient la machine à états de réception (octet en cours
  dans les bits 0-7 via `RECEIVED_BYTE_MASK`, nombre de bits reçus dans les
  bits 8-10 via `BIT_COUNT_MASK`/`BIT_COUNT_SHIFT`).

Elle **doit** être globale pour deux raisons :

1. un gestionnaire de signaux (`sigaction`/`SA_SIGINFO`) ne peut pas recevoir
   de paramètres supplémentaires ;
2. l'état doit survivre entre deux appels asynchrones du handler.

Elle est `volatile sig_atomic_t` : le type garantit des accès atomiques et
empêche le compilateur de mettre la valeur en cache dans un registre entre le
flux principal et le handler. Le reste du code ne possède **aucune** autre
variable globale et **n'alloue aucun** mémoire dynamique (donc aucun leak
possible).

---

## Gestion des erreurs

- Nombre d'arguments différent de 2 → message d'erreur + exit ≠ 0.
- PID invalide : vide, non numérique, signé, décimal partiel (`12abc`), `0`
  (dangereux : `kill(0, …)` tue tout le groupe de processus), négatif
  (`kill(-1, …)` : tous les processus), ou > `INT_MAX` → rejeté avant tout
  envoi.
- Serveur injoignable (`kill` échoue) ou muet (timeout ~2 s) → erreur propre.
- Tous les messages d'erreur sortent sur **stdout** (comportement `ft_printf`
  de l'école) et le programme ne quitte jamais de manière inattendue.

---

## Structure du projet

```
minitalk/
├── Makefile
├── minitalk.h            # protocole + toutes les constantes nommées
│                         # (ACK_*, masques, limites PID, délais) + g_protocol_state
├── minitalk_bonus.h      # header bonus (messages d'ACK affichés)
├── utils.c               # ft_strlen, ft_putstr_fd, ft_putnbr_fd,
│                         # ft_atoi_pid (strict), print_error_and_exit
├── transfer.c            # wait_for_ack (timeout), send_byte, send_message
├── client.c              # client obligatoire (handle_server_ack, …)
├── server.c              # serveur obligatoire (handle_received_bit, …)
├── client_bonus.c        # client + affichage de l'ACK (bonus)
├── server_bonus.c        # serveur + ACK de message SIGUSR2 (bonus)
├── docs/
│   ├── fr.subject.pdf / fr.subject.md     # énoncé français
│   └── en.subject.pdf / en.subject.md     # énoncé anglais
└── tests/
    ├── minitalk_full_test.py              # ma batterie complète (49 tests)
    ├── tester_sailingteam4.py             # tester communauté #1
    ├── minitalk_tester_malwarepup.py      # tester communauté #2
    └── tester_thibaud.sh                  # tester communauté #3
```

Conforme à la Norme 42 (norminette 3.3.60 : **0 erreur**), fonctions ≤ 25
lignes, ≤ 5 fonctions par fichier, que des fonctions autorisées par le sujet :
`write`, `sigemptyset`, `sigaddset`, `sigaction`, `kill`, `getpid`, `pause`,
`usleep`, `exit`.

---

## Tests

```sh
python3 tests/minitalk_full_test.py     # batterie complète : 49 tests
```

Résultats obtenus :

| Batterie                                          | Résultat |
|---------------------------------------------------|----------|
| `tests/minitalk_full_test.py` (ma batterie)       | **49/49** |
| sailingteam4/Minitalk-Tester                      | **100%** — 0 échec (5 pastebin + 6 stress + 10 parsing) |
| MalwarePup/minitalk_tester `-a`                   | **6/6** (100 chars en 0.03 s, 20 000 chars, unicode…) |
| ThibaudM13/minitalk-Tester `-m` / `-b`            | **Perfect, well done!** (obligatoire + bonus) |

Couverture de ma batterie : Makefile (flags, no-relink, toutes les règles),
norminette, PID affiché, message simple, chaîne vide, **vitesse 100 chars**,
intégrité 5 000 et 20 000 caractères, unicode/emojis, caractères spéciaux
(`\n \t " ' % \ {}`), métacaractères shell, 8 clients successifs sur le même
serveur, 14 cas de parsing/erreurs, bonus (ACK + unicode + 500 emojis +
3 000 chars), tolérance client_bonus↔server, rejet du PID 1.
