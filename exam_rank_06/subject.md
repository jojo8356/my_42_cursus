# Exam Rank 06 — 42 School (common core, v2025)

The new **Exam Rank 06** comprises **two** projects, both in system
programming / networking:

1. **mini_serv** — a non-blocking, multi-client TCP chat server (C).
2. **mini_db** — a simple key-value database served over a network protocol (C++).

Both subjects below are the most recent and complete versions retrieved from
42 school references.

---

## 1. mini_serv

**Assignment name** : mini_serv
**Expected files** : `mini_serv.c`
**Allowed functions** : `write, close, select, socket, accept, listen, send,
recv, bind, strstr, malloc, realloc, free, calloc, bzero, atoi, sprintf,
strlen, exit, strcpy, strcat, memset, htons, htonl`

Write a program that will listen for clients to connect on a certain port on
`127.0.0.1` and will let clients speak with each other.

- Takes the port to bind as first argument.
- If no argument is given → write `Wrong number of arguments` + `\n` to stderr,
  exit with status 1.
- If a system call returns an error before accepting connections → write
  `Fatal error` + `\n` to stderr, exit with status 1.
- If memory cannot be allocated → write `Fatal error` + `\n` to stderr, exit 1.

- The program must be **non-blocking**; a lazy client that does not read your
  message must **NOT** be disconnected.
- Must **not contain `#define`** preprocessor directives.
- Must only listen on `127.0.0.1`.
- The received fd is already non-blocking for `recv`/`send` when `select` has
  not been called; use `select` and **never** check `EAGAIN`.

**When a client connects:**
- It is given an id; the first client receives id `0`, each new client gets
  `last id + 1`. `%d` is replaced by this number.
- A message is sent to all connected clients: `server: client %d just arrived\n`.

**Clients can send messages:**
- Messages are printable characters only (no validation needed).
- A single message may contain multiple `\n`.
- When the server receives a message, it resends it to all **other** clients
  with `client %d: ` before every line.

**When a client disconnects:**
- A message is sent to all connected clients: `server: client %d just left\n`.

**Memory or fd leaks are forbidden.** The tester expects messages to be sent as
fast as possible (no unnecessary buffering). Hint: use `nc` to test.

---

## 2. mini_db

**Assignment name** : mini_db
**Expected files** : `*.cpp *.hpp`

Write a program that takes as arguments a **port** and a **path**. It must
listen for clients on `127.0.0.1` on the given port.

- When ready to accept connections, print `ready\n` on standard output.
- Maintain a database of key-values.
- Connections are **persistent**: a client can send multiple commands in one
  session.

**Commands:**

| Command | Synopsis | Description | Return value |
|---------|----------|-------------|--------------|
| POST | `POST <key> <value>` | add an entry to the db | `0` |
| GET | `GET <key>` | retrieve an entry | `0 <value>` or `1` if missing |
| DELETE | `DELETE <key>` | erase an entry | `0` if erased, `1` if missing |

- An unrecognized command must respond `2`.
- Keys and values are of arbitrary size and contain no whitespace; a request
  never exceeds 1000 characters.
- The program must be efficient.

**Data persistence:**
- On `SIGINT`, save the whole database to the path argument and exit.
- On startup, look for that file and load it.

Hint: use `nc` to test.
