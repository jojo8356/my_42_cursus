<div align="center">

# Piscine 42 — Complete C & UNIX Shell Reference

**Comprehensive study & verification workspace for the École 42 C Piscine (2025–2026 Edition): 20 modules (`Shell 00–01`, `C 00–13`, `Rush 00–02`, `BSQ`), archived subjects, and non-intrusive automated test drivers.**

[![Language: C & Shell](https://img.shields.io/badge/Language-C_%26_POSIX_Shell-00599C?style=flat-square&logo=c&logoColor=white)](#curriculum-matrix)
[![42 Norm](https://img.shields.io/badge/42_Norm-Strict_Compliance-00BABC?style=flat-square)](#code-style--moulinette-isolation)
[![Modules](https://img.shields.io/badge/Modules-20_Complete_(Shell_%C2%B7_C_%C2%B7_Rushes_%C2%B7_BSQ)-success?style=flat-square)](#curriculum-matrix)
[![Test Drivers](https://img.shields.io/badge/Test_Runner-20_Automated_Drivers-brightgreen?style=flat-square)](#running-the-test-suite)

**[ 🇬🇧 English ](#-english-version) · [ 🇫🇷 Version Française ](#-version-française)**

</div>

---

## 🇬🇧 English Version

> **Academic Integrity Notice**: The 42 Piscine is designed around peer-learning and problem-solving from first principles. Use this repository strictly as a post-attempt study reference, always write and understand every line of your own code, and respect your campus rules (exams are strictly closed-book).

### Table of Contents

- [Overview](#overview)
- [Curriculum Matrix](#curriculum-matrix)
- [Capstone & Rush Highlights](#capstone--rush-highlights)
- [Project Architecture & Moulinette Isolation](#project-architecture--moulinette-isolation)
- [Running the Test Suite](#running-the-test-suite)

---

### Overview

**`piscine42`** archives a complete, end-to-end progression through the intensive 4-week **École 42 C Selection Piscine**:
- **2 UNIX Shell modules** (`shell00`, `shell01`) covering permissions, `tar`, `git`, `find`, `sed`, `awk`, and base conversions.
- **14 progressive C modules** (`c00` through `c13`) advancing from raw `write(2)` syscalls and pointer arithmetic to variadic-free CLI tools (`cat`, `tail`, `hexdump`), function pointers, singly linked lists (`t_list`), and binary search trees (`t_btree`).
- **3 weekend team Rushes** (`rush00`, `rush01`, `rush02`) and the final **BSQ** (*Biggest Square*) dynamic-programming project.
- **34 official subject PDFs** archived in `subjects/pdf/` alongside **20 automated POSIX `sh` + C test drivers** in `tests/`.

---

### Curriculum Matrix

| Directory | Module | Exercises | Core Systems & Algorithmic Topics |
| :--- | :--- | :---: | :--- |
| `shell00/` | **Shell 00** | `ex00` → `ex09` | File permissions, symlinks, `tar`, SSH keys, `git log`/`status`, `patch`, custom `magic` file |
| `shell01/` | **Shell 01** | `ex01` → `ex08` | UNIX pipelines (`id`, `find`, `wc`, `ifconfig`, `sed`, `awk`, `tr`, `bc` arbitrary-base math) |
| `c00/` | **C 00** | `ex00` → `ex08` | Direct `write(2)` output, loops, `ft_putnbr`, $k$-combinations (`ft_print_combn`) |
| `c01/` | **C 01** | `ex00` → `ex08` | Pointers, multi-level dereferencing (`*********nbr`), array reversal & sorting |
| `c02/` | **C 02** | `ex00` → `ex12` | String primitives, `strlcpy`, hex escapes, hex+ASCII memory dump (`ft_print_memory`) |
| `c03/` | **C 03** | `ex00` → `ex05` | Lexicographical comparison (`strcmp`, `strncmp`), `strcat`, `strstr`, `strlcat` |
| `c04/` | **C 04** | `ex00` → `ex05` | Custom-sign `ft_atoi`, arbitrary-radix `ft_putnbr_base` & `ft_atoi_base` |
| `c05/` | **C 05** | `ex00` → `ex08` | Iteration vs. recursion, integer $\sqrt{n}$, primality testing, **10-Queens** backtracking (`ft_ten_queens_puzzle`) |
| `c06/` | **C 06** | `ex00` → `ex03` | Process entry point `main(int argc, char **argv)` & ASCII argument sorting |
| `c07/` | **C 07** | `ex00` → `ex05` | Heap allocation (`malloc`/`free`), `ft_strdup`, `ft_strjoin`, `ft_convert_base`, `ft_split` |
| `c08/` | **C 08** | `ex00` → `ex05` | Header guards (`#ifndef`), parameterized macros, `typedef struct` (`t_stock_str`) |
| `c09/` | **C 09** | `ex00` → `ex02` | Static library shell builder (`libft_creator.sh`), `Makefile` rules, `ft_split` |
| `c10/` | **C 10** | `ex00` → `ex03` | File I/O (`open`, `read`, `close`, `errno`, `strerror`): `ft_display_file`, `ft_cat`, `ft_tail -c`, `ft_hexdump -C` |
| `c11/` | **C 11** | `ex00` → `ex07` | Higher-order function pointers (`foreach`, `map`, `count_if`), `do-op` dispatch table, custom comparator sort |
| `c12/` | **C 12** | `ex00` → `ex17` | Complete singly linked list library (`t_list`: reverse, remove_if, merge, sort, sorted insert) |
| `c13/` | **C 13** | `ex00` → `ex07` | Binary trees (`t_btree`): preorder/inorder/postorder DFS, BST insert/search, height & BFS (`apply_by_level`) |
| `rush00/` | **Rush 00** | `ex00` + variants | Parameterized ASCII art box renderer (`rush00`..`rush04` + unified dispatcher bonus) |
| `rush01/` | **Rush 01** | `ex00` | **$N \times N$ Skyscrapers** (`3×3` to `9×9`) constraint-satisfaction puzzle solver via pruning & backtracking |
| `rush02/` | **Rush 02** | `ex00` | Arbitrary-precision string number-to-words converter up to $10^{36}$ (*undecillion* EN / *sextillion* FR, `stdin`, `-w` formatting, `free`) |
| `bsq/` | **BSQ** | Capstone | **Biggest Square** obstacle-grid solver using $O(R \cdot C)$ dynamic programming |

---

### Capstone & Rush Highlights

- **`bsq/` (Biggest Square)**: Parses arbitrary-sized obstacle maps from files or `stdin`, validates strict map invariants (`map error` on malformed headers/lines), and computes the top-most, left-most maximal empty square in linear $O(\text{rows} \times \text{cols})$ time.
- **`rush01/` (Skyscrapers)**: Solves valid $N \times N$ (`3×3` through `9×9`) visibility grids using row pre-filtering, bitmask pruning, and depth-first constraint propagation.
- **`rush02/` (Numbers in Words)**: Parses custom `.dict` key-value files (`numbers.dict`, `numbers_fr.dict`) with string-based keys, converts arbitrary-length integers up to **$10^{36}$** (*undecillion* / *sextillion* and beyond with larger dictionaries), supports interactive/piped `stdin` (zero-arg `./rush-02` or `"-"`), grammatical `-w` formatting (`-`, `,`, `and`), French plural rules, and deterministic `dict_free()` cleanup.

---

### Project Architecture & Moulinette Isolation

Every exercise directory (`cXX/exYY/`) contains **exclusively the exact deliverable file(s)** expected by 42's *Moulinette* — zero `main()` functions inside library exercises, zero `.o` files, and zero test clutter:

```text
piscine42/
├── c00/ .. c13/             # Pure Moulinette-ready exXX/ deliverables
├── shell00/ .. shell01/     # Pure Shell exercise deliverables
├── rush00/ .. rush02/       # Weekend team projects (Makefiles clean & untracked binaries)
├── bsq/                     # Final capstone project (Makefile + modular .c/.h)
├── subjects/
│   └── pdf/                 # 34 archived Piscine subject PDFs (2025-2026)
├── tests/                   # 20 automated test drivers (C harnesses + test_*.sh)
├── run_all_tests.sh         # Master test runner
└── README.md                # Bilingual documentation (EN / FR)
```

---

### Running the Test Suite

```bash
cd piscine42

# Run all available module & project drivers
./run_all_tests.sh

# Run a single C day or Shell module
./run_all_tests.sh c05
./run_all_tests.sh c12 c13

# Run capstone & rush suites
./run_all_tests.sh bsq rush01 rush02
```

Each driver compiles the target exercise with `-Wall -Wextra -Werror`, compares outputs against reference expectations, cleans temporary artifacts, and exits non-zero on any failure.

---
---

## 🇫🇷 Version Française

> **Intégrité académique** : la Piscine 42 est conçue pour apprendre en cherchant par soi-même et entre pairs. Utilisez ce dépôt uniquement comme support d'étude et de compréhension après avoir codé vos propres solutions (les examens se passent sans aide ni accès réseau).

### Présentation

**`piscine42`** rassemble l'intégralité du parcours de la **Piscine C de l'École 42 (édition 2025–2026)** :
- **2 modules Shell UNIX** (`shell00`, `shell01`)
- **14 modules de C progressifs** (`c00` à `c13`, ~130 fonctions et utilitaires système)
- **3 Rushs d'équipe** (`rush00`, `rush01`, `rush02`) et le projet final **BSQ** (*Biggest Square*)
- **34 sujets officiels archivés en PDF** dans `subjects/pdf/`
- **20 drivers de tests automatisés** isolés dans `tests/` et orchestrés par `run_all_tests.sh`

### Organisation propre & compatibilité Moulinette

```text
c00/ex07/ft_putnbr.c     ← livrable pur (seul fichier attendu par la Moulinette)
tests/c00_test.c         ← programme de test unitaire (jamais dans exXX/)
tests/test_c00.sh        ← script driver : compile (-Wall -Wextra -Werror) + exécute + vérifie
```

- Les dossiers `exNN/` ne contiennent **aucun `main` parasite**, **aucun `.o`** et **aucun binaire suivi par Git**.
- Les solutions respectent strictement la **Norme 42** (tabulations, 80 colonnes max, ≤ 25 lignes par fonction, ≤ 5 fonctions par fichier, sans `for` ni opérateur ternaire).

### Lancer les tests automatisés

```bash
cd piscine42

./run_all_tests.sh                # Lance l'ensemble des drivers disponibles
./run_all_tests.sh c07            # Teste uniquement le module C07
./run_all_tests.sh bsq rush01     # Teste BSQ et Rush 01
```

Chaque driver affiche `PASS` / `FAIL` et `run_all_tests.sh` retourne `0` uniquement si 100 % des suites exécutées sont au vert.
