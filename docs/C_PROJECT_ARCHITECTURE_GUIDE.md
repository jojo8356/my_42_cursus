# Guide d'Architecture Professionnelle d'un Projet C / Professional C Project Architecture Guide

<p align="center">
  <strong>Standards industriels (Pitchfork Layout, GNU, Linux, Redis, cURL, SQLite) & adaptation École 42 (Norme v4.1)</strong><br>
  <em>Industrial standards (Pitchfork Layout, GNU, Linux, Redis, cURL, SQLite) & 42 School adaptation (Norm v4.1)</em>
</p>

<p align="center">
  <a href="#-version-française">🇫🇷 Version Française</a> · <a href="#-english-version">🇬🇧 English Version</a>
</p>

---

## 🇫🇷 Version Française

### Table des matières
1. [Pourquoi l'architecture d'un projet C est critique](#1-pourquoi-larchitecture-dun-projet-c-est-critique)
2. [Le standard industriel de référence : The Pitchfork Layout (PFL)](#2-le-standard-industriel-de-référence--the-pitchfork-layout-pfl)
3. [Comment s'organisent les géants du C (Redis, cURL, SQLite, Linux)](#3-comment-sorganisent-les-géants-du-c-redis-curl-sqlite-linux)
4. [Arborescences recommandées selon la taille du projet](#4-arborescences-recommandées-selon-la-taille-du-projet)
5. [Règles professionnelles sur les headers (`.h`) et `#include`](#5-règles-professionnelles-sur-les-headers-h-et-include)
6. [Makefiles professionnels : Standard Entreprise vs Standard École 42](#6-makefiles-professionnels--standard-entreprise-vs-standard-école-42)
7. [Matrice de compatibilité Moulinette 42](#7-matrice-de-compatibilité-moulinette-42)
8. [Outillage qualité, sécurité mémoire, Git & CI/CD](#8-outillage-qualité-sécurité-mémoire-git--cicd)

---

### 1. Pourquoi l'architecture d'un projet C est critique

Contrairement à Rust (`Cargo`), Go (`go mod`) ou Java (packages liés aux dossiers), le langage **C n'impose aucune structure de répertoires par défaut**. Un dépôt C mal rangé souffre rapidement de cinq problèmes majeurs :
- **Pollution de la racine** : mélange de `.c`, `.h`, `.o`, binaires ELF, scripts de test et PDF.
- **Fuite d'encapsulation** : absence de séparation entre l'**API publique** (exposée aux consommateurs) et les **détails d'implémentation privés**.
- **Recompilations erronées ou partielles** : absence de suivi des dépendances de headers (`-MMD -MP`), provoquant des *segmentation faults* fantômes quand une `struct` change dans un `.h`.
- **Artefacts compilés suivis dans Git** : fichiers `.o`, `.a` ou exécutables versionnés par erreur, causant des conflits binaires et des erreurs d'architecture CPU (x86_64 vs ARM64).
- **Tests couplés au livrable** : impossibilité de compiler la bibliothèque seule ou de passer les vérificateurs stricts (comme la Moulinette 42).

---

### 2. Le standard industriel de référence : The Pitchfork Layout (PFL)

Dans l'écosystème C/C++ moderne, la spécification communautaire de référence est **The Pitchfork Layout (PFL)** (conçue pour standardiser CMake/Make, adoptée dans l'embarqué, AUTOSAR, et l'open source industriel).

#### Règle d'or du Pitchfork Layout
> **Un répertoire racine ne contient que des fichiers de métadonnées (`README.md`, `LICENSE`, `Makefile` / `CMakeLists.txt`, `.gitignore`, `.clang-format`) et un ensemble restreint de sous-dossiers normalisés.**

| Dossier | Statut Git | Rôle exact |
| :--- | :---: | :--- |
| `include/` | ✅ Suivi | **Headers publics** uniquement (API exposée à l'extérieur). Souvent préfixés `include/<nom_projet>/` pour éviter toute collision de noms. |
| `src/` | ✅ Suivi | **Fichiers sources `.c`** et **headers privés** internes qui ne doivent pas être exposés aux utilisateurs externes. |
| `lib/` ou `external/` | ✅ Suivi | Dépendances internes ou bibliothèques tierces embarquées (ex. `libft/` à 42, `deps/` dans Redis). |
| `build/` ou `obj/` | ❌ Ignoré | **Tous** les artefacts générés par la compilation (`.o`, `.d`, stamps). Supprimable à 100 % via `rm -rf build`. |
| `bin/` *(optionnel)* | ❌ Ignoré | Exécutables finaux compilés (note : à 42, le binaire final reste à la racine). |
| `tests/` | ✅ Suivi | Tests unitaires, tests d'intégration, fuzzers, mocks et harnais différentiels. |
| `docs/` | ✅ Suivi | Documentation d'architecture, spécifications protocolaires, manpages, sujets PDF. |
| `scripts/` ou `tools/` | ✅ Suivi | Scripts d'automatisation, benchmarks, générateurs de cas de tests, hooks CI. |
| `examples/` *(optionnel)* | ✅ Suivi | Programmes minimaux montrant comment intégrer la bibliothèque. |

---

### 3. Comment s'organisent les géants du C (Redis, cURL, SQLite, Linux)

| Projet | Structure réelle observée | Pourquoi ce choix ? |
| :--- | :--- | :--- |
| **cURL** (`curl/curl`) | `include/curl/` (API publique `libcurl`), `lib/` (moteur C `libcurl`), `src/` (outil CLI `curl`), `tests/`, `docs/` | Séparation stricte entre la **bibliothèque réutilisable** (`lib/` + `include/curl/`) et l'**exécutable CLI** (`src/`). |
| **Redis** (`redis/redis`) | `src/` (serveur C monolithique + headers internes), `deps/` (`jemalloc`, `hiredis`, `lua`), `tests/`, `utils/` | Serveur autonome (pas une bibliothèque publique) : `deps/` isole 100 % des dépendances tierces pour compiler avec un simple `make`. |
| **SQLite** (`sqlite/sqlite`) | `src/`, `ext/` (extensions JSON/FTS), `test/`, `tool/`, `doc/` | `tool/` génère l'amalgamation célèbre `sqlite3.c` + `sqlite3.h` à partir de `src/`. |
| **Noyau Linux** | `include/linux/`, `arch/`, `kernel/`, `mm/`, `fs/`, `net/`, `drivers/`, `scripts/`, `Documentation/` | Découpage **modulaire par domaine fonctionnel** dès que le projet dépasse quelques dizaines de fichiers. |

---

### 4. Arborescences recommandées selon la taille du projet

#### A. Projet C de taille petite à moyenne (10 à 50 fichiers — ex. `ft_printf`, `minitalk`, `pipex`, `push_swap`)

```text
mon_projet/
├── include/                    # Headers (.h)
│   └── mon_projet.h
├── src/                        # Implémentations (.c)
│   ├── main.c
│   ├── parser.c
│   └── executor.c
├── obj/                        # Généré par make (ignoré par Git !)
├── docs/                       # Spécifications, schémas, sujets
├── tests/                      # Suite de tests isolée du livrable
│   ├── Makefile
│   └── test_runner.c
├── .gitignore
├── Makefile
└── README.md
```

> **Exception 42 — `get_next_line` (v14.3)** : le sujet impose des noms de fichiers exacts *à la racine* (`get_next_line.{c,h}`, `get_next_line_utils.c` et leurs variantes `_bonus.{c,h}`) et **aucun `Makefile`**. On conserve donc uniquement ces 6 livrables à la racine, et l'on déporte sujet (`docs/`), tests internes (`tests/`), testeurs communautaires (`gnlTester/`) et l'orchestrateur (`run_tests.sh`) — sans jamais renommer ni déplacer les fichiers notés par la Moulinette.

#### B. Grand projet C modulaire (50 à 500+ fichiers — ex. `minishell`, `cub3d`, moteur réseau/système)

Évitez d'avoir 80 fichiers `.c` à plat dans `src/` : regroupez-les par **sous-système fonctionnel** (et reproduisez l'arborescence dans `obj/` avec `@mkdir -p $(dir $@)`).

```text
minishell/
├── include/
│   ├── minishell.h             # Point d'entrée des déclarations partagées
│   ├── lexer.h
│   ├── ast.h
│   └── executor.h
├── lib/
│   └── libft/                  # Sous-bibliothèque compilée via make -C lib/libft
├── src/
│   ├── main.c
│   ├── lexer/                  # Module 1 : analyse lexicale
│   │   ├── tokenize.c
│   │   └── quotes.c
│   ├── parser/                 # Module 2 : construction de l'AST
│   │   ├── parse_pipeline.c
│   │   └── parse_redir.c
│   ├── executor/               # Module 3 : processus, pipes, signaux
│   │   ├── exec_cmd.c
│   │   └── heredoc.c
│   └── builtins/               # Module 4 : cd, echo, env, export, unset, exit
│       ├── builtin_cd.c
│       └── builtin_export.c
├── obj/                        # Miroir automatique : obj/lexer/tokenize.o, etc.
├── tests/
├── docs/
├── .clang-format
├── .gitignore
├── Makefile
└── README.md
```

---

### 5. Règles professionnelles sur les headers (`.h`) et `#include`

1. **Autonomie des headers (*Self-contained headers* — Google C++ / C Style Guide)** :
   Chaque fichier `.h` doit compiler seul s'il est inclus en première ligne d'un `.c`. S'il utilise `size_t`, il doit inclure `<stddef.h>` ou `<stdlib.h>` lui-même.
2. **Ordre d'inclusion standardisé (du spécifique au général, ou standardisé Norme)** :
   En industrie (Google Style Guide), on inclut **d'abord** le header propre au module (`#include "lexer.h"`), puis les headers système C (`<unistd.h>`, `<stdlib.h>`), puis les bibliothèques externes (`"libft.h"`). Cela prouve immédiatement que `"lexer.h"` n'oublie aucune dépendance cachée !
3. **Include Guards systématiques** :
   ```c
   #ifndef MON_PROJET_LEXER_H
   # define MON_PROJET_LEXER_H
   /* ... déclarations uniquement ... */
   #endif
   ```
   *(Note : `#pragma once` est courant en entreprise, mais **interdit par la Norminette 42** — privilégiez toujours `#ifndef` / `# define` / `#endif`, universel et portable ISO C).*
4. **Visibilité minimale (`static`)** :
   Toute fonction qui n'est appelée **que** dans son propre fichier `.c` **doit** être déclarée `static` et **ne doit pas** figurer dans le `.h`. Cela évite la pollution de la table des symboles ELF (`nm -g`).
5. **Jamais de `#include "fichier.c"`** :
   On compile des unités de traduction séparées (`.c` → `.o`), puis l'éditeur de liens (`ld` / `ar`) les assemble.

---

### 6. Makefiles professionnels : Standard Entreprise vs Standard École 42

Attention à une différence fondamentale entre l'industrie et l'École 42 :
- **En entreprise**, on utilise `-MMD -MP` (génération automatique des dépendances de headers `.d`) et parfois des profils `BUILD=debug|release|asan`.
- **À l'École 42**, les jokers `$(wildcard *.c)` et `$(shell find ...)` sont **strictement interdits** (la Norme exige de lister explicitement chaque fichier `.c`), les règles `$(NAME)`, `all`, `clean`, `fclean`, `re` sont obligatoires, et **aucun relink** ne doit se produire au second appel de `make`.

#### Modèle 1 — Makefile Industriel Complet (C11, dépendances `.d` auto, profils Debug/Release/ASan)

```makefile
NAME        := bin/app
CC          := cc
STD         := -std=c11
WARN        := -Wall -Wextra -Werror -Wpedantic -Wshadow -Wconversion
CPPFLAGS    := -Iinclude

BUILD       ?= release
ifeq ($(BUILD),debug)
    CFLAGS  := $(STD) $(WARN) -g3 -O0
else ifeq ($(BUILD),asan)
    CFLAGS  := $(STD) $(WARN) -g3 -O1 -fsanitize=address,undefined -fno-omit-frame-pointer
    LDFLAGS := -fsanitize=address,undefined
else
    CFLAGS  := $(STD) $(WARN) -O2 -DNDEBUG
endif

SRC_DIR     := src
OBJ_DIR     := build/$(BUILD)/obj

SRCS        := src/main.c src/core/engine.c
OBJS        := $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
DEPS        := $(OBJS:.o=.d)

all: $(NAME)

$(NAME): $(OBJS)
	@mkdir -p $(dir $@)
	$(CC) $(LDFLAGS) $(OBJS) -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(CPPFLAGS) -MMD -MP -c $< -o $@

-include $(DEPS)

clean:
	rm -rf build

fclean: clean
	rm -rf bin

re: fclean all

.PHONY: all clean fclean re
```

#### Modèle 2 — Makefile Professionnel 100 % Compatible École 42 (avec sous-dossiers & Libft)

```makefile
NAME        = push_swap

CC          = cc
CFLAGS      = -Wall -Wextra -Werror
CPPFLAGS    = -Iinclude -I$(LIBFT_DIR)

LIBFT_DIR   = libft
LIBFT_A     = $(LIBFT_DIR)/libft.a

SRC_DIR     = src
OBJ_DIR     = obj

SRCS        = main.c \
              parse/parse_args.c \
              sort/turk_sort.c

OBJS        = $(addprefix $(OBJ_DIR)/, $(SRCS:.c=.o))
DEPS        = $(OBJS:.o=.d)

all: $(NAME)

$(LIBFT_A):
	$(MAKE) -C $(LIBFT_DIR)

$(NAME): $(LIBFT_A) $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -L$(LIBFT_DIR) -lft -o $(NAME)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(CPPFLAGS) -MMD -MP -c $< -o $@

-include $(DEPS)

clean:
	$(MAKE) -C $(LIBFT_DIR) clean
	rm -rf $(OBJ_DIR)

fclean: clean
	$(MAKE) -C $(LIBFT_DIR) fclean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
```

---

### 7. Matrice de compatibilité Moulinette 42

Dans ce dépôt `my_42_cursus`, chaque projet applique le rangement le plus propre autorisé par son énoncé officiel :

| Projet | Exigence Moulinette / Sujet 42 | Organisation adoptée dans ce dépôt |
| :--- | :--- | :--- |
| **`piscine42`** | Fichiers rendus dans `ex00/`, `ex01/`, etc. | Livrables purs dans `cXX/exYY/`, tests isolés dans `tests/`, sujets dans `subjects/pdf/`. |
| **`libft`** *(v19.3)* | *« Placez tous vos fichiers à la racine de votre dépôt »* (`Makefile`, `libft.h`, `ft_*.c`, `README.md`). | `Makefile`, `libft.h`, `README.md` et les 43 `ft_*.c` à la racine de `libft/` ; sujet dans `docs/` ; tests dans `tests/`. |
| **`get_next_line`** *(v14.3)* | 6 fichiers imposés à la racine : `get_next_line.{c,h}`, `get_next_line_utils.c` + `_bonus.{c,h}` ; **aucun `Makefile`** n'est requis ni attendu. | Les 6 fichiers de rendu restent seuls à la racine de `get_next_line/` ; sujet dans `docs/`, tests communautaires et internes dans `tests/`, `gnlTester/` et un `run_tests.sh` d'orchestration. |
| **`ft_printf`** | `Makefile, *.h, */*.h, *.c, */*.c` ; archive `libftprintf.a` générée à la racine. | Architecture Pitchfork adaptée 42 : `include/`, `src/`, `obj/` *(gitignored)*, `docs/`, `tests/`, `libftprintf.a` à la racine. |
| **`minitalk`** | `Makefile` à la racine ; binaires `client` et `server` à la racine ; bonus suffixés `_bonus.{c,h}`. | Architecture Pitchfork adaptée 42 : `include/`, `src/`, `obj/` *(gitignored)*, `docs/`, `tests/`, exécutables à la racine. |
| **Projets futurs** *(push_swap, pipex, minishell…)* | `Makefile` et binaire à la racine ; `libft/` à la racine si utilisée. | `include/`, `src/<modules>/`, `obj/` *(gitignored)*, `libft/`, `docs/`, `tests/`. |

> **Rappel avant un `git push` vers Vogsphere (dépôt de notation 42)** : ne poussez jamais les dossiers `tests/`, `printfTester/` ou les PDF lourds de `docs/` sur Vogsphere si le sujet interdit les fichiers supplémentaires non évalués — gardez-les dans votre portfolio GitHub ou ignorez-les lors de l'export vers Vogsphere.

---

### 8. Outillage qualité, sécurité mémoire, Git & CI/CD

Les plus grandes entreprises d'ingénierie système appliquent **5 garde-fous automatiques** sur tout code C :
1. **Zéro artefact binaire dans Git (`.gitignore`)** : aucun `.o`, `.a`, `.d`, `.so` ni exécutable ne doit jamais entrer dans `git status`.
2. **Analyse statique & formatage** :
   - À 42 : `norminette` (v4.1 : max 25 lignes/fonction, max 5 fonctions/fichier, max 80 colonnes).
   - En entreprise : `.clang-format` (style automatique) + `.clang-tidy` / `cppcheck` + flags `-Wall -Wextra -Werror -Wshadow -Wconversion`.
3. **Sanitizers à la compilation de test** :
   Compiler les tests avec `-g3 -fsanitize=address,undefined -fno-omit-frame-pointer` détecte instantanément les *buffer overflows*, *use-after-free*, *double-free*, *memory leaks* (LSan) et *signed integer overflows* (UBSan).
4. **Audit dynamique sous Valgrind** :
   `valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --error-exitcode=99 ./mon_programme` vérifie l'absence de lecture mémoire non initialisée et la fermeture des descripteurs (`--track-fds=yes`).
5. **Indexation IDE (`compile_commands.json`)** :
   Exécuter `bear -- make re` génère `compile_commands.json` pour donner à `clangd` (VS Code, Neovim, CLion) une autocomplétion et une navigation C instantanées avec `-Iinclude`.

---
---

## 🇬🇧 English Version

### Table of Contents
1. [Why C Project Structure Matters](#1-why-c-project-structure-matters)
2. [The Industry Standard: The Pitchfork Layout (PFL)](#2-the-industry-standard-the-pitchfork-layout-pfl)
3. [How Iconic C Codebases Are Organized](#3-how-iconic-c-codebases-are-organized)
4. [Recommended Directory Blueprints](#4-recommended-directory-blueprints)
5. [Header (`.h`) & `#include` Engineering Rules](#5-header-h--include-engineering-rules)
6. [42 School Compatibility Summary](#6-42-school-compatibility-summary)

---

### 1. Why C Project Structure Matters

Unlike Rust (`Cargo`) or Go (`go mod`), ISO C enforces no canonical directory tree. Professional C repositories rely on strict conventions to achieve:
- **Public vs. Private API Encapsulation** (`include/` vs `src/`).
- **Out-of-source / Isolated Build Artifacts** (`obj/` or `build/`, completely ignored by Git).
- **Deterministic Incremental Builds** (header dependency graphs via `-MMD -MP`, zero unnecessary relinking).
- **Isolated Verification Harnesses** (`tests/` decoupled from production deliverables).

### 2. The Industry Standard: The Pitchfork Layout (PFL)

**The Pitchfork Layout (PFL)** is the widely adopted C/C++ community specification:
- `include/` — Public headers exposed to consumers (ideally namespaced as `include/<project>/`).
- `src/` — Implementation `.c` translation units and private internal `.h` headers.
- `lib/` or `external/` — Bundled sub-libraries or third-party dependencies (e.g., `libft/`).
- `build/` or `obj/` — Ephemeral build outputs (`.o`, `.d`), **never** committed to version control.
- `tests/` — Unit, differential, sanitizer, and fault-injection test suites.
- `docs/` — Architecture notes, RFCs/specs, man pages, and subject PDFs.
- `scripts/` or `tools/` — Developer automation, test generators, and CI helpers.

### 3. How Iconic C Codebases Are Organized

- **cURL**: `include/curl/` (public API), `lib/` (`libcurl` engine), `src/` (`curl` CLI), `tests/`, `docs/`.
- **Redis**: `src/` (core server), `deps/` (vendored `jemalloc`, `lua`, `hiredis`), `tests/`, `utils/`.
- **SQLite**: `src/` (core modules), `ext/` (extensions), `test/`, `tool/` (amalgamation builder), `doc/`.
- **Linux Kernel**: Domain-driven subsystems (`kernel/`, `mm/`, `fs/`, `net/`, `drivers/`, `include/linux/`).

### 4. Recommended Directory Blueprints

- **Small-to-Medium C Libraries & CLIs (10–50 files)**: `include/`, `src/`, `obj/` *(gitignored)*, `tests/`, `docs/`, `Makefile`, `README.md`, `.gitignore`.
- **Large Multi-Subsystem Projects (50–500+ files)**: Group `src/` into cohesive domain subdirectories (`src/lexer/`, `src/parser/`, `src/executor/`) mirrored automatically under `obj/` via `@mkdir -p $(dir $@)`.
- **42 Exception — `get_next_line` (v14.3)**: The subject mandates exact filenames at the project root (`get_next_line.{c,h}`, `get_next_line_utils.c` and their `_bonus.{c,h}` variants) and **no `Makefile`**. Keep the root limited to those six graded deliverables and move the subject, tests, community testers, and `run_tests.sh` into `docs/`, `tests/`, and `gnlTester/`.

### 5. Header (`.h`) & `#include` Engineering Rules

1. **Self-Contained Headers**: Every `.h` file must include all headers required for its own declarations and pass double-inclusion checks via `#ifndef PROJECT_MODULE_H` guards.
2. **Symbol Visibility Hygiene**: Declare every file-local helper function `static` so `nm -g` exposes exclusively the intended public API.
3. **Never `#include` `.c` files**: Compile translation units independently and link object files via `cc` or `ar rcs`.

### 6. 42 School Compatibility Summary

- **`libft` (v19.3)**: Keep `Makefile`, `libft.h`, `README.md`, and `ft_*.c` at the root of `libft/` as strictly mandated by Chapter IV.1 (*"Place all your files at the root of your repository"*).
- **`get_next_line` (v14.3)**: The subject mandates six exact filenames at the root of the project (`get_next_line.c`, `get_next_line_utils.c`, `get_next_line.h`, plus `_bonus` variants) and **no `Makefile`**. Keep the root strictly limited to those deliverables, and place the subject, testers, and the `run_tests.sh` orchestrator in `docs/`, `tests/`, and `gnlTester/`.
- **`ft_printf`, `minitalk`, and Rank 2+ projects**: Use `include/`, `src/`, and `obj/` *(gitignored)* while emitting the final archive (`libftprintf.a`) or binaries (`client`, `server`) at the project root as required by the Moulinette.
