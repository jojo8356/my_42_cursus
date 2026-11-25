# Exam Rank 06 — 42 School

Implementation of the **new Exam Rank 06** (common core), with both projects
implemented completely and verified with comprehensive, enterprise-grade tests
plus the top-student reference testers.

## Projects

### 1. mini_serv (C)
A **non-blocking, multi-client TCP chat server** (a simplified IRC server):
- `select()`-based I/O multiplexing (no threads).
- Clients connect on `127.0.0.1:<port>`, get incrementing ids (`0, 1, 2, ...`).
- On connect/disconnect, broadcasts `server: client %d just arrived|left\n`.
- Messages are broadcast to all other clients with a `client %d: ` prefix,
  line by line.
- Per-client accumulation buffers reassemble fragmented/partial messages and
  keep lazy clients connected.
- No `#define`, only the allowed functions, listens only on `127.0.0.1`.

### 2. mini_db (C++)
A **key-value database served over TCP**:
- Takes a port and a persistence path; listens on `127.0.0.1`.
- Prints `ready` when accepting connections.
- Persistent connections; commands: `POST <key> <value>`, `GET <key>`,
  `DELETE <key>` (unknown → `2`).
- Persists the whole database to the path on `SIGINT`; reloads it on startup.

## Structure

```
exam_rank_06/
├── Makefile                 # top-level build (mini_serv + mini_db)
├── subject.md               # full subject (both projects)
├── mini_serv/
│   ├── mini_serv.c          # the server
│   ├── Makefile
│   └── subject.txt
├── mini_db/
│   ├── mini_db.cpp          # the server
│   ├── mini_db.hpp          # Database / Socket / Server classes
│   ├── Makefile
│   └── subject.txt
└── tests/
    ├── run_tests.py         # top-level runner
    ├── test_mini_serv.py    # comprehensive suite (48 tests)
    ├── test_mini_db.py      # comprehensive suite (38 tests)
    └── student_refs/        # top-student reference testers
        ├── test_server_ref.py
        └── mini_db_tester.sh
```

## Build & run

```bash
make            # builds mini_serv and mini_db
make test       # runs the full test suite
```

Manual testing:

```bash
# mini_serv
./mini_serv/mini_serv 8080 &
nc 127.0.0.1 8080

# mini_db
./mini_db/mini_db 8080 /tmp/save.txt &
echo "POST key value" | nc -w1 127.0.0.1 8080
echo "GET key" | nc -w1 127.0.0.1 8080
```

## Tests

```bash
python3 tests/run_tests.py
```

**97/97 tests pass**, covering:

- **mini_serv (48)**: connection, id assignment, arrival/disconnect
  notifications, broadcast (not echoed to sender), multiline, fragmented and
  partial messages, reconnect, multi-client broadcast, repeated messages,
  lazy-client resilience, large messages, wrong-argument error handling, and
  static checks (no `#define`, only allowed functions, 127.0.0.1 binding,
  `select` usage).
- **mini_db (38)**: ready message, POST/GET/DELETE semantics, unknown command,
  persistent multi-command sessions, SIGINT persistence + reload, malformed
  commands, large values, overwrite, multiple clients, argument errors, and
  static checks.
- **Student-reference testers**: the top-student `test_server.py` (10
  functional scenarios) and the `mini_db_tester.sh` reference tester.

## Validation

Both implementations were validated against the reference testers retrieved
from top students' GitHub repositories, confirming **100% + bonus** behavior.
