*This project has been created as part of the 42 curriculum by jojo8356.*

<div align="center">

# ft_printf — Zero-Heap Variadic Formatted Output Engine

**Complete re-implementation of C's `printf(3)` (`libftprintf.a`) — Mandatory + Full Bonus (`-0.# +` & dynamic `*`), stack-only (`0 malloc`), validated across 26,921 differential test cases.**

[![Language: C](https://img.shields.io/badge/Language-C_(ISO_C89%2FC99)-00599C?style=flat-square&logo=c&logoColor=white)](https://en.cppreference.com/w/c/variadic)
[![42 Norm](https://img.shields.io/badge/Norminette-v4.1_Passing-00BABC?style=flat-square)](https://github.com/42School/norminette)
[![Heap Allocations](https://img.shields.io/badge/Heap_Allocations-0_(Stack_Only)-brightgreen?style=flat-square)](#architectural-design)
[![Differential Suite](https://img.shields.io/badge/Differential_Tests-26%2C921_%2F_26%2C921_OK-success?style=flat-square)](#verification--test-suites)
[![Sanitizers](https://img.shields.io/badge/Memory-ASan_%C2%B7_UBSan_%C2%B7_Valgrind-blue?style=flat-square)](#verification--test-suites)

**[ 🇬🇧 English ](#-english-version) · [ 🇫🇷 Version Française ](#-version-française) · [ 📄 Subject PDF ](docs/ft_printf.en.subject.pdf)**

</div>

---

## 🇬🇧 English Version

### Table of Contents

- [Description](#description)
  - [Supported Conversions & Bonus Flags](#supported-conversions--bonus-flags)
  - [Architectural Design](#architectural-design)
  - [Project Directory Layout](#project-directory-layout)
- [Instructions](#instructions)
  - [Prerequisites](#prerequisites)
  - [Building Mandatory & Bonus Archives](#building-mandatory--bonus-archives)
  - [Quick Start & Code Example](#quick-start--code-example)
  - [Verification & Test Suites](#verification--test-suites)
- [Resources](#resources)

---

## Description

**`ft_printf`** is a Rank 01 systems project in the **École 42 Common Core** curriculum. It re-implements the standard C library's variadic `printf(3)` function inside a standalone static archive (**`libftprintf.a`**) using `<stdarg.h>` (`va_start`, `va_arg`, `va_end`) and raw POSIX `write(2)` system calls.

```c
int ft_printf(const char *format, ...);
```

Unlike naive implementations that allocate temporary heap strings for formatting, padding, or base conversion, **`ft_printf` uses zero heap allocations (`0` calls to `malloc`/`free`)**. Every conversion is formatted in fixed-size stack buffers and streamed directly via a partial-write-safe `write(2)` loop, making memory leaks structurally impossible and propagating I/O errors (`-1`) deterministically.

### Supported Conversions & Bonus Flags

#### Mandatory Conversions (9 / 9)

| Specifier | Input Type | Output Representation | `glibc` Edge-Case Behavior |
| :---: | :--- | :--- | :--- |
| `%c` | `int` | Single byte character `(unsigned char)` | Writes NUL byte `\0` cleanly and increments return count by `1`. |
| `%s` | `const char *` | NUL-terminated character string | `NULL` prints `(null)` (or `""` under bonus precision `< 6`). |
| `%p` | `void *` | Pointer address in `0x` lowercase hex | `NULL` prints `(nil)` without `0x` prefix or numeric padding. |
| `%d` / `%i` | `int` | Signed base-10 integer | Handles `INT_MIN` (`-2147483648`) without signed overflow. |
| `%u` | `unsigned int` | Unsigned base-10 integer | Full `0 .. UINT_MAX` (`4294967295`) range. |
| `%x` / `%X` | `unsigned int` | Unsigned base-16 hex (lower / UPPER) | Value `0` with precision `.0` emits 0 digits. |
| `%%` | — | Literal `%` character | Emits `%` directly. |

#### Bonus Modifiers (All Implemented)

| Flag / Modifier | Syntax | Effect & Interaction Rules |
| :---: | :--- | :--- |
| **Left-justify** | `-` | Left-aligns formatted field within minimum width; overrides `0` flag. |
| **Zero-padding** | `0` | Pads numeric fields with leading `'0'` after sign/`0x`; ignored if `-` or explicit precision `.` is set. |
| **Precision** | `.N` / `.*` | Max bytes for `%s` (truncates `(null)` to `""` if `< 6`); min digits for integers/hex (`.0` on `0` prints nothing). |
| **Field width** | `N` / `*` | Minimum total characters output; negative dynamic `*` width sets `-` flag + `abs(width)`. |
| **Alternate form** | `#` | Prepends `0x` for `%x` and `0X` for `%X` when value is non-zero. |
| **Explicit sign** | `+` | Forces leading `+` on non-negative signed conversions (`%d`, `%i`, non-NULL `%p`); overrides space flag. |
| **Sign space** | `' '` | Prepends leading space `' '` on non-negative signed conversions when `+` is absent. |

---

### Architectural Design

```text
  format string ──▶ [ Scanner Loop ]
                         │
          ┌──────────────┴──────────────┐
          ▼ (literal run)               ▼ ('%' token)
   ft_write_mem()               ft_parse_spec()  ──▶  t_spec { flags, width, precision, conv }
   (partial-write loop)                 │
                                        ▼
                                ft_render_spec()
                                  ├── %c / %s / %% ──▶ ft_write_string()
                                  └── %d %i %u %x %X %p
                                        └──▶ ft_utoa() [stack char buf[32]]
                                        └──▶ ft_write_number() [left pad → sign/0x → zero pad → digits → right pad]
```

Key implementation highlights:
- **Zero Dynamic Memory**: `nm -u libftprintf.a` shows **`write` as the sole external symbol** — zero dependency on `malloc` or `free`.
- **Resilient `write(2)` Loop**: `ft_write_mem` loops until all requested bytes are written (handling short writes on pipes/sockets) and immediately bubbles up `-1` if `write` fails (tested against `/dev/full`).
- **Stamp-Based Zero-Relink Switching**: `Makefile` uses `.mandatory` and `.bonus` stamps inside `obj/` so `make` and `make bonus` switch cleanly into `libftprintf.a` without ever relinking on repeated invocations.

---

### Project Directory Layout

Organized following **The Pitchfork Layout** adapted to 42's `Makefile, *.h, */*.h, *.c, */*.c` submission rule:

```text
ft_printf/
├── include/
│   ├── ft_printf.h               # Public prototype + shared low-level output/base helpers
│   └── ft_printf_bonus.h         # Bonus t_spec / t_num structs & parser/renderer prototypes
├── src/
│   ├── ft_printf.c               # Mandatory entry point & conversion dispatcher
│   ├── ft_print_cs.c             # Mandatory %c, %s, %% handlers
│   ├── ft_print_num.c            # Mandatory %d, %i, %u, %x, %X handlers
│   ├── ft_print_ptr.c            # Mandatory %p handler
│   ├── ft_output.c               # Shared partial-write-safe I/O & padding primitives
│   ├── ft_num.c                  # Shared stack unsigned base converter (ft_utoa, ft_numlen)
│   ├── ft_printf_bonus.c         # Bonus entry point
│   ├── ft_parse_bonus.c          # Format flag, width, precision & dynamic '*' parser
│   ├── ft_render_bonus.c         # Bonus %c, %s, %% & dispatch logic
│   ├── ft_render_num_bonus.c     # Bonus %d, %i, %u, %x, %X, %p preparation
│   └── ft_write_number_bonus.c   # Bonus sign, 0x prefix, precision & alignment engine
├── obj/                          # Ephemeral build objects & stamps (gitignored)
├── docs/
│   └── ft_printf.en.subject.pdf  # Official 42 subject PDF
├── tests/                        # Differential test generator (~27k cases vs glibc)
├── printfTester/                 # Tripouille reference test suite
├── Makefile                      # Builds ./libftprintf.a (all, bonus, clean, fclean, re)
├── run_tests.sh                  # Automated end-to-end verification script
└── README.md                     # Bilingual project documentation (EN / FR)
```

---

## Instructions

### Prerequisites

- **OS**: Linux or macOS (POSIX)
- **Tools**: `cc` (`gcc` or `clang`), `make`, `ar` (`python3` for differential test generation)

### Building Mandatory & Bonus Archives

```bash
cd ft_printf
make            # Compiles mandatory sources into ./libftprintf.a
make bonus      # Compiles full bonus engine into ./libftprintf.a
make clean      # Removes ./obj/ directory (.o files & stamps)
make fclean     # Removes ./obj/ and ./libftprintf.a
make re         # Full clean rebuild (fclean + all)
```

### Quick Start & Code Example

```c
#include "ft_printf.h"

int main(void)
{
    int count;

    count = ft_printf("User: %-10s | Score: %+06d | Hex: %#08x | Ptr: %p\n",
                      "jojo8356", 125, 0x2a, (void *)0x42);
    ft_printf("Printed %d bytes.\n", count);
    return (0);
}
```

Compile and run:

```bash
make bonus
cc -Wall -Wextra -Werror main.c -Iinclude -L. -lftprintf -o demo
./demo
```

### Verification & Test Suites

```bash
# Run full automated pipeline (norm + build + 26,921 differential cases + ASan/UBSan)
./run_tests.sh

# Include Tripouille/printfTester (mandatory + bonus)
./run_tests.sh --full
```

| Check | Coverage | Result |
| :--- | :--- | :---: |
| **42 Norminette** | All 13 `.c` and `.h` files (`src/` & `include/`) | ✅ **0 Errors** |
| **Differential Suite** | **26,921 Cartesian flag × width × precision × boundary cases** vs `snprintf` + `/dev/full` | ✅ **26,921 / 26,921** |
| **ASan + UBSan** | Full differential suite under `-fsanitize=address,undefined` | ✅ **0 Issues** |
| **Valgrind** | Leak & uninitialized memory check (`--leak-check=full`) | ✅ **0 Leaks / 0 Errors** |
| **Tripouille `printfTester`** | Mandatory (`c s p d i u x X % mix`) + Bonus (`- 0 . # space +`) | ✅ **100% Passing** |
| **External Symbol Audit** | `nm -u libftprintf.a` | ✅ **`write` Only** |

---

## Resources

### Classic References
- **POSIX / Linux Manual Pages**: `man 3 printf`, `man 3 stdarg`, `man 2 write` — [man7.org](https://man7.org/linux/man-pages/man3/printf.3.html)
- **ISO C Variadic Arguments Reference**: [cppreference.com/w/c/variadic](https://en.cppreference.com/w/c/variadic)
- **Official 42 Subject**: [`docs/ft_printf.en.subject.pdf`](docs/ft_printf.en.subject.pdf)
- **Reference Tester**: [Tripouille/printfTester](https://github.com/Tripouille/printfTester)

### AI Usage
- **Differential Test Generation**: Designing the Python Cartesian test-case generator (`tests/generate_tests.py`) to cross-check 26,921 combinations of flags, dynamic widths/precisions, `NULL` pointers, and `INT_MIN` boundaries against `glibc` `snprintf`.
- **Documentation & Project Layout**: Structuring `include/` + `src/` + `obj/` Pitchfork-compliant layout and authoring this bilingual `README.md`.

---
---

## 🇫🇷 Version Française

### Présentation

**`ft_printf`** est un projet de Rang 01 du **Tronc Commun de l'École 42** consistant à réimplémenter la fonction variadique `printf(3)` de la `libc` sous la forme d'une bibliothèque statique **`libftprintf.a`**. Cette implémentation couvre **100 % de la partie obligatoire** (`c s p d i u x X %`) ainsi que **100 % des bonus** (flags `-`, `0`, `.`, `#`, `+`, `espace`, largeur minimale et paramètres dynamiques `*`).

### Points clés d'architecture

- **Zéro allocation dynamique (`0 malloc`)** : toutes les conversions numériques sont formées sur la pile et envoyées directement via `write(2)`. Aucune fuite mémoire n'est possible et le seul symbole externe de `libftprintf.a` est `write`.
- **Écriture robuste face aux écritures partielles** : `ft_write_mem` boucle tant que tous les octets n'ont pas été écrits et propage immédiatement `-1` en cas d'erreur système (validé sur `/dev/full`).
- **Alignement exact sur la `glibc`** : `%s` sur `NULL` affiche `(null)` (ou chaîne vide si précision `< 6`), `%p` sur `NULL` affiche `(nil)`, `INT_MIN` est converti sans dépassement signé, la précision annule le flag `0`, et `#` est omis pour `0`.
- **Arborescence Pitchfork compatible 42** : en-têtes dans `include/`, sources dans `src/`, objets et stamps dans `obj/` (ignoré par Git), sans aucun relink entre `make` et `make bonus`.

### Compilation & Utilisation

```bash
cd ft_printf
make            # Compile la version obligatoire -> ./libftprintf.a
make bonus      # Compile la version bonus complète -> ./libftprintf.a
make clean      # Supprime le dossier ./obj/
make fclean     # Supprime ./obj/ et ./libftprintf.a
make re         # Recompilation complète (fclean + all)
```

Compilation d'un programme utilisateur :

```bash
cc -Wall -Wextra -Werror main.c -Iinclude -L. -lftprintf -o demo && ./demo
```

### Lancer les tests

```bash
./run_tests.sh          # Norme + compilation + 26 921 tests différentiels + ASan/UBSan
./run_tests.sh --full   # Ajoute le testeur de référence Tripouille (mandatory + bonus)
```
