<div align="center">

# 42 Cursus — Systems Programming in C

**Complete 42 School Portfolio: Piscine 42 (C) · Piscine Python · A-Maze-ing · Libft · get_next_line · ft_printf · Minitalk · Born2beRoot · Inception · Pipex · So_long · Push_swap · FdF · Fract'ol · Minishell · Philosophers · Codexion · CPP Modules 00–09 · FT_Containers · Cub3D · NetPratice · MiniRT · FT_Transcendence · Exam Rank 02 · Exam Rank 03 · Exam Rank 04 · Exam Rank 05 · Exam Rank 06**

[![Language: C](https://img.shields.io/badge/Language-C_(ISO_C89%2FC99)-00599C?style=flat-square&logo=c&logoColor=white)](https://en.cppreference.com/w/c)
[![42 Norm](https://img.shields.io/badge/42_Norm-v4.1_Compliant-00BABC?style=flat-square)](https://github.com/42School/norminette)
[![Compiler Flags](https://img.shields.io/badge/Flags--Wall_-Wextra_-Werror-success?style=flat-square)](#quality-assurance--engineering-standards)
[![Memory Safety](https://img.shields.io/badge/Memory-0_Leaks_(ASan_%C2%B7_UBSan_%C2%B7_Valgrind)-brightgreen?style=flat-square)](#verification--test-suites)
[![Automated Assertions](https://img.shields.io/badge/Tests-57%2C000%2B_Passing-blue?style=flat-square)](#verification--test-suites)

**[ 🇬🇧 English ](#-english-version) · [ 🇫🇷 Version Française ](#-version-française) · [ 📐 C Architecture Guide ](docs/C_PROJECT_ARCHITECTURE_GUIDE.md)**

</div>

---

## 🇬🇧 English Version

### Overview

**`my_42_cursus`** is a curated systems-programming monorepo containing my projects from **École 42** — from the intensive 4-week **C Piscine** and the **Python Piscine** through the **Common Core** curriculum. The portfolio spans pure-C systems engineering (**Libft**, **Minitalk**, **Pipex**, **Philosophers**, **Codexion**), algorithmic and Python work (**A-Maze-ing** maze generator, the **Piscine Python** modules 00–10), DevOps infrastructure (**Born2beRoot**, **Inception** Docker stack), and the C++/web tracks. Every project is written from scratch under strict constraints: **no standard high-level shortcuts**, **strict `-Wall -Wextra -Werror` compilation**, **42 Norm v4.1 compliance**, **deterministic error handling**, and **zero memory leaks**.

Beyond passing 42 evaluations, each repository module is structured according to professional C engineering conventions (see [`docs/C_PROJECT_ARCHITECTURE_GUIDE.md`](docs/C_PROJECT_ARCHITECTURE_GUIDE.md)) and backed by automated differential, unit, fault-injection, and sanitizer test suites totaling **over 57,000 automated test cases**.

### Table of Contents

- [Projects Matrix](#projects-matrix)
- [Repository Architecture](#repository-architecture)
- [Quick Start](#quick-start)
- [Verification & Test Suites](#verification--test-suites)
- [Quality Assurance & Engineering Standards](#quality-assurance--engineering-standards)
- [Author](#author)

---

### Projects Matrix

The portfolio is organized along the 42 curriculum's **4 circles** — from the
foundations (Circle 1) to advanced concurrency (Circle 3) and the C++ track
(Circle 4) — plus standalone **Exam Rank** practice repositories.

#### 🔵 Circle 1 — Foundations

| Stage | Project | Deliverable | Key Systems Concepts | Bonus | Automated Verification |
| :--- | :--- | :--- | :--- | :---: | :--- |
| **Rank 00** | [**`libft/`**](libft/README.md) | `libft.a` *(43 functions)* | Memory management, `libc` internals, string allocators, `t_list` singly linked lists *(Subject v19.3)* | Integrated *(v19.3)* | **11,407 assertions** + `malloc` fault injection + ASan/UBSan |
| **Rank 01** | [**`ft_printf/`**](ft_printf/README.md) | `libftprintf.a` | Variadic functions (`stdarg.h`), zero-heap stack rendering, partial `write(2)` resilience, format state machine | ✅ All (`-0.# +*`) | **26,921 differential cases** vs `glibc` + ASan/UBSan + Tripouille |
| **Rank 01** | [**`get_next_line/`**](get_next_line/README.md) | `get_next_line*.[ch]` | Static storage duration, buffered POSIX `read(2)`, amortized $O(L)$ dynamic stash, multi-FD linked list *(Subject v14.3)* | ✅ 1 Static + Multi-FD | **Tripouille + Francinette `STRICT_MEM`** (1,000 FDs, `malloc` mock) + GNL Station + ASan/UBSan |
| **Rank 02** | [**`born2beroot/`**](born2beroot/README.md) | Debian VM + `monitoring.sh` | LUKS2 + LVM partitioning, `pam_pwquality`/`login.defs` policy, `sudoers` hardening, UFW, AppArmor, systemd + cron | ✅ WordPress · vsftpd · fail2ban | **106 static + 167 requirement checks + 5 community testers (80 checks) + `verify.sh` (64)** |
| **DevOps** | [**`inception/`**](inception/README.md) | Docker Compose stack | Full web infrastructure via **Docker Compose**: NGINX (TLS 1.2/1.3, port 443) + WordPress/php-fpm (TCP 9000) + MariaDB, all images built from **Debian bookworm**; secrets via Docker secrets | ✅ Adminer · Redis · FTP · personal site | **58 tests** (static YAML/Dockerfile/compose/Nginx/php-fpm/MariaDB/.env analysis) |

#### 🟢 Circle 2 — Core Systems

| Stage | Project | Deliverable | Key Systems Concepts | Bonus | Automated Verification |
| :--- | :--- | :--- | :--- | :---: | :--- |
| **Rank 02** | [**`minitalk/`**](minitalk/README.md) | `client` & `server` | Asynchronous UNIX IPC (`SIGUSR1`/`SIGUSR2`), `sigaction` (`SA_SIGINFO`), bit-packing FSM, `volatile sig_atomic_t` | ✅ ACK + UTF-8 | **49 end-to-end tests** (~39 ms / 100 chars, 20k stress, PID guards) |
| **Rank 02** | [**`pipex/`**](pipex/README.md) | `pipex` binary | UNIX pipes: `fork` + `dup2` + `execve`, N-pipe chains, here_doc, PATH resolution, fd hygiene (no deadlock, no leak) | ✅ N pipes + here_doc | **131 checks** (differential vs bash, ASan/UBSan, robustness) + **3 community testers (71)** |
| **Rank 02** | [**`so_long/`**](so_long/README.md) | `so_long` & `so_long_bonus` binaries | miniLibX windowing, XPM textures, event hooks, map parsing & validation, tile rendering, game loop | ✅ Enemies · animation · on-screen counter | **106 checks** (10 map rules, gameplay, bonuses, ASan/UBSan over 41 maps, Norm) + **community tester (44)** |
| **Rank 02** | [**`push_swap/`**](push_swap/README.fr.md) | `push_swap` binary | Sort two integer stacks in minimal moves: sorter + pusher, SA/SB/PA/PB operations, cost-optimized decision engine | ✅ | **388 verifications** (cases, 20-benchmark, differential) |
| **Rank 02** | [**`fract-ol/`**](fract-ol/README.md) | `fract-ol` binary | miniLibX 2D fractal renderer (Mandelbrot, Julia), escape-time rendering, mouse zoom, color palettes | ✅ Burning Ship · Tricorn · Phoenix · Multibrot · multithreading | **1,077 tests** (6 suites, no X11 required) |
| **Rank 02** | [**`fdf/`**](fdf/README.md) | `fdf` binary | miniLibX wireframe 3D terrain renderer, `.fdf` height-map parsing, isometric projection, color gradient | ✅ 3D rotation · 5 color modes · rainbow HSL→RGB | **248 tests** (5 suites: parse, math, draw, color, hooks) |
| **Rank 04** | [**`cub3d/`**](cub3d/README.md) | `cub3d` binary | Raycasting (Wolfenstein 3D): DDA line casting, texture mapping, doors, sprites, minimap, miniLibX | ✅ Full bonus (doors, sprites, minimap) | **186 tests** (8 suites: parser, player, raycasting, doors, sprites, integration, robustness) |
| **Rank 04** | [**`netpratice/`**](netpratice/) | 10 solution JSONs | IPv4 addressing, subnet masks & routing engine over 10 levels: CIDR/mask math, longest-prefix routing, solver & validator | ✅ Full solver + fd-safe pipelines | **206 tests** (8 suites: ip, mask, routing, json, solution, validate, solver, levels) |

#### 🟡 Circle 3 — Advanced Concurrency

| Stage | Project | Deliverable | Key Systems Concepts | Bonus | Automated Verification |
| :--- | :--- | :--- | :--- | :---: | :--- |
| **Rank 03** | [**`minishell/`**](minishell/README.md) | `minishell` binary | Mini shell: lexer/parser, env & variable expansion, pipes, redirections, logical operators (`&&`/`||`), wildcards | ✅ Bonus features | **60 tests** (7 C suites: basic, variables, quotes, pipes, redirects, logical, wildcards) |
| **Rank 03** | [**`philosophers/`**](philosophers/README.md) | `philo` & `philo_bonus` binaries | Dining philosophers: threads + mutex (mandatory), processes + named semaphores (bonus), race-free timing & messaging | ✅ Named semaphores | **87/87 tests** (Python suite, mandatory + bonus) |
| **Rank 03** | [**`codexion/`**](codexion/README.md) | `codexion` binary | Dining-philosophers-style concurrency: `n_coders` threads each needing **both** neighboring USB dongles to compile, monitor thread tracking burnout deadlines, POSIX mutex + condition variables, **no global variables** (single `t_sim` struct), FIFO & EDF scheduling | ✅ EDF scheduler · dongle cooldown | **157/158 student tests** (log format, monotonicity, burnout deadline, cooldown, stability) + **Norm v4.1** |

#### 🔴 Circle 4 — C++

| Stage | Project | Deliverable | Key Systems Concepts | Bonus | Automated Verification |
| :--- | :--- | :--- | :--- | :---: | :--- |
| **Rank 03** | [**`cpp_modules/`**](cpp_modules/README.md) | 37 exercises (`cpp_00`–`cpp_09`) | OOP: classes, inheritance, exceptions, templates, orthodox canonical form; STL (`map`/`vector`/`deque`/`stack`) | ✅ BSP · Forms · DiamondTrap | **93/93 tests** (build `-Wall -Wextra -Werror -std=c++98`, functional, edge cases, 3000-element stress, ASan/TSan) |
| **C++98** | [**`ft_containers/`**](ft_containers/subjects/ft_containers_subject.md) | 10 headers `<container>.hpp` | STL containers recoded from scratch in C++98, namespace `ft`, `std::allocator`: `vector`, `map`, `stack` (+ bonus `set`, `deque`, `queue`, `multimap`, `multiset`), red-black trees, `iterator_traits`, `reverse_iterator`, `enable_if`, `is_integral`, `lexicographical_compare`/`equal`, `pair`/`make_pair` | ✅ All bonus containers | **3,408 ASan/UBSan assertions** + **5 testers** (llefranc, monkey, mazoise, mli, static) + **official libstdc++ GCC 4.9.4 testsuite** (0 applicable bugs) |

#### 🐍 Python Track — Piscine Python & Algorithmic

| Stage | Project | Deliverable | Key Systems Concepts | Bonus | Automated Verification |
| :--- | :--- | :--- | :--- | :---: | :--- |
| **Piscine** | [**`piscine_python/`**](piscine_python/README.md) | Modules 00–10 | Complete **Python Piscine**: fundamentals, OOP, exceptions, collections, I/O, ABC, import/packages, design patterns, env & deps, Pydantic v2 validation, functional programming | ✅ All bonus modules | **44/44** reference checker (modules 00–08) + **65/65** home pytest suite (modules 09–10 + structure) |
| **Algorithmic** | [**`a_maze_ing/`**](a_maze_ing/README.md) | `a_maze_ing` + `mazegen` package | Maze **generator, solver & visualizer**: `KEY=VALUE` config, DFS/Prim generation, BFS shortest path, hex wall-file output, terminal UI, hidden **"42"**, perfect-maze vs Pac-Man board | ✅ Prim algorithm · SEED · reusable `mazegen` (pip-installable) | **20 pytest tests** + independent maze analyzer (wall coherence, connectivity, 3×3 areas, loops, path optimality) |

#### 🟣 Final Project — Web

| Stage | Project | Deliverable | Key Systems Concepts | Bonus | Automated Verification |
| :--- | :--- | :--- | :--- | :---: | :--- |
| **Final** | [**`ft_transcendence/`**](ft_transcendence/README.md) | Full-stack web app | Pong multiplayer game over websockets (Django + Channels + vanilla JS SPA), IA adversary, tournaments + matchmaking, live chat, OAuth 2.0 (42), JWT + 2FA, user management, customization, multilingue, RGPD | ✅ All modules | **147/147 tests** (engine, AI, bracket, API, security, student patterns) |

#### 📚 Exam Rank Practice

| Stage | Project | Deliverable | Key Systems Concepts | Bonus | Automated Verification |
| :--- | :--- | :--- | :--- | :---: | :--- |
| **Rank 02** | [**`exam_rank02/`**](exam_rank02/README.md) | `exam_rank02` binary | Exam Rank 02 practice — 4 levels: strings, advanced manipulation, algorithms, memory & linked lists | ✅ | **111 tests** (4 C suites, level 1–4) |
| **Rank 03** | [**`exam_rank_03/`**](exam_rank_03/README.md) | 8 programs | Exam Rank 03 practice — level 1 (filter, ft_scanf, get_next_line) & level 2 (n_queens, permutations, powerset, rip, tsp) | ✅ | **90/90 tests** |
| **Rank 04** | [**`exam_rank_04/`**](exam_rank_04/README.md) | `microshell` binary | Exam Rank 04 — microshell: shell command execution, `|`/`;` pipelines, `cd` builtin, fd-safe concurrency | ✅ `&&`/`||` short-circuit | **33 tests** (11 unit parseur + 22 integration) |
| **Rank 05** | [**`miniRT/`**](miniRT/README.md) | `miniRT` binary | Exam Rank 05 — CPU ray tracer: `.rt` scene parsing, intersections, lighting, shadows, reflections, cameras | ✅ Full features | **14,660 tests** (9 suites: matrix, parser, objects, intersect, lighting, camera, integration) |
| **Rank 05** | [**`exam_rank_05/`**](exam_rank_05/README.md) | `vect2` · `bigint` · `polyset` · `life` · `bsq` | Exam Rank 05 (new 2025) — level 0 C++98 (2D vector, arbitrary-precision int, polymorphic set) & level 1 C (Conway's Game of Life, biggest square) | ✅ All bonuses | **Suite complète** (unit C++ + intégration Python, testers étudiants) |
| **Rank 06** | [**`exam_rank_06/`**](exam_rank_06/README.md) | `mini_serv` · `mini_db` | Exam Rank 06 (new 2025) — `mini_serv`: chat TCP multi-clients non-bloquant (`select`) ; `mini_db`: base clé-valeur sur TCP avec persistance | ✅ Full scope | **97/97 tests** (fonctionnels, statiques, testers étudiants) |

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
├── pipex/                              # Rank 02 — UNIX pipes with fork/dup2/execve
│   ├── include/                        # pipex.h & pipex_bonus.h
│   ├── src/                            # Mandatory engine + src/bonus (N pipes, here_doc)
│   ├── libft/                          # The 42 library, built by its own Makefile
│   ├── docs/                           # Official v2 PDF + EN & FR transcriptions
│   └── tests/                          # audit.py, 5 suites, 3 community testers
├── born2beroot/                        # Rank 02 — Systems administration on Debian
│   ├── docs/                           # Official v5.2 PDF + EN & FR Markdown subjects
│   ├── config/                         # vm.conf, partitions.layout, sshd, sudoers, ufw, pwquality
│   ├── scripts/                        # setup.sh orchestrator, 10 steps, monitoring.sh, verify.sh
│   └── tests/                          # VM emulator, per-requirement cases, 5 community testers
├── inception/                          # DevOps — Docker Compose web infrastructure (NGINX + WordPress + MariaDB)
│   ├── srcs/                           # .env, docker-compose.yml, requirements/{nginx,wordpress,mariadb,bonus}
│   ├── tests/                          # 58-check static suite (YAML, Dockerfiles, compose, Nginx, php-fpm, MariaDB)
│   └── Makefile                        # up / down / test orchestration
├── so_long/                            # Rank 02 — 2D game with the miniLibX
│   ├── include/                        # so_long.h
│   ├── src/                            # Parsing, rendering, hooks + src/bonus
│   ├── assets/                         # 9 XPM sprites, 2 frames per entity
│   ├── docs/                           # Official PDF + EN & FR transcriptions
│   └── tests/                          # miniLibX shim, 6 suites, 1 community tester
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
├── piscine_python/                     # Python Piscine — modules 00–10 (fundamentals → OOP → functional)
│   ├── module00/ .. module10/          # Per-module exXX/ deliverables (11 modules)
│   ├── tests/                          # Reference checker (00–08) + home pytest suite (09–10, structure)
│   └── README.md                       # 44/44 + 65/65 validation table
├── a_maze_ing/                         # Algorithmic — maze generator, solver & visualizer
│   ├── mazegen/                        # Reusable, pip-installable generation package (independent of CLI/UI)
│   ├── a_maze_ing.py                   # CLI front-end: config parsing, output writing, terminal UI
│   ├── tests/                          # 20 pytest tests + independent maze analyzer
│   └── pyproject.toml                  # Packaging (mazegen wheel)
├── fdf/                               # Rank 02 — miniLibX wireframe 3D terrain renderer
│   ├── includes/                      # fdf.h (types, prototypes)
│   ├── src/                           # parse, math, color, draw, render, hooks
│   ├── tests/                         # 5 suites / 248 tests + test maps
│   └── Makefile
├── fract-ol/                          # Rank 02 — miniLibX 2D fractal renderer
│   ├── includes/                      # fract-ol.h
│   ├── src/                           # Mandelbrot/Julia + bonus fractals, multithread
│   ├── tests/                         # 6 suites / 1,077 tests (headless)
│   └── Makefile
├── push_swap/                         # Rank 02 — sort two integer stacks in minimal moves
│   ├── push_swap.c                    # sorter + pusher
│   ├── push_swap_utils.c              # operations SA/SB/PA/PB
│   ├── tests/                         # 388 verifications (cases, benchmark, differential)
│   └── Makefile
├── minishell/                         # Rank 03 — minimal shell in C
│   ├── include/                       # minishell.h (types, prototypes)
│   ├── src/                           # lexer, parser, env, builtins, execution
│   ├── bonus/                         # bonus feature files
│   ├── libft/                         # vendored libft
│   ├── tests/                         # 7 C suites / 60 tests (run_tests.sh)
│   └── Makefile
├── philosophers/                      # Rank 03 — dining philosophers
│   ├── philo/                         # mandatory: threads + mutex
│   ├── philo_bonus/                   # bonus: processes + named semaphores
│   ├── tests/                         # Python suite (87/87)
│   └── Makefile
├── codexion/                          # Rank 03 — dining-philosophers concurrency (coders + USB dongles)
│   ├── codexion.c / codexion.h        # main, arg parsing, t_sim orchestration
│   ├── coder/                         # coder threads + compilation cycle (wait/compile/debug/refactor)
│   ├── dongle/                        # per-dongle mini-heap (FIFO/EDF) + acquisition logic
│   ├── monitor/                       # monitor thread tracking burnout deadlines
│   ├── log/ utils/                    # thread-safe logging + helpers
│   └── Makefile
├── exam_rank02/                       # Exam Rank 02 practice — 4 levels
│   ├── include/exam_rank02.h          # prototypes level 1–4 (+ list_t)
│   ├── level_1/ .. level_4/           # level implementations
│   ├── libft/                         # local libft
│   └── tests/                         # 4 C suites / 111 tests
├── exam_rank_03/                      # Exam Rank 03 practice — level 1 & 2
│   ├── filter/ ft_scanf/ get_next_line/      # level 1 programs
│   ├── n_queens/ permutations/ powerset/ rip/ tsp/   # level 2 programs
│   └── tests/                         # Python suite (90/90)
├── exam_rank_04/                      # Exam Rank 04 — microshell
│   ├── microshell.h                   # t_sep / t_cmd / t_group(s) types, API
│   ├── parser.c / microshell.c        # parsing + pipeline/group execution
│   ├── subjects/                      # Official subject (EN & FR)
│   └── tests/                         # 11 unit + 22 integration tests
├── cub3d/                             # Rank 04 — Wolfenstein 3D raycasting (miniLibX)
│   ├── include/ src/                  # raycaster, textures, doors, sprites, minimap
│   └── tests/                         # 8 suites / 186 tests
├── netpratice/                        # Rank 04 — IPv4 addressing, masks & routing (10 levels)
│   ├── src/                           # IP/mask/routing engine + JSON solution writer
│   ├── levels/                        # level1.json … level10.json deliverables
│   └── tests/                         # 8 suites / 206 tests
├── miniRT/                            # Exam Rank 05 — CPU ray tracer
│   ├── include/ src/                  # .rt parser, intersections, lighting, camera
│   ├── scenes/                        # sample scenes
│   └── tests/                         # 9 suites / 14,660 tests
├── cpp_modules/                        # Rank 03 — C++ Modules 00-09 (OOP & STL)
│   ├── subjects/                      # Official subject texts (CPP_Module_00.txt … 09)
│   ├── cpp_00/ .. cpp_09/             # 37 exercises (ex00..exN per module)
│   │   ├── Makefile                    # all/clean/fclean/re targets
│   │   └── *.hpp / *.cpp / main.cpp    # Orthodox canonical form, C++98 strict
│   └── tests/                          # 93-check Python suite (functional, stress, ASan/TSan)
└── ft_containers/                      # C++98 — STL containers recoded in namespace ft
    ├── Makefile                        # Orchestrates: internal suite + 5 student testers
    ├── subjects/                       # Official subject (ft_containers_subject.md)
    ├── includes/                       # vector.hpp, map.hpp, stack.hpp, set.hpp, deque.hpp,
    │                                   # queue.hpp, multimap.hpp, multiset.hpp, list.hpp,
    │                                   # priority_queue.hpp, rb_tree.hpp, iterator.hpp, ...
    ├── tests/
    │   ├── test_containers.cpp         # Internal ASan/UBSan suite (3,408 assertions)
    │   ├── compile_check.cpp           # Header-only compile smoke test
    │   ├── official/                   # Official libstdc++ GCC 4.9.4 testsuite (shim + run.sh)
    │   └── testers/                    # llefranc, monkey, mazoise, mli, static testers
    └── (no binaries tracked)           # header-only library, builds via Makefile
├── ft_transcendence/                   # Final — Full-stack Pong web app (Django + Channels + SPA)
│   ├── config/                         # settings (JWT/OAuth/Channels), ASGI, urls + SPA catch-all
│   ├── accounts/ games/ tournaments/ chat/   # Django apps: auth, engine/AI, bracket, chat
│   ├── frontend/                       # vanilla JS SPA (Pong canvas, auth, chat, tournaments)
│   ├── subject.txt                     # full subject v3.0.1
│   └── tests/                          # 147-test suite (engine, AI, bracket, API, security)
├── exam_rank_05/                       # Exam Rank 05 (new 2025) — vect2/bigint/polyset (C++98), Life/BSQ (C)
│   ├── level-0/                        # vect2, bigint, polyset
│   ├── level-1/                        # Life, BSQ
│   ├── subject.md                      # full subjects
│   └── tests/                          # unit C++ + integration Python suite
└── exam_rank_06/                       # Exam Rank 06 (new 2025) — mini_serv (C) + mini_db (C++)
    ├── mini_serv/                      # non-blocking multi-client TCP chat server (select)
    ├── mini_db/                        # key-value database over TCP (persistent connections)
    ├── subject.md                      # full subjects
    └── tests/                          # 97-test suite + student reference testers
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

# 4. Build FdF (wireframe 3D terrain), Fract'ol (2D fractals) and push_swap
make -C fdf
make -C fract-ol
make -C push_swap

# 5. Build Minishell and Philosophers (Rank 03)
make -C minishell
make -C philosophers

# 6. Build Exam Rank 02 (4-level practice binary)
make -C exam_rank02

# 7. Build Exam Rank 04 — microshell
make -C exam_rank_04

# 8. Build Cub3d (Wolfenstein 3D raycasting, Rank 04) & MiniRT (ray tracer, Rank 05)
make -C cub3d
make -C miniRT

# 9. Build NetPratice (IPv4/routing engine, Rank 04)
make -C netpratice

# 10. Build CPP Modules (each exercise in cpp_modules/cpp_XX/exYY)
make -C cpp_modules/cpp_09/ex02

# 11. Build FT_Containers (header-only STL containers, C++98) + run internal suite + 5 testers
make -C ft_containers re

# 12. Build Exam Rank 05 (new 2025: vect2, bigint, polyset, Life, BSQ)
make -C exam_rank_05

# 13. Build Exam Rank 06 (new 2025: mini_serv + mini_db)
make -C exam_rank_06

# 14. FT_Transcendence (Django full-stack Pong web app)
cd ft_transcendence && pip install -r requirements.txt && python manage.py migrate
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

# FdF: 5 suites / 248 tests (parse, math, draw, color, hooks)
make -C fdf/tests

# Fract'ol: 6 suites / 1,077 tests (headless, no X11 required)
make -C fract-ol/tests

# Push_swap: 388 verifications (cases, 20-benchmark, differential)
bash push_swap/tests/run_tests.sh

# Minishell: 7 C suites / 60 tests (basic, variables, quotes, pipes, redirects, logical, wildcards)
bash minishell/tests/run_tests.sh

# Philosophers: 87/87 tests (Python, mandatory + bonus)
python3 philosophers/tests/test_philo.py

# Codexion: 157/158 student tests (log format, monotonicity, burnout deadline, cooldown, stability)
#   via the external Testeur-Codexion suite (./run_all_tests.sh <codexion dir> --no-valgrind)

# Piscine Python: 44/44 reference checker (modules 00-08) + 65/65 home pytest (09-10 + structure)
cd piscine_python && python3 -m pytest tests/

# A-Maze-ing: 20 pytest tests + independent maze analyzer
cd a_maze_ing && python3 -m pytest tests/

# Inception: 58-check static suite (YAML, Dockerfiles, compose, Nginx, php-fpm, MariaDB, .env)
make -C inception test

# Exam Rank 02: 4 C suites / 111 tests (level 1-4)
make -C exam_rank02 test

# Exam Rank 03: Python suite (90/90)
python3 exam_rank_03/tests/test_exam_rank_03.py

# Exam Rank 04 — microshell: 11 unit + 22 integration tests
make -C exam_rank_04 test

# Cub3d: 8 suites / 186 tests (parser, player, raycasting, doors, sprites, integration, robustness)
bash cub3d/tests/run_tests.sh

# NetPratice: 8 suites / 206 tests (IP, mask, routing, JSON, solution, validate, solver, levels)
make -C netpratice test

# MiniRT: 9 suites / 14,660 tests (matrix, parser, objects, intersect, lighting, camera, integration)
bash miniRT/tests/run_tests.sh

# CPP Modules 00-09: 93 checks (build, functional, edge cases, 3000-element stress, ASan/TSan)
python3 cpp_modules/tests/test_cpp_modules.py

# FT_Containers: internal ASan/UBSan suite (3,408) + 5 testers (llefranc, monkey, mazoise, mli, static)
#   + official libstdc++ GCC 4.9.4 testsuite (PASS 172, FAIL 18 *_neg, 0 applicable bugs)
make -C ft_containers re

# Exam Rank 05 (new 2025): full suite (vect2, bigint, polyset, Life, BSQ)
python3 exam_rank_05/tests/run_tests.py

# Exam Rank 06 (new 2025): mini_serv + mini_db (97/97)
python3 exam_rank_06/tests/run_tests.py

# FT_Transcendence: 147-test suite (engine, AI, bracket, API, security, student patterns)
cd ft_transcendence && python3 tests/run_tests.py
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

**`my_42_cursus`** est le monorepo regroupant l'ensemble de mes projets réalisés à l'**École 42** — de la **Piscine C** et de la **Piscine Python** jusqu'aux projets du **Tronc Commun (*Common Core*)**. Le portfolio couvre l'ingénierie système en C pur (**Libft**, **Minitalk**, **Pipex**, **Philosophes**, **Codexion**), le travail algorithmique et Python (générateur de labyrinthe **A-Maze-ing**, les modules **Piscine Python** 00–10), l'infrastructure DevOps (**Born2beRoot**, stack Docker **Inception**), ainsi que les tracks C++ et web. Chaque projet est développé intégralement à partir de zéro selon des contraintes d'ingénierie strictes : compilation sous **`-Wall -Wextra -Werror`**, respect intégral de la **Norme 42 v4.1**, gestion défensive des erreurs et **zéro fuite mémoire**.

Chaque projet est structuré proprement selon les standards de l'industrie adaptés aux règles de la Moulinette 42 (voir le guide complet [`docs/C_PROJECT_ARCHITECTURE_GUIDE.md`](docs/C_PROJECT_ARCHITECTURE_GUIDE.md)) et validé par plus de **57 000 tests automatisés** (tests unitaires, différentiels face à la `glibc`, injection de pannes `malloc`, AddressSanitizer, UBSan et Valgrind).

### Sommaire

- [Catalogue des projets](#catalogue-des-projets)
- [Architecture du dépôt](#architecture-du-dépôt)
- [Démarrage rapide](#démarrage-rapide)
- [Lancer les suites de tests](#lancer-les-suites-de-tests)
- [Standards de qualité & ingénierie](#standards-de-qualité--ingénierie)
- [Auteur](#auteur)

---

### Catalogue des projets

Le portfolio est organisé selon les **4 cercles** du cursus 42 — des
fondations (cercle 1) jusqu'à la concurrence avancée (cercle 3) et le
track C++ (cercle 4) — complétés par les recueils d'examen **Exam Rank**.

#### 🔵 Cercle 1 — Fondements

| Étape | Projet | Livrable | Compétences clés | Bonus | Validation automatisée |
| :--- | :--- | :--- | :--- | :---: | :--- |
| **Rang 00** | [**`libft/`**](libft/README.md) | `libft.a` *(43 fonctions)* | Gestion mémoire bas niveau, réimplémentation `libc`, allocateurs de chaînes, listes chaînées `t_list` *(Sujet 2026 v19.3)* | Intégrés *(v19.3)* | **11 407 assertions** + injection d'échecs `malloc` + ASan/UBSan |
| **Rang 01** | [**`ft_printf/`**](ft_printf/README.md) | `libftprintf.a` | Fonctions variadiques (`stdarg.h`), rendu 100 % sur la pile (`0 malloc`), écriture robuste `write(2)`, automate de parsing | ✅ Complet (`-0.# +*`) | **26 921 tests différentiels** vs `glibc` + ASan/UBSan + Tripouille |
| **Rang 01** | [**`get_next_line/`**](get_next_line/README.md) | `get_next_line*.[ch]` | Variables `static`, `read(2)` bufferisé, tampon dynamique $\Theta(L)$, liste chaînée multi-FD *(Sujet v14.3)* | ✅ 1 `static` + Multi-FD | **Tripouille + Francinette `STRICT_MEM`** (1 000 FDs, pannes `malloc`) + GNL Station + ASan/UBSan |
| **Rang 02** | [**`born2beroot/`**](born2beroot/README.fr.md) | VM Debian + `monitoring.sh` | Partitionnement LUKS2 + LVM, politique `pam_pwquality`/`login.defs`, durcissement `sudoers`, UFW, AppArmor, systemd + cron | ✅ WordPress · vsftpd · fail2ban | **106 vérifications statiques + 167 exigences + 5 testeurs communautaires (80 contrôles) + `verify.sh` (64)** |
| **DevOps** | [**`inception/`**](inception/README.md) | Stack Docker Compose | Infrastructure web complète via **Docker Compose** : NGINX (TLS 1.2/1.3, port 443) + WordPress/php-fpm (TCP 9000) + MariaDB, images construites depuis **Debian bookworm**, secrets via Docker secrets | ✅ Adminer · Redis · FTP · site perso | **58 tests** (analyse statique YAML/Dockerfile/compose/Nginx/php-fpm/MariaDB/.env) |

#### 🟢 Cercle 2 — Systèmes de base

| Étape | Projet | Livrable | Compétences clés | Bonus | Validation automatisée |
| :--- | :--- | :--- | :--- | :---: | :--- |
| **Rang 02** | [**`minitalk/`**](minitalk/README.md) | `client` & `server` | IPC UNIX par signaux (`SIGUSR1`/`SIGUSR2`), `sigaction` (`SA_SIGINFO`), encodage bit-à-bit avec ACK, `volatile sig_atomic_t` | ✅ ACK + UTF-8 | **49 tests bout-en-bout** (~39 ms / 100 chars, 20 000 chars, erreurs PID) |
| **Rang 02** | [**`pipex/`**](pipex/README.md) | binaire `pipex` | Pipes UNIX : `fork` + `dup2` + `execve`, chaînes de N pipes, here_doc, résolution du `PATH`, hygiène des descripteurs (ni deadlock ni fuite) | ✅ N pipes + here_doc | **131 contrôles** (différentiels vs bash, ASan/UBSan, robustesse) + **3 testeurs tiers (71)** |
| **Rang 02** | [**`so_long/`**](so_long/README.fr.md) | binaires `so_long` & `so_long_bonus` | Fenêtrage miniLibX, textures XPM, hooks d’événements, parsing et validation de carte, rendu par tuiles, boucle de jeu | ✅ Ennemis · animation · compteur à l’écran | **106 contrôles** (10 règles de carte, gameplay, bonus, ASan/UBSan sur 41 cartes, Norme) + **testeur tiers (44)** |
| **Rang 02** | [**`push_swap/`**](push_swap/README.fr.md) | binaire `push_swap` | Trier deux piles d'entiers en un minimum de mouvements : sorter + pousseur, opérations SA/SB/PA/PB, moteur de décision optimisé | ✅ | **388 vérifications** (cas, benchmark 20, différentiel) |
| **Rang 02** | [**`fract-ol/`**](fract-ol/README.md) | binaire `fract-ol` | Rendu de fractales 2D miniLibX (Mandelbrot, Julia), calcul d'échappement, zoom à la souris, palettes de couleurs | ✅ Burning Ship · Tricorn · Phoenix · Multibrot · multithreading | **1 077 tests** (6 suites, sans X11 requis) |
| **Rang 02** | [**`fdf/`**](fdf/README.md) | binaire `fdf` | Rendu de terrain 3D en wireframe miniLibX, parsing de height-maps `.fdf`, projection isométrique, gradient de couleur | ✅ Rotation 3D · 5 modes de couleur · rainbow HSL→RGB | **248 tests** (5 suites : parse, math, draw, color, hooks) |
| **Rang 04** | [**`cub3d/`**](cub3d/README.md) | binaire `cub3d` | Raycasting (Wolfenstein 3D) : lancer de rayons DDA, cartographie de textures, portes, sprites, minimap, miniLibX | ✅ Bonus complet (portes, sprites, minimap) | **186 tests** (8 suites : parser, player, raycasting, portes, sprites, intégration, robustesse) |
| **Rang 04** | [**`netpratice/`**](netpratice/) | 10 JSON de solution | Adressage IPv4, masques de sous-réseau et routage sur 10 niveaux : calculs CIDR/masque, routage plus long préfixe, solveur & validateur | ✅ Solveur complet + pipelines sans fuite de FD | **206 tests** (8 suites : ip, mask, routing, json, solution, validate, solver, levels) |

#### 🟡 Cercle 3 — Concurrence avancée

| Étape | Projet | Livrable | Compétences clés | Bonus | Validation automatisée |
| :--- | :--- | :--- | :--- | :---: | :--- |
| **Rang 03** | [**`minishell/`**](minishell/README.md) | binaire `minishell` | Mini shell : lexeur/parsing, expansion d'environnement et variables, pipes, redirections, opérateurs logiques (`&&`/`||`), wildcards | ✅ Fonctionnalités bonus | **60 tests** (7 suites C : basic, variables, quotes, pipes, redirects, logiques, wildcards) |
| **Rang 03** | [**`philosophers/`**](philosophers/README.md) | binaires `philo` & `philo_bonus` | Philosophes dînant : threads + mutex (obligatoire), processus + sémaphores nommés (bonus), timing et messagerie sans courses | ✅ Sémaphores nommés | **87/87 tests** (suite Python, obligatoire + bonus) |
| **Rang 03** | [**`codexion/`**](codexion/README.md) | binaire `codexion` | Concurrence sur le modèle des philosophes dînants : `n_coders` threads devant détenir **les deux** dongles USB voisins pour compiler, thread monitor traquant les deadlines de burnout, mutex + variables de condition POSIX, **aucune variable globale** (un seul `t_sim`), ordonnancement FIFO & EDF | ✅ Ordonnanceur EDF · cooldown des dongles | **157/158 tests étudiants** (format de log, monotonie, deadline de burnout, cooldown, stabilité) + **Norme v4.1** |

#### 🔴 Cercle 4 — C++

| Étape | Projet | Livrable | Compétences clés | Bonus | Validation automatisée |
| :--- | :--- | :--- | :--- | :---: | :--- |
| **Rang 03** | [**`cpp_modules/`**](cpp_modules/README.md) | 37 exercices (`cpp_00`–`cpp_09`) | POO : classes, héritage, exceptions, templates, forme canonique ; STL (`map`/`vector`/`deque`/`stack`) | ✅ BSP · Forms · DiamondTrap | **93/93 tests** (build `-Wall -Wextra -Werror -std=c++98`, fonctionnels, cas limites, stress 3000, ASan/TSan) |
| **C++98** | [**`ft_containers/`**](ft_containers/subjects/ft_containers_subject.md) | 10 headers `<container>.hpp` | Conteneurs STL recodés de zéro en C++98, namespace `ft`, `std::allocator` : `vector`, `map`, `stack` (+ bonus `set`, `deque`, `queue`, `multimap`, `multiset`), arbres rouge-noir, `iterator_traits`, `reverse_iterator`, `enable_if`, `is_integral`, `lexicographical_compare`/`equal`, `pair`/`make_pair` | ✅ Tous les conteneurs bonus | **3 408 assertions ASan/UBSan** + **5 testeurs** (llefranc, monkey, mazoise, mli, static) + **testsuite OFFICIELLE libstdc++ GCC 4.9.4** (0 bug applicable) |

#### 🐍 Track Python — Piscine Python & Algorithmique

| Étape | Projet | Livrable | Compétences clés | Bonus | Validation automatisée |
| :--- | :--- | :--- | :--- | :---: | :--- |
| **Piscine** | [**`piscine_python/`**](piscine_python/README.md) | Modules 00–10 | **Piscine Python** complète : fondamentaux, POO, exceptions, collections, E/S, ABC, import/packages, patterns de conception, environnement & dépendances, validation Pydantic v2, programmation fonctionnelle | ✅ Tous les modules bonus | **44/44** checker de référence (modules 00–08) + **65/65** suite pytest maison (modules 09–10 + structure) |
| **Algorithmique** | [**`a_maze_ing/`**](a_maze_ing/README.md) | `a_maze_ing` + package `mazegen` | **Générateur, solveur & visualiseur** de labyrinthes : config `KEY=VALUE`, génération DFS/Prim, plus court chemin BFS, sortie fichier murs hexadécimal, UI terminale, **« 42 »** caché, labyrinthe parfait vs plateau Pac-Man | ✅ Algorithme Prim · SEED · `mazegen` réutilisable (pip) | **20 tests pytest** + analyseur de labyrinthe indépendant (cohérence des murs, connectivité, zones 3×3, boucles, optimalité du chemin) |

#### 🟣 Projet final — Web

| Étape | Projet | Livrable | Compétences clés | Bonus | Validation automatisée |
| :--- | :--- | :--- | :--- | :---: | :--- |
| **Final** | [**`ft_transcendence/`**](ft_transcendence/README.md) | Application web full-stack | Jeu Pong multijoueur en temps réel (websockets, Django + Channels + SPA vanilla JS), IA adversaire, tournois + matchmaking, chat temps réel, OAuth 2.0 (42), JWT + 2FA, gestion utilisateurs, personnalisation, multilingue, RGPD | ✅ Tous les modules | **147/147 tests** (moteur, IA, bracket, API, sécurité, patterns étudiants) |

#### 📚 Exam Rank — Recueils d'examen

| Étape | Projet | Livrable | Compétences clés | Bonus | Validation automatisée |
| :--- | :--- | :--- | :--- | :---: | :--- |
| **Rang 02** | [**`exam_rank02/`**](exam_rank02/README.md) | binaire `exam_rank02` | Pratique Exam Rank 02 — 4 niveaux : chaînes, manipulation avancée, algorithmes, mémoire & listes chaînées | ✅ | **111 tests** (4 suites C, niveaux 1–4) |
| **Rang 03** | [**`exam_rank_03/`**](exam_rank_03/README.md) | 8 programmes | Pratique Exam Rank 03 — niveau 1 (filter, ft_scanf, get_next_line) et niveau 2 (n_queens, permutations, powerset, rip, tsp) | ✅ | **90/90 tests** |
| **Rang 04** | [**`exam_rank_04/`**](exam_rank_04/README.md) | binaire `microshell` | Exam Rank 04 — microshell : exécution de commandes shell, pipelines `|`/`;`, builtin `cd`, concurrence sans fuite de descripteurs | ✅ `&&`/`||` short-circuit | **33 tests** (11 unitaires parseur + 22 intégration) |
| **Rang 05** | [**`miniRT/`**](miniRT/README.md) | binaire `miniRT` | Exam Rank 05 — traceur de rayons CPU : parsing de scènes `.rt`, intersections, éclairage, ombres, réflexions, caméras | ✅ Fonctionnalités complètes | **14 660 tests** (9 suites : matrix, parser, objects, intersect, lighting, camera, intégration) |
| **Rang 05** | [**`exam_rank_05/`**](exam_rank_05/README.md) | `vect2` · `bigint` · `polyset` · `life` · `bsq` | Exam Rank 05 (nouveau 2025) — niveau 0 C++98 (vecteur 2D, entier à précision arbitraire, ensemble polymorphe) et niveau 1 C (Jeu de la vie, plus grand carré) | ✅ Tous les bonus | **Suite complète** (unitaires C++ + intégration Python, testers étudiants) |
| **Rang 06** | [**`exam_rank_06/`**](exam_rank_06/README.md) | `mini_serv` · `mini_db` | Exam Rank 06 (nouveau 2025) — `mini_serv` : chat TCP multi-clients non-bloquant (`select`) ; `mini_db` : base clé-valeur sur TCP avec persistance | ✅ Périmètre complet | **97/97 tests** (fonctionnels, statiques, testers étudiants) |

---

### Architecture du dépôt

Le dépôt est organisé sans aucun fichier binaire parasite (`.o`, `.a`, exécutables ignorés via `.gitignore`) et sans double imbrication de dossiers :
- **`ft_printf/`** et **`minitalk/`** adoptent le **Pitchfork Layout** adapté 42 : headers dans `include/`, sources dans `src/`, objets compilés hors-source dans `obj/` (non suivi par Git), documentation dans `docs/` et harnais de validation dans `tests/`.
- **`get_next_line/`** conserve les **6 fichiers de rendu imposés à la racine** (`get_next_line.{c,h}`, `get_next_line_utils.c` et leurs variantes `_bonus.{c,h}`) — le sujet v14.3 n'attend **aucun `Makefile`** — tout en déportant le sujet dans `docs/`, les testeurs dans `tests/` et `gnlTester/`, et l'orchestrateur dans `run_tests.sh`.
- **`libft/`** conserve volontairement `Makefile`, `libft.h`, `README.md` et les 43 `ft_*.c` à sa racine conformément à l'exigence stricte du **Chapitre IV.1 du sujet Libft 2026 v19.3**, tout en isolant le sujet dans `docs/` et les tests dans `tests/`.
- **`piscine42/`** préserve des dossiers `exXX/` strictement purs (prêts pour la Moulinette) et déporte l'intégralité des bancs d'essai dans `piscine42/tests/`.
- **`piscine_python/`** est la **Piscine Python** (modules 00–10) : fondamentaux, POO, exceptions, collections, E/S, ABC, import/packages, patterns, env & dépendances, validation Pydantic v2, programmation fonctionnelle. Validée par un **checker de référence 44/44** (modules 00–08) et une **suite pytest maison 65/65** (modules 09–10 + structure).
- **`a_maze_ing/`** est le **générateur, solveur et visualiseur de labyrinthes** : parsing `KEY=VALUE`, génération DFS/Prim, plus court chemin BFS, sortie hexadécimale, UI terminale, « 42 » caché, labyrinthe parfait vs Pac-Man. Le cœur génératif est isolé dans le package **`mazegen`** (réutilisable, installable via pip). Validée par **20 tests pytest** + un analyseur de labyrinthe indépendant.
- **`inception/`** est l'infrastructure **DevOps** : stack **Docker Compose** NGINX (TLS) + WordPress/php-fpm + MariaDB (images depuis Debian bookworm, secrets via Docker secrets) + services bonus (Adminer, Redis, FTP, site perso). Validée par une **suite statique de 58 tests** (YAML, Dockerfiles, compose, Nginx, php-fpm, MariaDB, `.env`).
- **`fdf/`** est le rendu de terrain 3D en wireframe miniLibX : parsing de height-maps `.fdf`, projection isométrique, 5 modes de couleur, rotation 3D. Validé par **5 suites / 248 tests**.
- **`fract-ol/`** est le rendu de fractales 2D miniLibX : Mandelbrot, Julia + bonus (Burning Ship, Tricorn, Phoenix, Multibrot), rendu multithread. Validé par **6 suites / 1 077 tests** (sans X11).
- **`push_swap/`** trie deux piles d'entiers en un minimum de mouvements : `push_swap.c` (sorteur + pousseur), `push_swap_utils.c` (opérations SA/SB/PA/PB), validé par **388 vérifications** (cas, benchmark, différentiel).
- **`minishell/`** est un mini shell C : lexeur/parser, expansion d'environnement et variables, pipes, redirections, opérateurs logiques (`&&`/`||`) et wildcards. Validé par **7 suites C / 60 tests** (`tests/run_tests.sh`).
- **`philosophers/`** implémente le problème des philosophes dînants : `philo/` (threads + mutex, obligatoire) et `philo_bonus/` (processus + sémaphores nommés, bonus). Validé par une **suite Python 87/87**.
- **`codexion/`** est une simulation de concurrence sur le modèle des philosophes dînants : `n_coders` threads doivent détenir les deux dongles USB voisins pour compiler, un thread **monitor** traque les deadlines de burnout, synchronisation par mutex + variables de condition POSIX, **aucune variable globale** (un seul `t_sim` partagé), ordonnancement **FIFO/EDF** via tas mini par dongle. Validée par la **suite étudiante 157/158** et conforme à la **Norme v4.1**.
- **`exam_rank02/`** est le recueil d'examen Rank 02 : 4 niveaux (chaînes, manipulation avancée, algorithmes, mémoire & listes) regroupés par dossier `level_1/`…`level_4/`, prototypes dans `include/exam_rank02.h`. Validé par **4 suites C / 111 tests** (`make test`).
- **`exam_rank_03/`** est le recueil d'examen Rank 03 : niveau 1 (`filter`, `ft_scanf`, `get_next_line`) et niveau 2 (`n_queens`, `permutations`, `powerset`, `rip`, `tsp`), chacun autonome. Validé par une **suite Python 90/90**.
- **`cub3d/`** est le raycasting (Wolfenstein 3D) : lancer de rayons DDA, cartographie de textures, portes, sprites, minimap, miniLibX — **Rang 04 avec bonus complet**. Validé par **8 suites / 186 tests**.
- **`netpratice/`** est le moteur d'adressage IPv4, de masques de sous-réseau et de routage sur 10 niveaux : calculs CIDR/masque, routage par plus long préfixe, solveur et validateur, livrant 10 JSON de solution. Validé par **8 suites / 206 tests**.
- **`exam_rank_04/`** est l'examen **Rank 04 — microshell** : exécution de commandes shell, pipelines `|`/`;`, builtin `cd`, concurrence sans fuite de descripteurs (un pipe ouvert à la fois, validé avec `ulimit -n 30`), bonus `&&`/`||` short-circuit. Utilise **uniquement les 16 fonctions autorisées**. Validé par **11 tests unitaires + 22 intégration**.
- **`miniRT/`** est le traceur de rayons CPU (Exam Rank 05) : parsing de scènes `.rt`, intersections, éclairage, ombres, réflexions, caméras. Validé par **9 suites / 14 660 tests**.
- **`cpp_modules/`** contient les 10 modules **CPP 00–09** : chaque exercice (`cpp_XX/exYY/`) est autonome avec son `Makefile` et ses fichiers `*.hpp`/`*.cpp` en **C++98 strict** (`-Wall -Wextra -Werror`), forme canonique obligatoire de `cpp_02` à `cpp_09`, STL réservée aux modules 08–09. Les énoncés officiels sont dans `subjects/` et la suite de tests Python dans `tests/`.
- **`ft_containers/`** recode les **conteneurs STL en C++98 strict** dans le namespace **`ft`** avec `std::allocator` : `vector`, `map` (arbre rouge-noir), `stack` (obligatoires) + `set`, `deque`, `queue`, `multimap`, `multiset` (bonus), ainsi que `iterator_traits`, `reverse_iterator`, `enable_if`, `is_integral`, `lexicographical_compare`/`equal`, `pair`/`make_pair`. Bibliothèque entièrement en headers (`includes/`), aucun binaire suivi. Validée par une **suite interne ASan/UBSan (3 408 assertions)**, **5 testeurs élèves** (llefranc, monkey, mazoise, mli, static) et la **testsuite OFFICIELLE libstdc++ GCC 4.9.4** (0 bug applicable). Le `Makefile` orchestre la suite interne et tous les testeurs via `make re`.

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

# Compiler FdF (terrain 3D wireframe), Fract'ol (fractales 2D) et push_swap
make -C fdf
make -C fract-ol
make -C push_swap

# Compiler Minishell et Philosophers (Rang 03)
make -C minishell
make -C philosophers

# Compiler Exam Rank 02 (binaire 4 niveaux)
make -C exam_rank02

# Compiler Exam Rank 04 — microshell
make -C exam_rank_04

# Compiler Cub3d (raycasting Wolfenstein 3D, Rang 04) et MiniRT (traceur de rayons, Rang 05)
make -C cub3d
make -C miniRT

# Compiler NetPratice (moteur IPv4/routage, Rang 04)
make -C netpratice

# Compiler CPP Modules (chaque exercice dans cpp_modules/cpp_XX/exYY)
make -C cpp_modules/cpp_09/ex02

# Compiler FT_Containers (conteneurs STL en header-only, C++98) + suite interne + 5 testeurs
make -C ft_containers re

# Compiler Exam Rank 05 (nouveau 2025 : vect2, bigint, polyset, Life, BSQ)
make -C exam_rank_05

# Compiler Exam Rank 06 (nouveau 2025 : mini_serv + mini_db)
make -C exam_rank_06

# FT_Transcendence (application web Pong full-stack Django)
cd ft_transcendence && pip install -r requirements.txt && python manage.py migrate
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

# FdF : 5 suites / 248 tests (parse, math, draw, color, hooks)
make -C fdf/tests

# Fract'ol : 6 suites / 1 077 tests (sans X11 requis)
make -C fract-ol/tests

# Push_swap : 388 vérifications (cas, benchmark, différentiel)
bash push_swap/tests/run_tests.sh

# Minishell : 7 suites C / 60 tests (basic, variables, quotes, pipes, redirects, logiques, wildcards)
bash minishell/tests/run_tests.sh

# Philosophers : 87/87 tests (Python, obligatoire + bonus)
python3 philosophers/tests/test_philo.py

# Codexion : 157/158 tests étudiants (format de log, monotonie, deadline de burnout, cooldown, stabilité)
#   via le testeur externe Testeur-Codexion (./run_all_tests.sh <dir codexion> --no-valgrind)

# Piscine Python : 44/44 checker de référence (00-08) + 65/65 pytest maison (09-10 + structure)
cd piscine_python && python3 -m pytest tests/

# A-Maze-ing : 20 tests pytest + analyseur de labyrinthe indépendant
cd a_maze_ing && python3 -m pytest tests/

# Inception : suite statique de 58 tests (YAML, Dockerfiles, compose, Nginx, php-fpm, MariaDB, .env)
make -C inception test

# Exam Rank 02 : 4 suites C / 111 tests (niveaux 1–4)
make -C exam_rank02 test

# Exam Rank 03 : suite Python (90/90)
python3 exam_rank_03/tests/test_exam_rank_03.py

# Exam Rank 04 — microshell : 11 tests unitaires + 22 intégration
make -C exam_rank_04 test

# Cub3d : 8 suites / 186 tests (parser, player, raycasting, portes, sprites, intégration, robustesse)
bash cub3d/tests/run_tests.sh

# NetPratice : 8 suites / 206 tests (IP, masque, routage, JSON, solution, validate, solver, levels)
make -C netpratice test

# MiniRT : 9 suites / 14 660 tests (matrix, parser, objects, intersect, lighting, camera, intégration)
bash miniRT/tests/run_tests.sh

# CPP Modules 00-09 : 93 checks (build, fonctionnels, cas limites, stress 3000, ASan/TSan)
python3 cpp_modules/tests/test_cpp_modules.py

# FT_Containers : suite interne ASan/UBSan (3 408) + 5 testeurs (llefranc, monkey, mazoise, mli, static)
#   + testsuite OFFICIELLE libstdc++ GCC 4.9.4 (PASS 172, FAIL 18 *_neg, 0 bug applicable)
make -C ft_containers re

# Exam Rank 05 (nouveau 2025) : suite complète (vect2, bigint, polyset, Life, BSQ)
python3 exam_rank_05/tests/run_tests.py

# Exam Rank 06 (nouveau 2025) : mini_serv + mini_db (97/97)
python3 exam_rank_06/tests/run_tests.py

# FT_Transcendence : suite 147 tests (moteur, IA, bracket, API, sécurité, patterns étudiants)
cd ft_transcendence && python3 tests/run_tests.py
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
