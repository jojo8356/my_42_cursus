*This project has been created as part of the 42 curriculum by jojo8356.*

<div align="center">

# Libft — Standalone C Standard & Data-Structure Library

**Zero-dependency static C library (`libft.a`) implementing 43 essential memory, string, I/O, and singly linked-list routines.**

[![Language: C](https://img.shields.io/badge/Language-C_(ISO_C89)-00599C?style=flat-square&logo=c&logoColor=white)](https://en.cppreference.com/w/c)
[![Subject](https://img.shields.io/badge/42_Subject-2026_v19.3-00BABC?style=flat-square)](docs/SUJET_LIBFT_2026_V19.3.md)
[![42 Norm](https://img.shields.io/badge/Norminette-v4.1_Passing-brightgreen?style=flat-square)](https://github.com/42School/norminette)
[![Unit Assertions](https://img.shields.io/badge/Assertions-11%2C407_%2F_11%2C407_OK-success?style=flat-square)](#verification--test-suite)
[![Memory Safety](https://img.shields.io/badge/Sanitizers-ASan_%C2%B7_UBSan_%C2%B7_MallocWrap-blue?style=flat-square)](#verification--test-suite)

**[ 🇬🇧 English ](#-english-version) · [ 🇫🇷 Version Française ](#-version-française) · [ 📄 Subject v19.3 (FR) ](docs/SUJET_LIBFT_2026_V19.3.md)**

</div>

---

## 🇬🇧 English Version

### Table of Contents

- [Description](#description)
  - [What Changed in 42 Subject v19.3 (2026)](#what-changed-in-42-subject-v193-2026)
  - [Architecture & Project Structure](#architecture--project-structure)
  - [API Reference (43 Functions)](#api-reference-43-functions)
- [Instructions](#instructions)
  - [Prerequisites](#prerequisites)
  - [Building the Library](#building-the-library)
  - [Integrating `libft.a` Into Your C Project](#integrating-libfta-into-your-c-project)
  - [Verification & Test Suite](#verification--test-suite)
- [Resources](#resources)

---

## Description

**Libft** is the foundational Rank 00 project of the **École 42 Common Core** curriculum (**Subject v19.3**, **Norm v4.1**). Because standard library routines are restricted across subsequent 42 C projects unless personally re-implemented, `libft` provides a rock-solid, self-contained static archive (**`libft.a`**) built from scratch using exclusively `malloc(3)`, `free(3)`, and `write(2)` where authorized.

Key engineering guarantees of this implementation:
- **Zero Global State & Clean Symbol Table**: Audited via `nm` across all 43 object files — zero global or file-scope static variables, zero forbidden external calls, and `static` linkage on every internal helper.
- **Defensive Memory Safety**: Arithmetic overflow protection (`nmemb > SIZE_MAX / size` in `ft_calloc`), full rollback deallocation on mid-sequence `malloc` failures (`ft_split`, `ft_lstmap`), overlap-safe `ft_memmove`, and safe `INT_MIN` (`-2147483648`) handling without signed overflow.
- **Strict Compiler & Norm Discipline**: Compiles cleanly with `cc -Wall -Wextra -Werror` without C99 `restrict` qualifiers and with zero unnecessary relinking.

### What Changed in 42 Subject v19.3 (2026)

1. **All 43 Functions Are Mandatory**: Linked-list utilities (`ft_lst*`, formerly Bonus) now form **Mandatory Part 3** and are compiled directly by `make` / `make all`.
2. **Exact `1` / `0` Predicates**: `ft_isalpha`, `ft_isdigit`, `ft_isalnum`, `ft_isascii`, and `ft_isprint` strictly return `1` (true) or `0` (false).
3. **`unsigned int` Return Type for `ft_lstsize`**: Updated signature `unsigned int ft_lstsize(t_list *lst);`.
4. **Zero-Size `ft_calloc` Contract**: `ft_calloc(0, n)` and `ft_calloc(n, 0)` return a unique heap pointer safely passable to `free(3)`.

### Architecture & Project Structure

```text
libft/
├── Makefile                        # Builds libft.a (all, clean, fclean, re — zero relink)
├── libft.h                         # Public header (include-guarded, 43 prototypes + t_list)
├── ft_*.c                          # 43 standalone translation units (1 public function/file)
├── README.md                       # Bilingual project documentation (EN / FR)
├── docs/
│   └── SUJET_LIBFT_2026_V19.3.md   # Detailed 2026 v19.3 specification reference
└── tests/                          # Isolated 4-phase verification suite (never submitted)
    ├── Makefile
    ├── run_all_tests.sh            # Master test pipeline
    ├── test_setup.sh               # Norm, prototype, Makefile, relink & nm symbol audit
    ├── test_part1.c                # Unit tests for 23 libc functions
    ├── test_part2.c                # Unit tests for 11 additional functions
    ├── test_part3.c                # Unit tests for 9 linked-list functions
    └── test_malloc_fail.c          # Linker --wrap=malloc deterministic fault injection
```

> **Why are `ft_*.c` and `libft.h` at the root of `libft/` instead of `src/` and `include/`?**  
> Chapter IV.1 of the official 42 Libft v19.3 subject explicitly mandates: *"Place all your files at the root of your repository."* Keeping the 43 `.c` files, `libft.h`, and `Makefile` at the root ensures 100% compatibility with 42's *Deepthought* (*Moulinette*) grader and seamless drop-in embedding (`-Ilibft -Llibft -lft`) in downstream 42 projects.

---

### API Reference (43 Functions)

#### Part 1 — Standard `libc` Functions (23)

| Function | Prototype | Allowed Syscalls | Behavior Summary |
| :--- | :--- | :---: | :--- |
| `ft_isalpha` | `int ft_isalpha(int c);` | None | Returns `1` if ASCII alphabetic (`A-Z`, `a-z`), `0` otherwise. |
| `ft_isdigit` | `int ft_isdigit(int c);` | None | Returns `1` if ASCII decimal digit (`0-9`), `0` otherwise. |
| `ft_isalnum` | `int ft_isalnum(int c);` | None | Returns `1` if ASCII alphanumeric, `0` otherwise. |
| `ft_isascii` | `int ft_isascii(int c);` | None | Returns `1` if 7-bit US-ASCII (`0..127`), `0` otherwise. |
| `ft_isprint` | `int ft_isprint(int c);` | None | Returns `1` if printable ASCII (`32..126`), `0` otherwise. |
| `ft_strlen` | `size_t ft_strlen(const char *s);` | None | Computes byte length of NUL-terminated string `s`. |
| `ft_memset` | `void *ft_memset(void *s, int c, size_t n);` | None | Fills `n` bytes of `s` with `(unsigned char)c`. |
| `ft_bzero` | `void ft_bzero(void *s, size_t n);` | None | Zeroes `n` bytes at `s`. |
| `ft_memcpy` | `void *ft_memcpy(void *dest, const void *src, size_t n);` | None | Copies `n` non-overlapping bytes from `src` to `dest`. |
| `ft_memmove` | `void *ft_memmove(void *dest, const void *src, size_t n);` | None | Overlap-safe `n`-byte copy (backward copy when `dest > src`). |
| `ft_strlcpy` | `size_t ft_strlcpy(char *dst, const char *src, size_t size);` | None | Size-bounded string copy; NUL-terminates if `size > 0`; returns `strlen(src)`. |
| `ft_strlcat` | `size_t ft_strlcat(char *dst, const char *src, size_t size);` | None | Size-bounded string append; returns `min(size, strlen(dst)) + strlen(src)`. |
| `ft_toupper` | `int ft_toupper(int c);` | None | Converts lowercase ASCII letter to uppercase. |
| `ft_tolower` | `int ft_tolower(int c);` | None | Converts uppercase ASCII letter to lowercase. |
| `ft_strchr` | `char *ft_strchr(const char *s, int c);` | None | Locates first occurrence of `(char)c` in `s` (including `'\0'`). |
| `ft_strrchr` | `char *ft_strrchr(const char *s, int c);` | None | Locates last occurrence of `(char)c` in `s` (including `'\0'`). |
| `ft_strncmp` | `int ft_strncmp(const char *s1, const char *s2, size_t n);` | None | Lexicographically compares up to `n` bytes as `unsigned char`. |
| `ft_memchr` | `void *ft_memchr(const void *s, int c, size_t n);` | None | Scans `n` bytes of `s` for `(unsigned char)c`. |
| `ft_memcmp` | `int ft_memcmp(const void *s1, const void *s2, size_t n);` | None | Compares `n` raw bytes as `unsigned char`. |
| `ft_strnstr` | `char *ft_strnstr(const char *big, const char *little, size_t len);` | None | Finds substring `little` within first `len` characters of `big`. |
| `ft_atoi` | `int ft_atoi(const char *nptr);` | None | Parses leading whitespace, optional sign, and base-10 digits to `int`. |
| `ft_calloc` | `void *ft_calloc(size_t nmemb, size_t size);` | `malloc` | Overflow-checked zero-initialized allocation; returns freeable pointer if `0`. |
| `ft_strdup` | `char *ft_strdup(const char *s);` | `malloc` | Allocates and duplicates NUL-terminated string `s`. |

#### Part 2 — Additional String & File-Descriptor Utilities (11)

| Function | Prototype | Allowed Syscalls | Behavior Summary |
| :--- | :--- | :---: | :--- |
| `ft_substr` | `char *ft_substr(char const *s, unsigned int start, size_t len);` | `malloc` | Extracts newly allocated substring from `start` of max length `len`. |
| `ft_strjoin` | `char *ft_strjoin(char const *s1, char const *s2);` | `malloc` | Allocates concatenation of `s1` and `s2`. |
| `ft_strtrim` | `char *ft_strtrim(char const *s1, char const *set);` | `malloc` | Strips leading and trailing characters present in `set`. |
| `ft_split` | `char **ft_split(char const *s, char c);` | `malloc`, `free` | Splits `s` on delimiter `c` into `NULL`-terminated array (leak-proof on OOM). |
| `ft_itoa` | `char *ft_itoa(int n);` | `malloc` | Converts signed `int` (including `INT_MIN`) to allocated decimal string. |
| `ft_strmapi` | `char *ft_strmapi(char const *s, char (*f)(unsigned int, char));` | `malloc` | Maps indexed function `f` over `s` into a new string. |
| `ft_striteri` | `void ft_striteri(char *s, void (*f)(unsigned int, char*));` | None | Applies indexed mutator `f` in place to each character of `s`. |
| `ft_putchar_fd` | `void ft_putchar_fd(char c, int fd);` | `write` | Writes character `c` to file descriptor `fd`. |
| `ft_putstr_fd` | `void ft_putstr_fd(char *s, int fd);` | `write` | Writes string `s` to file descriptor `fd`. |
| `ft_putendl_fd` | `void ft_putendl_fd(char *s, int fd);` | `write` | Writes string `s` followed by `'\n'` to `fd`. |
| `ft_putnbr_fd` | `void ft_putnbr_fd(int n, int fd);` | `write` | Writes signed integer `n` (`INT_MIN..INT_MAX`) to `fd`. |

#### Part 3 — Singly Linked Lists `t_list` (9 — Mandatory in v19.3)

```c
typedef struct s_list
{
	void			*content;
	struct s_list	*next;
}	t_list;
```

| Function | Prototype | Allowed Syscalls | Behavior Summary |
| :--- | :--- | :---: | :--- |
| `ft_lstnew` | `t_list *ft_lstnew(void *content);` | `malloc` | Allocates node with `content` and `next = NULL`. |
| `ft_lstadd_front` | `void ft_lstadd_front(t_list **lst, t_list *new);` | None | Prepends `new` at head of `*lst` in $O(1)$. |
| `ft_lstsize` | `unsigned int ft_lstsize(t_list *lst);` | None | Counts nodes in `lst` (`unsigned int` per v19.3) in $O(n)$. |
| `ft_lstlast` | `t_list *ft_lstlast(t_list *lst);` | None | Returns tail node of `lst` (or `NULL`) in $O(n)$. |
| `ft_lstadd_back` | `void ft_lstadd_back(t_list **lst, t_list *new);` | None | Appends `new` to tail of `*lst` in $O(n)$. |
| `ft_lstdelone` | `void ft_lstdelone(t_list *lst, void (*del)(void *));` | `free` | Calls `del(lst->content)` and frees `lst` without freeing `next`. |
| `ft_lstclear` | `void ft_lstclear(t_list **lst, void (*del)(void *));` | `free` | Deletes entire chain from `*lst` onward and sets `*lst = NULL`. |
| `ft_lstiter` | `void ft_lstiter(t_list *lst, void (*f)(void *));` | None | Applies `f` to every node's `content`. |
| `ft_lstmap` | `t_list *ft_lstmap(t_list *lst, void *(*f)(void *), void (*del)(void *));` | `malloc`, `free` | Maps `f` into a new list; cleans up both mapped payload & partial list on OOM. |

---

## Instructions

### Prerequisites

- **OS**: Linux or macOS (POSIX)
- **Tools**: `cc` (`gcc` or `clang`), GNU `make`, `ar`

### Building the Library

```bash
cd libft
make          # Compiles all 43 .o files and archives ./libft.a
make clean    # Removes intermediate .o object files
make fclean   # Removes .o object files and ./libft.a
make re       # Full clean rebuild (fclean + all)
```

### Integrating `libft.a` Into Your C Project

```c
#include <stdio.h>
#include "libft.h"

int main(void)
{
    char    **words;
    size_t  i;

    words = ft_split("42::Systems::Engineering::in::C", ':');
    if (!words)
        return (1);
    i = 0;
    while (words[i])
    {
        ft_putendl_fd(words[i], 1);
        free(words[i++]);
    }
    free(words);
    return (0);
}
```

Compile and link against `libft.a`:

```bash
cc -Wall -Wextra -Werror main.c -I./libft -L./libft -lft -o app
./app
```

### Verification & Test Suite

Run the 4-phase verification suite from `libft/tests/`:

```bash
make -C tests           # Runs all 4 phases end-to-end
make -C tests fclean    # Cleans test binaries
```

| Phase | Target | Verification Scope | Status |
| :---: | :--- | :--- | :---: |
| **1** | `make -C tests setup` | File inventory (43 `.c`), prototypes, double-include, no-relink, `nm` symbol audit, README v19.3 check | ✅ **17 / 17 OK** |
| **2** | `make -C tests unit` | Standard build edge-case unit tests across all 43 functions | ✅ **11,407 / 11,407 OK** |
| **3** | `make -C tests asan` | Full recompilation & execution under `ASan + UBSan + LeakSanitizer` | ✅ **11,407 / 11,407 OK** |
| **4** | `make -C tests alloc_fail` | Deterministic `--wrap=malloc` fault injection across all 10 allocating functions | ✅ **10 / 10 Leak-Free** |

---

## Resources

### Classic References
- **Linux Programmer's Manual (`man 3`)**: `strlen`, `memset`, `memcpy`, `memmove`, `strlcpy`, `strlcat`, `calloc`, `strdup`, `atoi` — [man7.org](https://man7.org/linux/man-pages/)
- **ISO/IEC 9899 C Reference**: [cppreference.com/w/c](https://en.cppreference.com/w/c)
- **GNU Make Manual**: [gnu.org/software/make/manual](https://www.gnu.org/software/make/manual/)
- **42 School Norm v4.1 & Norminette**: [github.com/42School/norminette](https://github.com/42School/norminette)
- **Official 42 Libft Subject (2026 Version 19.3)**: archived in [`docs/SUJET_LIBFT_2026_V19.3.md`](docs/SUJET_LIBFT_2026_V19.3.md)

### AI Usage
In accordance with Chapter III (*AI Instructions*) and Chapter V of the 42 Libft v19.3 subject:
- **Specification Verification**: Auditing exact behavioral requirements introduced in Libft v19.2 and v19.3 (`1`/`0` return values for `ft_is*`, `unsigned int` return type for `ft_lstsize`, zero-size `ft_calloc` contract, and mandatory inclusion of `t_list` routines).
- **Test Engineering**: Designing non-submitted edge-case assertions, sanitizer targets, and `--wrap=malloc` deterministic OOM fault-injection harnesses in `tests/`.
- **Documentation**: Structuring and translating this bilingual enterprise-grade `README.md`.

---
---

## 🇫🇷 Version Française

### Présentation

**Libft** est le tout premier projet du **Tronc Commun de l'École 42** (**Sujet 2026 v19.3**, **Norme v4.1**). Son objectif est de concevoir de zéro une bibliothèque statique C autonome (**`libft.a`**) regroupant **43 fonctions essentielles** : réimplémentation de routines standards de la `libc`, allocateurs et transformateurs de chaînes, fonctions d'écriture sur descripteur de fichier (`fd`) et manipulation complète de listes simplement chaînées (`t_list`).

### Points forts d'ingénierie

- **Conformité stricte v19.3** : intégration des 9 fonctions de listes chaînées au `make` obligatoire, retour strict `1` ou `0` pour `ft_is*`, signature `unsigned int ft_lstsize(t_list *lst)` et pointeur unique libérable par `free()` pour `ft_calloc(0, n)`.
- **Table des symboles immaculée (`nm`)** : zéro variable globale, zéro variable `static` hors fonction, toutes les sous-fonctions internes déclarées `static`, zéro appel externe non autorisé.
- **Résilience aux pannes mémoire (`OOM`)** : protection contre le dépassement d'entier `nmemb * size > SIZE_MAX` dans `ft_calloc`, libération intégrale en cascade en cas d'échec d'allocation intermédiaire dans `ft_split` et `ft_lstmap`.
- **Pourquoi les fichiers `.c` et `.h` sont-ils à la racine de `libft/` ?** Le Chapitre IV.1 du sujet officiel v19.3 impose explicitement : *« Placez tous vos fichiers à la racine de votre dépôt. »* Cette disposition garantit une compatibilité totale avec la Moulinette (*Deepthought*) et l'inclusion directe dans les projets suivants du cursus.

### Compilation & Utilisation

```bash
cd libft
make            # Compile les 43 fichiers objets et génère libft.a
make clean      # Supprime les fichiers objets .o
make fclean     # Supprime les .o et l'archive libft.a
make re         # Recompilation complète (fclean + all)
```

Liaison avec un programme C :

```bash
cc -Wall -Wextra -Werror main.c -I./libft -L./libft -lft -o programme
```

### Suite de tests automatisée (4 phases)

```bash
make -C tests           # Exécute les 4 phases de validation (11 407 assertions)
make -C tests fclean    # Nettoie les binaires de test
```

1. **Phase 1 (`test_setup.sh`)** : vérification des 43 fichiers, double inclusion de `libft.h`, absence de `restrict`, absence de relink, audit binaire `nm` et conformité du `README.md` v19.3 (**17/17 OK**).
2. **Phase 2 (`unit`)** : batterie de tests unitaires et cas limites en compilation standard (**11 407/11 407 assertions OK**).
3. **Phase 3 (`asan`)** : exécution complète sous **AddressSanitizer + UBSan + LeakSanitizer** (**11 407/11 407 assertions OK**).
4. **Phase 4 (`alloc_fail`)** : injection déterministe d'échecs `malloc` via `--wrap=malloc` sur chacune des 10 fonctions d'allocation (**0 fuite mémoire**).
