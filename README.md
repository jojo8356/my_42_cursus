<div align="center">

# 42 Cursus — Systems Programming in C

**Complete 42 School C Portfolio: Piscine 42 · Libft (v19.3) · get_next_line (v14.3) · ft_printf · Minitalk**

[![Language: C](https://img.shields.io/badge/Language-C_(ISO_C89%2FC99)-00599C?style=flat-square&logo=c&logoColor=white)](https://en.cppreference.com/w/c)
[![42 Norm](https://img.shields.io/badge/42_Norm-v4.1_Compliant-00BABC?style=flat-square)](https://github.com/42School/norminette)
[![Compiler Flags](https://img.shields.io/badge/Flags--Wall_-Wextra_-Werror-success?style=flat-square)](#quality-assurance--engineering-standards)
[![Memory Safety](https://img.shields.io/badge/Memory-0_Leaks_(ASan_%C2%B7_UBSan_%C2%B7_Valgrind)-brightgreen?style=flat-square)](#verification--test-suites)
[![Automated Assertions](https://img.shields.io/badge/Tests-38%2C400%2B_Passing-blue?style=flat-square)](#verification--test-suites)

**[ 🇬🇧 English ](#-english-version) · [ 🇫🇷 Version Française ](#-version-française) · [ 📐 C Architecture Guide ](docs/C_PROJECT_ARCHITECTURE_GUIDE.md)**

</div>

---

## 🇬🇧 English Version

### Overview

**`my_42_cursus`** is a curated systems-programming monorepo containing my projects from **École 42** — from the intensive 4-week **C Piscine** through the **Common Core** curriculum. Every project is written from scratch in pure C under strict constraints: **no standard high-level shortcuts**, **strict `-Wall -Wextra -Werror` compilation**, **42 Norm v4.1 compliance**, **deterministic error handling**, and **zero memory leaks**.

Beyond passing 42 evaluations, each repository module is structured according to professional C engineering conventions (see [`docs/C_PROJECT_ARCHITECTURE_GUIDE.md`](docs/C_PROJECT_ARCHITECTURE_GUIDE.md)) and backed by automated differential, unit, fault-injection, and sanitizer test suites totaling **over 38,400 automated test cases**.

### Table of Contents

- [Projects Matrix](#projects-matrix)
- [Repository Architecture](#repository-architecture)
- [Quick Start](#quick-start)
- [Verification & Test Suites](#verification--test-suites)
- [Quality Assurance & Engineering Standards](#quality-assurance--engineering-standards)
- [Author](#author)

---

### Projects Matrix

| Stage | Project | Deliverable | Key Systems Concepts | Bonus | Automated Verification |
| :--- | :--- | :--- | :--- | :---: | :--- |
| **Selection** | [**`piscine42/`**](piscine42/README.md) | 16 Modules + 3 Rushes + `bsq` | Shell, pointers, recursion, backtracking, dynamic programming, linked lists, binary trees | ✅ | 20 automated test drivers (`run_all_tests.sh`) |
| **Rank 00** | [**`libft/`**](libft/README.md) | `libft.a` *(43 functions)* | Memory management, `libc` internals, string allocators, `t_list` singly linked lists *(Subject v19.3)* | Integrated *(v19.3)* | **11,407 assertions** + `malloc` fault injection + ASan/UBSan |
| **Rank 01** | [**`get_next_line/`**](get_next_line/README.md) | `get_next_line*.[ch]` | Static storage duration, buffered POSIX `read(2)`, amortized $O(L)$ dynamic stash, multi-FD linked list *(Subject v14.3)* | ✅ 1 Static + Multi-FD | **Tripouille + Francinette `STRICT_MEM`** (1,000 FDs, `malloc` mock) + GNL Station + ASan/UBSan |
| **Rank 01** | [**`ft_printf/`**](ft_printf/README.md) | `libftprintf.a` | Variadic functions (`stdarg.h`), zero-heap stack rendering, partial `write(2)` resilience, format state machine | ✅ All (`-0.# +*`) | **26,921 differential cases** vs `glibc` + ASan/UBSan + Tripouille |
| **Rank 02** | [**`minitalk/`**](minitalk/README.md) | `client` & `server` | Asynchronous UNIX IPC (`SIGUSR1`/`SIGUSR2`), `sigaction` (`SA_SIGINFO`), bit-packing FSM, `volatile sig_atomic_t` | ✅ ACK + UTF-8 | **49 end-to-end tests** (~39 ms / 100 chars, 20k stress, PID guards) |

---

### Repository Architecture

Each project is self-contained, free of tracked build artifacts, and organized according to **The Pitchfork Layout** adapted for 42's automated grading (*Moulinette*):

```text
my_42_cursus/
├── README.md                           # Monorepo overview (EN / FR)
├── .gitignore                          # Strict C artifact & binary ignore rules
├── docs/
│   └── C_PROJECT_ARCHITECTURE_GUIDE.md # Professional C project layout & Makefile guide
├── libft/                              # Rank 00 — Standalone C standard library (v19.3)
│   ├── Makefile                        # Builds libft.a (root layout mandated by v19.3)
│   ├── libft.h                         # Public API (43 prototypes + t_list)
│   ├── ft_*.c                          # 43 Norm-compliant translation units
│   ├── docs/                           # Official 2026 v19.3 specification
│   └── tests/                          # 4-phase test harness (unit, ASan, malloc wrap)
├── get_next_line/                      # Rank 01 — Amortized O(L) stream line reader (v14.3)
│   ├── get_next_line*.[ch]             # Mandatory & _bonus.[ch] root deliverables (1 static var)
│   ├── docs/                           # Official v14.3 PDF + EN & FR Markdown subjects
│   ├── gnlTester/                      # Tripouille reference tester (mandatory + bonus)
│   ├── tests/                          # Custom ASan suite + Francinette fsoares + GNL Station
│   └── run_tests.sh                    # Full Norm, symbol, sanitizer & community test runner
├── ft_printf/                          # Rank 01 — Variadic formatted output library
│   ├── include/                        # ft_printf.h & ft_printf_bonus.h
│   ├── src/                            # Mandatory & _bonus.c implementation files
│   ├── docs/                           # Official subject PDF
│   ├── tests/                          # 26,921-case differential generator & runner
│   ├── printfTester/                   # Tripouille reference tester
│   └── Makefile                        # Out-of-source obj/ build -> libftprintf.a
├── minitalk/                           # Rank 02 — UNIX signal bit-stream IPC
│   ├── include/                        # minitalk.h & minitalk_bonus.h
│   ├── src/                            # Client, server, bit transfer & strict PID utils
│   ├── docs/                           # Official EN & FR subjects (MD + PDF)
│   ├── tests/                          # 49-check Python test suite + community testers
│   └── Makefile                        # Out-of-source obj/ build -> client & server
└── piscine42/                          # 42 C Piscine (Shell00-01, C00-C13, Rush00-02, BSQ)
    ├── shell00/ .. shell01/            # UNIX shell exercises
    ├── c00/ .. c13/                    # Pure Moulinette-ready exXX/ C deliverables
    ├── rush00/ .. rush02/              # Team weekend sprints (Skyscrapers, Dict parser)
    ├── bsq/                            # Capstone dynamic-programming square solver
    ├── subjects/pdf/                   # 34 archived subject PDFs
    └── tests/                          # Non-intrusive C & POSIX sh test drivers
```

---

### Quick Start

#### Prerequisites
- **OS**: Linux or macOS (POSIX-compliant)
- **Toolchain**: `cc` (`gcc` or `clang`), `make`, `ar`, `python3` (for differential/IPC test runners)

#### Clone & Build Any Project

```bash
git clone https://github.com/jojo8356/my_42_cursus.git
cd my_42_cursus

# 1. Build Libft (static library -> libft/libft.a)
make -C libft

# 2. Build ft_printf with all bonus flags (-> ft_printf/libftprintf.a)
make -C ft_printf bonus

# 3. Build Minitalk client & server (+ bonus binaries)
make -C minitalk
make -C minitalk bonus
```

---

### Verification & Test Suites

Run each project's full automated verification pipeline directly from its directory:

```bash
# Libft: symbol audit (nm) + 11,407 unit assertions + ASan/UBSan + malloc fault injection
make -C libft/tests

# get_next_line: Norm/symbol audit + ASan/UBSan + Tripouille + Francinette STRICT_MEM + GNL Station
./get_next_line/run_tests.sh --strict

# ft_printf: 26,921 differential cases vs libc snprintf + ASan/UBSan
./ft_printf/run_tests.sh

# Minitalk: 49 protocol, speed (<40ms/100 chars), UTF-8, 20k-char & error tests
python3 minitalk/tests/minitalk_full_test.py

# Piscine 42: run all module & capstone drivers (or pass specific targets e.g. bsq rush02)
./piscine42/run_all_tests.sh
```

---

### Quality Assurance & Engineering Standards

- **Strict Compilation**: Every translation unit compiles cleanly under `cc -Wall -Wextra -Werror` with zero warnings and zero relinking on consecutive `make` invocations.
- **42 Norm v4.1**: Maximum 25 lines per function, maximum 5 functions per `.c` file, maximum 4 parameters per function, explicit control flow, and `static` encapsulation of all internal helpers (verified via `nm` symbol table inspection).
- **Memory & Undefined-Behavior Safety**: Validated with **AddressSanitizer (`ASan`)**, **LeakSanitizer (`LSan`)**, **UndefinedBehaviorSanitizer (`UBSan`)**, **Valgrind**, and linker-wrapped `--wrap=malloc` deterministic allocation failure injection.
- **Clean Git Tree**: Zero `.o`, `.a`, or ELF binaries tracked in version control.

---
---

## 🇫🇷 Version Française

### Présentation

**`my_42_cursus`** est le monorepo regroupant l'ensemble de mes projets de programmation système en **C** réalisés à l'**École 42** — depuis la **Piscine C** jusqu'aux projets du **Tronc Commun (*Common Core*)**. Chaque projet est développé intégralement à partir de zéro en C pur selon des contraintes d'ingénierie strictes : compilation sous **`-Wall -Wextra -Werror`**, respect intégral de la **Norme 42 v4.1**, gestion défensive des erreurs et **zéro fuite mémoire**.

Chaque projet est structuré proprement selon les standards de l'industrie adaptés aux règles de la Moulinette 42 (voir le guide complet [`docs/C_PROJECT_ARCHITECTURE_GUIDE.md`](docs/C_PROJECT_ARCHITECTURE_GUIDE.md)) et validé par plus de **38 400 tests automatisés** (tests unitaires, différentiels face à la `glibc`, injection de pannes `malloc`, AddressSanitizer, UBSan et Valgrind).

### Sommaire

- [Catalogue des projets](#catalogue-des-projets)
- [Architecture du dépôt](#architecture-du-dépôt)
- [Démarrage rapide](#démarrage-rapide)
- [Lancer les suites de tests](#lancer-les-suites-de-tests)
- [Standards de qualité & ingénierie](#standards-de-qualité--ingénierie)
- [Auteur](#auteur)

---

### Catalogue des projets

| Étape | Projet | Livrable | Compétences clés | Bonus | Validation automatisée |
| :--- | :--- | :--- | :--- | :---: | :--- |
| **Sélection** | [**`piscine42/`**](piscine42/README.md) | 16 modules + 3 Rushs + `bsq` | Shell UNIX, pointeurs, récursivité, backtracking, programmation dynamique, listes & arbres binaires | ✅ | 20 drivers de tests (`run_all_tests.sh`) |
| **Rang 00** | [**`libft/`**](libft/README.md) | `libft.a` *(43 fonctions)* | Gestion mémoire bas niveau, réimplémentation `libc`, allocateurs de chaînes, listes chaînées `t_list` *(Sujet 2026 v19.3)* | Intégrés *(v19.3)* | **11 407 assertions** + injection d'échecs `malloc` + ASan/UBSan |
| **Rang 01** | [**`get_next_line/`**](get_next_line/README.md) | `get_next_line*.[ch]` | Variables `static`, `read(2)` bufferisé, tampon dynamique $\Theta(L)$, liste chaînée multi-FD *(Sujet v14.3)* | ✅ 1 `static` + Multi-FD | **Tripouille + Francinette `STRICT_MEM`** (1 000 FDs, pannes `malloc`) + GNL Station + ASan/UBSan |
| **Rang 01** | [**`ft_printf/`**](ft_printf/README.md) | `libftprintf.a` | Fonctions variadiques (`stdarg.h`), rendu 100 % sur la pile (`0 malloc`), écriture robuste `write(2)`, automate de parsing | ✅ Complet (`-0.# +*`) | **26 921 tests différentiels** vs `glibc` + ASan/UBSan + Tripouille |
| **Rang 02** | [**`minitalk/`**](minitalk/README.md) | `client` & `server` | IPC UNIX par signaux (`SIGUSR1`/`SIGUSR2`), `sigaction` (`SA_SIGINFO`), encodage bit-à-bit avec ACK, `volatile sig_atomic_t` | ✅ ACK + UTF-8 | **49 tests bout-en-bout** (~39 ms / 100 chars, 20 000 chars, erreurs PID) |

---

### Architecture du dépôt

Le dépôt est organisé sans aucun fichier binaire parasite (`.o`, `.a`, exécutables ignorés via `.gitignore`) et sans double imbrication de dossiers :
- **`ft_printf/`** et **`minitalk/`** adoptent le **Pitchfork Layout** adapté 42 : headers dans `include/`, sources dans `src/`, objets compilés hors-source dans `obj/` (non suivi par Git), documentation dans `docs/` et harnais de validation dans `tests/`.
- **`get_next_line/`** conserve les **6 fichiers de rendu imposés à la racine** (`get_next_line.{c,h}`, `get_next_line_utils.c` et leurs variantes `_bonus.{c,h}`) — le sujet v14.3 n'attend **aucun `Makefile`** — tout en déportant le sujet dans `docs/`, les testeurs dans `tests/` et `gnlTester/`, et l'orchestrateur dans `run_tests.sh`.
- **`libft/`** conserve volontairement `Makefile`, `libft.h`, `README.md` et les 43 `ft_*.c` à sa racine conformément à l'exigence stricte du **Chapitre IV.1 du sujet Libft 2026 v19.3**, tout en isolant le sujet dans `docs/` et les tests dans `tests/`.
- **`piscine42/`** préserve des dossiers `exXX/` strictement purs (prêts pour la Moulinette) et déporte l'intégralité des bancs d'essai dans `piscine42/tests/`.

Pour aller plus loin sur l'organisation professionnelle d'un projet C en entreprise et à 42, consultez [**`docs/C_PROJECT_ARCHITECTURE_GUIDE.md`**](docs/C_PROJECT_ARCHITECTURE_GUIDE.md).

---

### Démarrage rapide

```bash
git clone https://github.com/jojo8356/my_42_cursus.git
cd my_42_cursus

# Compiler Libft (-> libft/libft.a)
make -C libft

# Compiler ft_printf en mode obligatoire ou bonus (-> ft_printf/libftprintf.a)
make -C ft_printf
make -C ft_printf bonus

# Compiler Minitalk (client/server + versions bonus)
make -C minitalk
make -C minitalk bonus
```

---

### Lancer les suites de tests

```bash
# Libft : audit binaire nm + 11 407 assertions + ASan/UBSan + pannes malloc déterministes
make -C libft/tests

# get_next_line : audit Norme/nm + ASan/UBSan + Tripouille + Francinette STRICT_MEM + GNL Station
./get_next_line/run_tests.sh --strict

# ft_printf : 26 921 cas différentiels face à snprintf + ASan/UBSan
./ft_printf/run_tests.sh

# Minitalk : 49 tests (Makefile, vitesse, intégrité 20k chars, UTF-8/emojis, sécurité PID)
python3 minitalk/tests/minitalk_full_test.py

# Piscine 42 : exécuter tous les drivers (ou cibler ex. ./piscine42/run_all_tests.sh bsq rush02)
./piscine42/run_all_tests.sh
```

---

### Standards de qualité & ingénierie

- **Zéro warning, zéro relink** : Makefiles conformes aux standards GNU Make et aux règles 42 (`all`, `clean`, `fclean`, `re`, `bonus`).
- **Isolation des symboles** : toutes les fonctions internes sont déclarées `static` ; aucune variable globale n'est utilisée (à l'exception de l'unique `volatile sig_atomic_t` requise et justifiée dans `minitalk`).
- **Sécurité mémoire prouvée** : vérification systématique sous AddressSanitizer, UndefinedBehaviorSanitizer, LeakSanitizer et injection d'échecs d'allocation sur chaque appel `malloc`.

---

### Author / Auteur

- **GitHub**: [@jojo8356](https://github.com/jojo8356)
- **Curriculum**: École 42 — Common Core & C Piscine
