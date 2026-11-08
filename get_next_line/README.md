*This project has been created as part of the 42 curriculum by jojo8356.*

<div align="center">

# get_next_line — Amortized $O(L)$ Line-by-Line Stream Reader

**Zero-leak, arbitrary-buffer POSIX file-descriptor line reader (`char *get_next_line(int fd)`) with full multi-FD bonus using a single static variable.**

[![Language: C](https://img.shields.io/badge/Language-C_(ISO_C89%2FC99)-00599C?style=flat-square&logo=c&logoColor=white)](https://en.cppreference.com/w/c)
[![Subject](https://img.shields.io/badge/42_Subject-v14.3_(2026)-00BABC?style=flat-square)](docs/en.subject.md)
[![42 Norm](https://img.shields.io/badge/Norminette-v4.1_Passing-brightgreen?style=flat-square)](https://github.com/42School/norminette)
[![Bonus](https://img.shields.io/badge/Bonus-1_Static_%C2%B7_Unlimited_Multi--FD-success?style=flat-square)](#algorithm)
[![Testers](https://img.shields.io/badge/Verified-Tripouille_%C2%B7_Francinette_Strict_%C2%B7_ASan-blue?style=flat-square)](#verification--test-suites)

**[ 🇬🇧 English ](#-english-version) · [ 🇫🇷 Version Française ](#-version-française) · [ 📄 Subject v14.3 (EN) ](docs/en.subject.md) · [ 📄 Sujet v14.3 (FR) ](docs/fr.subject.md)**

</div>

---

## 🇬🇧 English Version

### Table of Contents

- [Description](#description)
  - [Project Architecture](#project-architecture)
  - [API Specification](#api-specification)
- [Instructions](#instructions)
  - [Prerequisites](#prerequisites)
  - [Compiling Mandatory & Bonus](#compiling-mandatory--bonus)
  - [Minimal Working Example](#minimal-working-example)
  - [Verification & Test Suites](#verification--test-suites)
- [Algorithm](#algorithm)
  - [Design Goals & Invariants](#design-goals--invariants)
  - [Amortized $O(L)$ Direct-Read Stash (Mandatory)](#amortized-ol-direct-read-stash-mandatory)
  - [Single-Static-Pointer Dynamic Multi-FD List (Bonus)](#single-static-pointer-dynamic-multi-fd-list-bonus)
  - [Complexity & Memory-Safety Justification](#complexity--memory-safety-justification)
- [Resources](#resources)

---

### Description

**`get_next_line`** is a Rank 01 project of the **École 42 Common Core** (*Subject Version 14.3*). Its goal is to implement `char *get_next_line(int fd)` — a function that returns **one complete line per call** read from any valid POSIX file descriptor (regular files, `stdin`, pipes, FIFOs, or sockets), while teaching persistent state management via **C `static` storage duration** and strict heap lifecycle discipline (`read`, `malloc`, `free` only).

#### Key Capabilities (Mandatory + Full Bonus)
- **Exact Line Semantics**: Returns each line including its terminating `\n`, or the final characters without `\n` when `EOF` is reached cleanly. Returns `NULL` at `EOF` or upon any I/O / allocation error.
- **`BUFFER_SIZE` Agnostic**: Compiles with or without `-D BUFFER_SIZE=n` (default `42`). Handles extreme values from `BUFFER_SIZE=1` up to `BUFFER_SIZE=10000000` (10 MB) without stack overflow, and safely rejects `BUFFER_SIZE <= 0` at runtime.
- **Minimal Read Guarantee**: Stops calling `read(2)` immediately as soon as `\n` is present in the stash — never pre-reading the whole file.
- **Full Bonus — 1 Static Variable & Unlimited Multi-FD**:
  - Uses **strictly one `static` variable** (`static t_fd *head;` in bonus, `static t_stash stash;` in mandatory).
  - Supports **interleaved reading across an unlimited number of arbitrary file descriptors** (including high `dup2()` descriptors such as `fd = 555` or `1,000` simultaneous FDs) without cross-stream contamination or fixed `FD_MAX` array limits.
- **Binary & Fault Resilience**: Tracks explicit byte lengths (`size_t len`), preserving embedded `\0` bytes inside binary lines, and deterministically frees all persistent state on `read() == -1` or **any** `malloc()` failure (passing **Francinette `fsoares` `STRICT_MEM`**).

#### Project Architecture

In strict compliance with **Chapter IV & Chapter VI of Subject v14.3**, all 6 turn-in source and header files reside directly at the root of `get_next_line/`:

```text
get_next_line/
├── get_next_line.h                 # Mandatory header (prototype, default BUFFER_SIZE=42, t_stash)
├── get_next_line.c                 # Mandatory implementation (get_next_line, gnl_fill, gnl_extract)
├── get_next_line_utils.c           # Mandatory helpers (gnl_clear, gnl_memcpy, gnl_find_nl, gnl_reserve)
├── get_next_line_bonus.h           # Bonus header (t_fd linked-list node & prototypes)
├── get_next_line_bonus.c           # Bonus implementation (1 static variable, unlimited multi-FD)
├── get_next_line_utils_bonus.c     # Bonus helpers (gnl_memcpy, gnl_find_nl, gnl_reserve, gnl_del_node)
├── README.md                       # Bilingual project & algorithm documentation (v14.3 compliant)
├── run_tests.sh                    # Master verification runner (Norm + ASan + 3 community testers)
├── docs/
│   ├── en.subject.pdf              # Official 42 Get Next Line Subject PDF (Version 14.3)
│   ├── en.subject.md               # Full English Markdown subject (Version 14.3)
│   └── fr.subject.md               # Full French Markdown subject (Version 14.3)
├── gnlTester/                      # Community suite #1 — Tripouille/gnlTester (m + b)
└── tests/
    ├── Makefile                    # Custom multi-BUFFER_SIZE & ASan/UBSan test builder
    ├── test_gnl.c                  # Edge-case suite (pipes, binary NULs, dup2 high-FD, 200KB O(L) perf)
    ├── fsoares/                    # Community suite #2 — Francinette fsoares (+ STRICT_MEM mock)
    └── gnl_station/                # Community suite #3 — kodpe/sloquet GNL Station Tester
```

#### API Specification

```c
char *get_next_line(int fd);
```

| Parameter / Return | Specification |
| :--- | :--- |
| **`fd`** | Open readable file descriptor (`0` for `stdin`, file, pipe, socket). Negative or invalid FDs return `NULL`. |
| **Return (`char *`)** | Newly heap-allocated, NUL-terminated string of exact size `line_len + 1` (caller must `free()`). |
| **Return (`NULL`)** | End-of-file reached with no unread bytes remaining, invalid `fd`/`BUFFER_SIZE`, `read()` error (`-1`), or `malloc()` failure. |
| **Allowed Externals** | `read`, `malloc`, `free` *(verified via `nm -u`)*. |

---

### Instructions

#### Prerequisites
- POSIX operating system (Linux / macOS)
- C compiler (`cc` / `gcc` / `clang`), `make`, `g++` or `clang++` (for Tripouille C++ runner), `python3` (for static Norm/symbol audit)

#### Compiling Mandatory & Bonus

`get_next_line` does not use a root `Makefile` (per 42 Moulinette turn-in rules); compile either the mandatory pair or the bonus pair directly alongside your caller file:

```bash
# Mandatory — explicit BUFFER_SIZE
cc -Wall -Wextra -Werror -D BUFFER_SIZE=42 get_next_line.c get_next_line_utils.c main.c -o gnl_demo

# Mandatory — default BUFFER_SIZE (42) when -D flag is omitted
cc -Wall -Wextra -Werror get_next_line.c get_next_line_utils.c main.c -o gnl_demo

# Bonus — multiple simultaneous file descriptors (1 static variable)
cc -Wall -Wextra -Werror -D BUFFER_SIZE=42 get_next_line_bonus.c get_next_line_utils_bonus.c main.c -o gnl_bonus_demo
```

#### Minimal Working Example

```c
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include "get_next_line_bonus.h"

int main(void)
{
    int   fd1 = open("file_a.txt", O_RDONLY);
    int   fd2 = open("file_b.txt", O_RDONLY);
    char  *l1 = get_next_line(fd1);
    char  *l2 = get_next_line(fd2);

    while (l1 || l2)
    {
        if (l1) { printf("[fd1] %s", l1); free(l1); l1 = get_next_line(fd1); }
        if (l2) { printf("[fd2] %s", l2); free(l2); l2 = get_next_line(fd2); }
    }
    close(fd1);
    close(fd2);
    return (0);
}
```

#### Verification & Test Suites

Run the complete verification pipeline (42 Norm v4.1 + symbol audit + custom ASan/UBSan suite + **Tripouille** + **Francinette `fsoares`** + **GNL Station**):

```bash
cd get_next_line

# Standard full run (fast)
./run_tests.sh

# Strict full run (adds Francinette fsoares STRICT_MEM: exhaustive malloc-fail injection + 1,000 FDs)
./run_tests.sh --strict
```

You can also invoke each community tester individually:

```bash
make -C gnlTester a                                    # Tripouille (Mandatory + Bonus: BS=1, 42, 10M)
make -C tests/fsoares mandatory bonus EXEC_STRICT=1    # Francinette strict malloc-mock + 1,000 FDs
make -C tests/gnl_station all                          # GNL Station (BS=0, 1, 2, 42, 42000)
```

---

### Algorithm

#### Design Goals & Invariants

Because `read(fd, buf, BUFFER_SIZE)` operates on fixed-size byte chunks rather than line boundaries (and `lseek(2)` is forbidden), bytes read past the first `\n` must be preserved across calls. At the entry and exit of every `get_next_line(fd)` call, our implementation maintains a strict **Stash Invariant**:

> *The persistent stash for `fd` holds **exclusively** the unread suffix of bytes already consumed from the kernel descriptor `fd` that have not yet been returned to the caller. Whenever 0 unread bytes remain (or any I/O or allocation error occurs), all heap memory associated with `fd` is immediately freed and reset.*

#### Amortized $O(L)$ Direct-Read Stash (Mandatory)

Naive implementations allocate a temporary buffer on every read and concatenate strings with `ft_strjoin`, copying all previously accumulated bytes on every iteration. When reading a line of length $L$ with `BUFFER_SIZE = 1`, naive `strjoin` performs $O(L)$ allocations and **$O(L^2)$ byte copies** (e.g., $\approx 2 \times 10^{10}$ operations on a 200 KB line, causing timeouts).

Instead, `get_next_line` uses a single `static t_stash stash` structure:

```c
typedef struct s_stash
{
    char    *buf;   /* Dynamically grown heap buffer */
    size_t  len;    /* Number of valid unread bytes currently stored */
    size_t  cap;    /* Allocated capacity of buf */
    size_t  scan;   /* Cursor of bytes already inspected for '\n' */
}   t_stash;
```

Each call executes three phases:
1. **Incremental Scan & Direct Read (`gnl_fill`)**:
   - `gnl_find_nl` scans `buf` starting at `scan` (never re-scanning older bytes). If `\n` is already present from an earlier over-read, **zero `read()` syscalls** are issued.
   - Otherwise, `gnl_reserve` ensures `cap >= len + BUFFER_SIZE + 1`. Whenever reallocation is needed, `cap` **doubles geometrically** ($\max(2 \cdot \text{cap}, \text{need})$), and `read(fd, s->buf + s->len, BUFFER_SIZE)` writes **directly into the tail of `s->buf`** (eliminating any temporary chunk buffer `malloc`/`free` and avoiding stack allocation when `BUFFER_SIZE = 10,000,000`).
2. **Exact-Size Extraction (`gnl_extract`)**:
   - Once `\n` is located at index `k` (`llen = k + 1`) or `EOF` is hit (`llen = s->len`), `malloc(llen + 1)` allocates the exact byte length required for the returned line.
3. **In-Place Remainder Compaction or Immediate Release**:
   - Let `rem = s->len - llen`. If `rem == 0`, `gnl_clear` immediately `free()`s `s->buf` and resets `{NULL, 0, 0, 0}` — meaning **zero heap memory stays allocated once the last buffered byte is consumed**, even before the caller makes a final `EOF` call.
   - If `rem > 0`, `gnl_memcpy` shifts `s->buf + llen` to `s->buf[0..rem-1]` **in place** (`0` allocations, so remainder preservation can never fail `malloc` after `line` was allocated) and resets `scan = 0`.

#### Single-Static-Pointer Dynamic Multi-FD List (Bonus)

To satisfy both bonus constraints (**1 single `static` variable** + **simultaneous multi-FD tracking**) without imposing a hardcoded `FD_MAX` array ceiling or wasting `.bss` memory, `get_next_line_bonus.c` declares a single head pointer:

```c
static t_fd *head;
```

where each `t_fd` node encapsulates `{fd, buf, len, cap, scan, next}`:
- `gnl_get_node(&head, fd)` locates the active descriptor node or prepends a fresh initialized node.
- `gnl_fill` and `gnl_extract` operate strictly on `node`, isolating each descriptor's read state.
- Whenever `rem == 0`, `EOF` is reached, `read() < 0`, or **any `malloc()` fails**, `gnl_del_node(&head, fd)` unlinks `node` from `head` and frees both `node->buf` and `node`, leaving other open file descriptors completely intact.

#### Complexity & Memory-Safety Justification

| Metric | Naive `strjoin` GNL | This Implementation |
| :--- | :--- | :--- |
| **Time to read a line of length $L$ (`BS=1`)** | $\Theta(L^2)$ *(quadratic re-copy & re-scan)* | **$\Theta(L)$ amortized** *(geometric doubling + `scan` cursor)* |
| **Heap allocations for a 20,000-byte line (`BS=1`)** | ~40,000 `malloc`s | **11 `malloc`s** *(10 doublings + 1 exact line)* |
| **Stack usage when `BUFFER_SIZE = 10,000,000`** | Crashes (`SIGSEGV`) if stack-buffered | **$O(1)$ stack** (< 64 bytes; reads into heap) |
| **Multi-FD upper bound (Bonus)** | Fixed `FD_MAX` array (fails if `fd >= FD_MAX`) | **Unbounded (`0 .. INT_MAX`)** via dynamic `t_fd` list |
| **Leak behavior on `read() == -1` or `malloc == NULL`** | Often leaks leftover stash | **0 bytes leaked** (validated by `fsoares STRICT_MEM`) |

---

### Resources

#### Classic References
- **Official 42 Subject**: [`docs/en.subject.pdf`](docs/en.subject.pdf) / [`docs/en.subject.md`](docs/en.subject.md) (*Get Next Line — Version 14.3*).
- **POSIX & Linux Manual Pages**: `man 2 read`, `man 2 open`, `man 2 pipe`, `man 2 dup2`, `man 3 malloc`, `man 3 free`.
- **C Standard & Data Structures**: B. Kernighan & D. Ritchie, *The C Programming Language* (2nd ed.) — Section 4.6 (*Static Variables*) & dynamic array amortized analysis (Cormen et al., *Introduction to Algorithms*, Chapter 17).
- **Community Test Suites Integrated**:
  - [Tripouille / `gnlTester`](https://github.com/Tripouille/gnlTester)
  - [xicodomingues / `francinette` (`fsoares` strict GNL suite)](https://github.com/xicodomingues/francinette)
  - [kodpe / `gnl-station-tester`](https://github.com/kodpe/gnl-station-tester)

#### AI Usage Disclosure (Subject v14.3 — Chapter III & V)
In accordance with 42's transparency guidelines on AI tools:
- **Tasks assisted by AI**: Structuring and formatting bilingual (`EN`/`FR`) technical documentation in `README.md` and `docs/`, cross-checking edge-case matrices across community test harnesses (`Tripouille`, `fsoares`, `gnl-station-tester`), and scripting automated static Norm/symbol checks in `run_tests.sh`.
- **Core engineering & verification**: Every C source and header file (`get_next_line*.[ch]`) uses pure manual memory management (`read`, `malloc`, `free`), complies strictly with 42 Norm v4.1, and is validated end-to-end under AddressSanitizer, UndefinedBehaviorSanitizer, and deterministic `malloc` fault injection.

---
---

## 🇫🇷 Version Française

### Sommaire

- [Description (FR)](#description-fr)
- [Instructions & Compilation](#instructions--compilation)
- [Algorithme & Justification technique](#algorithme--justification-technique)
- [Suites de tests & Vérification](#suites-de-tests--vérification)
- [Ressources & Déclaration IA](#ressources--déclaration-ia)

---

### Description (FR)

**`get_next_line`** est un projet fondamental du **Tronc Commun de l'École 42** (*Sujet officiel Version 14.3*, disponible dans [`docs/fr.subject.md`](docs/fr.subject.md)). Il consiste à écrire la fonction `char *get_next_line(int fd)` qui lit et renvoie **une ligne à la fois** depuis n'importe quel descripteur de fichier valide (fichier régulier, entrée standard `stdin`, tube `pipe`, socket), en s'appuyant uniquement sur `read`, `malloc` et `free`.

#### Points clés (Obligatoire + Bonus complet)
- **Comportement exact** : renvoie la ligne lue terminée par `\n` (sauf en fin de fichier sans `\n` final) et `NULL` à `EOF` ou en cas d'erreur.
- **Indépendant de `BUFFER_SIZE`** : fonctionne de `BUFFER_SIZE=1` jusqu'à `BUFFER_SIZE=10000000` (10 Mo sur le tas, aucun risque de *stack overflow*), avec ou sans `-D BUFFER_SIZE` à la compilation (défaut : `42`), et gère proprement `BUFFER_SIZE <= 0`.
- **Bonus intégral** :
  1. **Une seule variable statique** (`static t_fd *head;` en bonus, `static t_stash stash;` en obligatoire).
  2. **Multi-FD illimité** : lecture entrelacée simultanée sur un nombre arbitraire de descripteurs (`fd 3`, `4`, `5`, `dup2` sur `fd 555`, jusqu'à `1 000+` FDs simultanés) sans tableau de taille fixe ni mélange des flux.
- **Zéro fuite mémoire garantie** : libération immédiate de l'état interne dès que le reliquat est vide (`rem == 0`), sur erreur `read() == -1`, ou lors de n'importe quel échec d'allocation `malloc()` (**100 % validé sous Francinette `fsoares STRICT_MEM`**).

---

### Instructions & Compilation

Les 6 fichiers de rendu sont placés directement à la racine de `get_next_line/` conformément aux exigences de la Moulinette :

```bash
# Partie obligatoire (avec ou sans -D BUFFER_SIZE)
cc -Wall -Wextra -Werror -D BUFFER_SIZE=42 get_next_line.c get_next_line_utils.c main.c -o gnl

# Partie bonus (multi-FD + 1 seule variable statique)
cc -Wall -Wextra -Werror -D BUFFER_SIZE=42 get_next_line_bonus.c get_next_line_utils_bonus.c main.c -o gnl_bonus
```

---

### Algorithme & Justification technique

Plutôt qu'une concaténation naïve par `ft_strjoin` à chaque appel de `read()` — qui entraîne une complexité quadratique **$\Theta(L^2)$** et des dizaines de milliers d'allocations lorsque `BUFFER_SIZE = 1` sur de longues lignes — cette implémentation repose sur :

1. **Tampon dynamique à croissance géométrique ($\times 2$) et lecture directe (`gnl_reserve` + `gnl_fill`)** :
   - Les octets lus par `read(fd, buf + len, BUFFER_SIZE)` sont écrits **directement à la fin du tampon persistant** sur le tas, sans tampon intermédiaire.
   - Un curseur `scan` mémorise l'indice déjà inspecté afin que `gnl_find_nl` ne reparcoure jamais deux fois les mêmes octets : la lecture d'une ligne de longueur $L$ s'exécute en temps **strictement linéaire amorti $\Theta(L)$** (une ligne de 200 Ko avec `BUFFER_SIZE=1` est lue en quelques millisecondes).
   - Le suivi explicite de la longueur `len` permet en prime de supporter proprement les flux binaires contenant des octets nuls (`\0`).
2. **Extraction au juste octet & compactage sur place (`gnl_extract`)** :
   - La ligne retournée est allouée à la taille exacte `llen + 1` (`MOK` sous Tripouille et `check_mem_size` sous Francinette).
   - Le reliquat `rem = len - llen` est décalé sur place en début de tampon sans aucun `malloc` supplémentaire ; s'il vaut `0`, le tampon (et le maillon `t_fd` en bonus) est immédiatement libéré.
3. **Liste chaînée dynamique `static t_fd *head` (Bonus)** :
   - Une unique variable statique de 8 octets référence uniquement les descripteurs ayant effectivement des octets en attente, offrant le support de n'importe quel numéro de `fd` positif sans plafond `FD_MAX`.

---

### Suites de tests & Vérification

```bash
cd get_next_line

# Exécution complète : Norme + symboles nm + tests unitaires/ASan/UBSan + Tripouille + fsoares + GNL Station
./run_tests.sh

# Mode ultra-strict (ajoute l'injection exhaustive d'échecs malloc + test 1 000 FDs de Francinette)
./run_tests.sh --strict
```

---

### Ressources & Déclaration IA

- **Sujets officiels v14.3** : [`docs/fr.subject.md`](docs/fr.subject.md) (FR), [`docs/en.subject.md`](docs/en.subject.md) (EN) et [`docs/en.subject.pdf`](docs/en.subject.pdf) (PDF original).
- **Pages de manuel POSIX** : `man 2 read`, `man 3 malloc`, `man 3 free`.
- **Usage de l'IA** : conformément au Chapitre V du sujet v14.3, l'IA a été utilisée pour la structuration bilingue de la documentation (`README.md`, `docs/`) et l'intégration automatisée des bancs de test communautaires (`Tripouille`, `Francinette/fsoares`, `gnl-station-tester`) dans `run_tests.sh`.
