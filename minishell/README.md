_*A minimal shell implementation in C, following the 42 School subject requirements.*_

# Minishell

## Description

Minishell is a custom shell implementation written in C, created as part of the 42 School curriculum. It supports all mandatory features including command execution, pipes, redirections, variable expansion, quotes, builtins, and bonus features like logical operators (`&&`, `||`), parentheses, and wildcards.

## Features

### Mandatory
- **Prompt**: Interactive prompt `minishell> `
- **History**: Command history with arrow key navigation (via readline)
- **PATH search**: External commands are searched in PATH
- **Quotes**: Single quotes (`'`) block all interpretation; double quotes (`"`) block all except `$`
- **Redirections**: Input `<`, Output `>`, Append `>>`
- **Pipes**: `|` for connecting commands
- **Variable expansion**: `$VAR` and `$?` (previous exit code)
- **Signals**: Ctrl-C (newline + prompt), Ctrl-D (quit on empty prompt), Ctrl-\ (ignored)
- **Builtins**: `echo -n`, `cd` (relative/absolute), `pwd`, `export`, `unset`, `env`, `exit`

### Bonus
- **Logical operators**: `&&` and `||` with proper short-circuit evaluation
- **Parentheses**: `(...)` for grouping commands
- **Wildcards**: `*` expands to files in current directory
- **Multiple commands**: Each command executed separately

## Architecture

```
minishell/
├── includes/
│   ├── minishell.h          # Main header with types and function declarations
│   └── readline_mock.h      # Readline mock for sandbox compilation
├── src/
│   ├── 0_main.c             # Main loop and shell entry point
│   ├── 1_tokenizer*.c       # Input tokenization
│   ├── 2_parser*.c          # Token → command structure parsing
│   ├── 3_expander*.c        # Variable, quote, and wildcard expansion
│   ├── 4_executor*.c        # Command execution with pipes and fork
│   ├── 5_builtins*.c        # Builtin command implementations
│   ├── 6_redirections.c     # File descriptor redirection
│   ├── 7_signals.c          # Signal handling
│   ├── 8_cleanup.c          # Memory cleanup
│   ├── shell_state.c        # Shell exit state management
│   └── readline_mock.c      # Mock readline implementation
├── libft/                   # Vendored libft library
├── bonus/                   # Bonus feature files
├── tests/                   # Test suite
│   ├── test_framework.h     # Test framework
│   ├── test_basic.c         # Basic command tests
│   ├── test_variables.c     # Variable expansion tests
│   ├── test_quotes.c        # Quote handling tests
│   ├── test_pipes.c         # Pipe execution tests
│   ├── test_redirects.c     # Redirection tests
│   ├── test_logical.c       # && || tests
│   ├── test_wildcards.c     # Wildcard tests
│   └── run_tests.sh         # Test runner script
└── Makefile
```

## Build

```bash
make            # Build minishell
make clean      # Remove object files
make fclean     # Remove object files and binary
make re         # Clean and rebuild
```

## Run

```bash
./minishell
```

Or with piped input:

```bash
echo "echo hello" | ./minishell
```

## Test

```bash
./tests/run_tests.sh
```

The test suite covers all mandatory features and bonus features with 49+ tests.

## Examples

```bash
# Basic commands
minishell> echo hello world
hello world

# Variables
minishell> export NAME=42
minishell> echo $NAME
42

# Pipes
minishell> echo hello | tr a-z A-Z
HELLO

# Redirections
minishell> echo hello > output.txt
minishell> cat < output.txt
hello

# Logical operators
minishell> echo a && echo b
a
b
minishell> false || echo "not found"
not found

# Wildcards
minishell> echo *.txt
file1.txt file2.txt
```

## Technical Notes

- **Single global**: `volatile sig_atomic_t g_signal` for signal number only
- **No system()**: All system calls use allowed functions only
- **Memory**: Proper cleanup with `free_pipeline`, `free_tokens`
- **Norm**: Follows 42 School Norm (≤ 4 parameters per function, no globals except signal)

---

## FR

_*Une implémentation minimale d'un shell en C, suivant les exigences du sujet de 42 School.*_

# Minishell

## Description

Minishell est une implémentation personnalisée d'un shell écrite en C, créée dans le cadre du cursus de 42 School. Il prend en charge toutes les fonctionnalités obligatoires : exécution de commandes, pipes, redirections, expansion de variables, quotes, builtins, et les bonus comme les opérateurs logiques (`&&`, `||`), les parenthèses et les wildcards.

## Fonctionnalités

### Obligatoires
- **Prompt** : Prompt interactif `minishell> `
- **History** : Historique de commandes avec navigation par touches flèches (via readline)
- **Recherche PATH** : Les commandes externes sont recherchées dans PATH
- **Quotes** : Les quotes simples (`'`) bloquent toute interprétation ; les doubles quotes (`"`) bloquent tout sauf `$`
- **Redirections** : Entrée `<`, Sortie `>`, Append `>>`
- **Pipes** : `|` pour connecter des commandes
- **Expansion de variables** : `$VAR` et `$?` (code de sortie précédent)
- **Signaux** : Ctrl-C (nouvelle ligne + prompt), Ctrl-D (quit si prompt vide), Ctrl-\ (ignoré)
- **Builtins** : `echo -n`, `cd` (relative/absolue), `pwd`, `export`, `unset`, `env`, `exit`

### Bonus
- **Opérateurs logiques** : `&&` et `||` avec évaluation court-circuit
- **Parenthèses** : `(...)` pour grouper des commandes
- **Wildcards** : `*` s'expand en fichiers du CWD
- **Commandes multiples** : Chaque commande exécutée séparément

## Build

```bash
make            # Compile minishell
make clean      # Supprime les .o
make fclean     # Supprime les .o et le binaire
make re         # Clean et rebuild
```

## Tests

```bash
./tests/run_tests.sh
```

La suite de tests couvre toutes les fonctionnalités obligatoires et bonus avec 49+ tests.

## Notes techniques

- **Global unique** : `volatile sig_atomic_t g_signal` pour le numéro de signal uniquement
- **Pas de system()** : Tous les appels système utilisent les fonctions autorisées
- **Memory** : Nettoyage approprié avec `free_pipeline`, `free_tokens`
- **Norm** : Suit la Norm de 42 (≤ 4 paramètres/fonction, pas de globals sauf signal)
