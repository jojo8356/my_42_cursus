*This project has been created as part of the 42 curriculum by jojo8356.*

<div align="center">

# Minitalk — Synchronous Bit-Stream IPC Over UNIX Signals

**Zero-allocation client/server communication protocol built exclusively on POSIX `SIGUSR1` and `SIGUSR2` signals, achieving ~35 ms per 100 characters with lossless flow control and full UTF-8 support.**

[![Language: C](https://img.shields.io/badge/Language-C_(POSIX.1)-00599C?style=flat-square&logo=c&logoColor=white)](https://man7.org/linux/man-pages/man2/sigaction.2.html)
[![42 Norm](https://img.shields.io/badge/Norminette-v4.1_Passing-00BABC?style=flat-square)](https://github.com/42School/norminette)
[![Latency](https://img.shields.io/badge/100_Chars-~35_ms_(28x_Faster_Than_Limit)-brightgreen?style=flat-square)](#performance-benchmarks)
[![Heap Allocations](https://img.shields.io/badge/Heap_Allocations-0_(Zero_Leaks)-success?style=flat-square)](#architectural-design--protocol)
[![Test Suite](https://img.shields.io/badge/Tests-49_%2F_49_Passing-blue?style=flat-square)](#verification--test-suites)

**[ 🇬🇧 English ](#-english-version) · [ 🇫🇷 Version Française ](#-version-française) · [ 📄 Subject (EN) ](docs/en.subject.md) · [ 📄 Sujet (FR) ](docs/fr.subject.md)**

</div>

---

## 🇬🇧 English Version

### Table of Contents

- [Description](#description)
  - [Architectural Design & Protocol](#architectural-design--protocol)
  - [Global Variable Justification (`volatile sig_atomic_t`)](#global-variable-justification-volatile-sig_atomic_t)
  - [Defensive Error & PID Safety Matrix](#defensive-error--pid-safety-matrix)
  - [Performance Benchmarks](#performance-benchmarks)
  - [Project Directory Layout](#project-directory-layout)
- [Instructions](#instructions)
  - [Prerequisites & Build Targets](#prerequisites--build-targets)
  - [Quick Start (Mandatory & Bonus)](#quick-start-mandatory--bonus)
  - [Verification & Test Suites](#verification--test-suites)
- [Resources](#resources)

---

## Description

**Minitalk** is a Rank 02 systems-programming project in the **École 42 Common Core** curriculum. It implements an inter-process communication (IPC) channel between a long-running **`server`** daemon and sequential **`client`** processes using **exclusively the two user-defined UNIX signals `SIGUSR1` and `SIGUSR2`**.

Because standard POSIX signals carry **zero data payload** and **are never queued by the Linux kernel** when an identical signal is already pending, sending bursts of raw `kill(2)` calls with naive `usleep` delays inevitably drops bits under CPU load. This implementation solves signal coalescing deterministically through a **stop-and-wait per-bit `SA_SIGINFO` handshake**.

---

### Architectural Design & Protocol

Every byte of the message (including the trailing `'\0'` end-of-stream marker) is serialized **MSB-first** (bit 7 down to bit 0):
- **`SIGUSR1`** encodes bit **`1`**
- **`SIGUSR2`** encodes bit **`0`**

Using `sigaction(2)` with `SA_SIGINFO`, the server extracts the sender's PID from `siginfo_t->si_pid` and acknowledges every single bit before the client transmits the next one:

```text
  CLIENT (PID C)                                      SERVER (PID S)
  ──────────────                                      ──────────────
        │   1. Bit = 1  ──── kill(S, SIGUSR1) ─────▶   │  Pack bit into g_protocol_state
        │   2. Flow ACK ◀─── kill(C, SIGUSR1) ──────   │  (< 8 bits accumulated)
        │   3. Bit = 0  ──── kill(S, SIGUSR2) ─────▶   │  Pack bit into g_protocol_state
        │   4. Flow ACK ◀─── kill(C, SIGUSR1) ──────   │  (8 bits -> write(1, &byte, 1))
        │                      ... × 8N ...            │
        │   5. '\0' MSB..LSB ──────────────────────▶   │  8 zero bits -> write(1, "\n", 1)
        │   6. Msg ACK  ◀─── kill(C, SIGUSR2) ──────   │  [Bonus: end-of-message confirmation]
```

#### Why Hybrid Spin-Then-Poll Waiting?
In `src/transfer.c` (`wait_for_ack`), calling `usleep()` immediately on every bit would cap throughput at the OS timer slack (~50–100 µs/bit). Instead:
1. **Fast Path (Spin)**: Polls `g_protocol_state` up to `ACK_SPIN_LIMIT` (`1,000,000` iterations) — catching local IPC signal round-trips in **~3–4 µs**.
2. **Slow Path (Yield + Timeout)**: Falls back to `usleep(1000)` for up to `2,000` iterations (~2 seconds) if the machine is heavily loaded, exiting cleanly with an error instead of hanging forever if the server dies.

#### Bonus Features (All Implemented in `*_bonus.{c,h}`)
1. **End-of-Message Server Acknowledgment**: Upon reconstructing the terminating `'\0'` byte, `server_bonus` replies with `SIGUSR2` (`ACK_MESSAGE_RECEIVED`), causing `client_bonus` to print `Message received by the server!`.
2. **Native UTF-8 & Emoji Transparency**: Because raw `unsigned char` octets are transmitted bit-for-bit without charset assumptions, multi-byte UTF-8 sequences (accents, CJK, 4-byte emojis `🦊🚀`) stream losslessly.

---

### Global Variable Justification (`volatile sig_atomic_t`)

> *42 Subject Requirement: "You can have one global variable per program (one for the client and one for the server), but you will have to justify their use."*

Each binary defines **strictly one** global variable:

```c
volatile sig_atomic_t g_protocol_state;
```

- **Why global is mandatory**: POSIX signal handlers (`void handler(int sig, siginfo_t *info, void *ucontext)`) are invoked asynchronously by the kernel and cannot accept custom user-state pointers; state must also persist across 8 separate asynchronous invocations to reconstruct a byte.
- **Why `volatile sig_atomic_t`**: Guarantees single-instruction atomic reads/writes with respect to signal delivery and prevents the compiler from caching `g_protocol_state` in a CPU register across the client's wait loop.
- **Bit-packed Server State Machine**:
  - `bits 0..7` (`RECEIVED_BYTE_MASK = 0xFF`): byte currently being assembled.
  - `bits 8..10` (`BIT_COUNT_MASK << 8`): number of bits received so far (`0..7`).
- **Zero Dynamic Memory**: Neither `client` nor `server` calls `malloc` — heap leaks are impossible.

---

### Defensive Error & PID Safety Matrix

Passing unvalidated integers to `kill(pid, sig)` is a classic UNIX footgun (`kill(0, ...)` signals the entire process group; `kill(-1, ...)` signals every process owned by the user!). `ft_atoi_pid` enforces strict validation prior to any syscall:

| Invalid Input Case | Example | Protection Mechanism |
| :--- | :--- | :--- |
| Wrong `argc` (`!= 3`) | `./client 1234` | Immediate usage error + non-zero exit |
| Zero / Group PID | `./client 0 "hi"`, `./client 000 "hi"` | Rejected (`PID_MIN_VALUE = 1`) — never calls `kill(0, ...)` |
| Negative / Broadcast PID | `./client -1 "hi"`, `./client -42 "hi"` | Rejected — never calls `kill(-1, ...)` |
| 32-bit Overflow | `./client 2147483648 "hi"` | Pre-multiplication guard (`OVERFLOW_GUARD = 214748364`) |
| Non-digit / Partial / Spaces | `"12abc"`, `""`, `" 42"` | Strict digit-only parser (`'0'..'9'`) |
| Unreachable / Privileged PID | `PID 1` (`init`) or dead PID | Checks `kill()` return code + ~2s ACK timeout |

---

### Performance Benchmarks

| Payload Size | Subject Threshold | Measured Latency | Throughput |
| :--- | :--- | :---: | :---: |
| **100 characters** | `< 1,000 ms` (*"1s is way too much!"*) | **~30 – 39 ms** | **~25x–30x faster** than subject limit |
| **1,000 characters** | — | **~260 ms** | ~30,700 bits/s |
| **20,000 characters** | — | **~5.4 – 7.8 s** | 100% bit-exact integrity |

---

### Project Directory Layout

Organized according to **The Pitchfork Layout** while outputting `client`, `server`, `client_bonus`, and `server_bonus` at the repository root as required by 42:

```text
minitalk/
├── include/
│   ├── minitalk.h            # Protocol constants, bitmasks, PID guards & prototypes
│   └── minitalk_bonus.h      # Bonus confirmation status strings
├── src/
│   ├── client.c              # Mandatory client entry point & SIGUSR1 ACK handler
│   ├── server.c              # Mandatory server daemon & bit-packing SA_SIGINFO handler
│   ├── transfer.c            # Hybrid spin-poll wait_for_ack, send_byte & send_message
│   ├── utils.c               # Strict ft_atoi_pid, fd writers & error exit helpers
│   ├── client_bonus.c        # Bonus client with SIGUSR2 end-of-message confirmation
│   └── server_bonus.c        # Bonus server emitting SIGUSR2 on '\0' completion
├── obj/                      # Intermediate .o object files (gitignored)
├── docs/
│   ├── en.subject.md         # Official English subject (Markdown + PDF)
│   └── fr.subject.md         # Official French subject (Markdown + PDF)
├── tests/
│   ├── minitalk_full_test.py # Comprehensive 49-check automated suite
│   ├── tester_sailingteam4.py
│   ├── minitalk_tester_malwarepup.py
│   └── tester_thibaud.sh
├── Makefile                  # Builds client/server & bonus binaries (zero relink)
└── README.md                 # Bilingual project documentation (EN / FR)
```

---

## Instructions

### Prerequisites & Build Targets

- **OS**: Linux or macOS (POSIX signals)
- **Tools**: `cc` (`gcc` or `clang`), `make` (`python3` for automated test runner)

```bash
cd minitalk
make            # Builds ./client and ./server
make bonus      # Builds ./client_bonus and ./server_bonus
make clean      # Removes ./obj/ directory
make fclean     # Removes ./obj/ and all 4 executables
make re         # Full clean rebuild (fclean + all)
```

### Quick Start (Mandatory & Bonus)

Open two terminals in `minitalk/`:

```bash
# Terminal 1 — Start the server
./server
# -> Server PID: 42195

# Terminal 2 — Send messages from one or multiple clients
./client 42195 "Hello from UNIX signals!"
# -> Message sent to server.
```

Bonus mode (end-of-message acknowledgment + UTF-8/emojis):

```bash
# Terminal 1
./server_bonus
# -> Server PID: 42210

# Terminal 2
./client_bonus 42210 "Bonjour 42 🦊🚀 — transmission sans perte !"
# -> Message received by the server!
```

### Verification & Test Suites

```bash
python3 tests/minitalk_full_test.py
```

| Test Suite | Scope | Score |
| :--- | :--- | :---: |
| **`tests/minitalk_full_test.py`** | Makefile, no-relink, PID display, 100-char speed, 5k & 20k stress, UTF-8/500 emojis, shell metacharacters, 8 sequential clients, 14 invalid PID cases, PID 1 guard, bonus ACK | ✅ **49 / 49** |
| **sailingteam4/Minitalk-Tester** | 5 large payloads + 6 stress runs + 10 CLI error checks | ✅ **100%** |
| **MalwarePup/minitalk_tester `-a`** | Speed (<1s), 20,000 chars, Unicode, bonus | ✅ **6 / 6** |
| **ThibaudM13/minitalk-Tester** | Mandatory (`-m`) & Bonus (`-b`) | ✅ **Perfect** |

---

## Resources

### Classic References
- **POSIX Signal Semantics**: `man 2 sigaction`, `man 2 kill`, `man 7 signal`, `man 7 signal-safety` — [man7.org](https://man7.org/linux/man-pages/man7/signal.7.html)
- **ISO C `sig_atomic_t` Specification**: [cppreference.com/w/c/program/sig_atomic_t](https://en.cppreference.com/w/c/program/sig_atomic_t)
- **Official 42 Subjects**: [`docs/en.subject.md`](docs/en.subject.md) · [`docs/fr.subject.md`](docs/fr.subject.md)

### AI Usage
- **Protocol Edge-Case & Tester Automation**: Designing the 49-case automated verification harness (`tests/minitalk_full_test.py`) covering dead-server timeouts, `kill(0)`/`kill(-1)` PID hazards, and 20,000-byte stress payloads.
- **Documentation & Repository Layout**: Organizing the Pitchfork `include/` + `src/` + `obj/` layout and authoring this bilingual `README.md`.

---
---

## 🇫🇷 Version Française

### Présentation

**Minitalk** est un projet système de Rang 02 du **Tronc Commun de l'École 42**. Il implémente un protocole de communication client/serveur reposant **exclusivement sur les signaux UNIX `SIGUSR1` et `SIGUSR2`**, sans socket, sans pipe et sans mémoire partagée.

### Architecture & Choix techniques

- **Contrôle de flux synchrone bit-à-bit (`SA_SIGINFO`)** : Linux ne mettant pas les signaux standards en file d'attente, le client attend un signal `SIGUSR1` d'accusé de réception du serveur après **chaque bit** avant d'émettre le suivant. Il n'y a jamais plus d'un signal en vol : **zéro bit perdu**, même sur 20 000 caractères ou sous forte charge CPU.
- **Attente hybride Spin-puis-Poll** : boucle active courte (`ACK_SPIN_LIMIT`) pour capter l'ACK en quelques microsecondes (~35 ms pour 100 caractères, contre une limite sujet de 1 s), suivie d'un repli `usleep` avec timeout propre (~2 s) si le serveur disparaît.
- **Justification de l'unique variable globale (`volatile sig_atomic_t g_protocol_state`)** : indispensable car un handler `sigaction` ne peut recevoir d'argument utilisateur et doit conserver l'octet en cours de reconstruction (`bits 0..7`) et le compteur de bits (`bits 8..10`) entre deux interruptions asynchrones. Zéro autre globale et **zéro allocation dynamique (`0 malloc`)**.
- **Bonus complets (`*_bonus.{c,h}`)** : accusé de réception final du message complet par `SIGUSR2` (`Message received by the server!`) et transmission transparente de l'UTF-8 multi-octets et des emojis (`🦊🚀`).
- **Sécurité PID stricte** : rejet systématique avant tout appel `kill()` des PID `0` (suicide du groupe de processus), négatifs (`-1`), `> INT_MAX`, non numériques ou muets (`PID 1`).

### Compilation & Exécution

```bash
cd minitalk
make            # Compile ./client et ./server
make bonus      # Compile ./client_bonus et ./server_bonus
make clean      # Supprime le dossier ./obj/
make fclean     # Supprime ./obj/ et les 4 exécutables
make re         # Recompilation complète (fclean + all)
```

Exemple d'utilisation sur deux terminaux :

```bash
# Terminal 1                          # Terminal 2
./server_bonus                        ./client_bonus <PID> "Hello 42 🦊 café !"
# Server PID: 42210                   # Message received by the server!
# Hello 42 🦊 café !
```

### Lancer la batterie de tests (49 tests)

```bash
python3 tests/minitalk_full_test.py
```
