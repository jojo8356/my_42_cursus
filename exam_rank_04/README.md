# Exam Rank 04 — microshell

Projet **Exam Rank 04** de 42 School : **microshell**. Écrire un programme
qui se comporte comme l'exécution d'une commande de shell.

## Énoncé

L'énoncé complet est dans [`subjects/microshell_subject.en.txt`](subjects/microshell_subject.en.txt)
(et version française `microshell_subject.fr.txt`).

**Fonctions autorisées** (et uniquement celles-ci) :
`malloc, free, write, close, fork, waitpid, signal, kill, exit, chdir,
execve, dup, dup2, pipe, strcmp, strncmp`.

## Fonctionnalités (obligatoire)

- Exécution d'une commande dont le chemin est absolu ou relatif (pas de
  recherche dans `PATH`).
- Opérateur `|` (pipeline) et `;` (séquence), comme bash.
- Commande interne `cd` (un chemin en argument) :
  - mauvais nombre d'arguments → `error: cd: bad arguments` sur STDERR
  - échec → `error: cd: cannot change directory to <chemin>` sur STDERR
- Toute erreur de système d'appel (sauf `execve`/`chdir`) → `error: fatal`
  sur STDERR + sortie.
- Échec de `execve` → `error: cannot execute <chemin>` sur STDERR.
- Gestion de **centaines de `|`** sans épuiser les descripteurs (limite
  < 30 fichiers ouverts).
- Passage de `environ` à `execve` et **pas de fuite de descripteurs**.

## Bonus (Level 2)

- Opérateurs logiques `&&` et `||` avec **short-circuit** évalué
  gauche à droite (comme bash) :
  - `A && B` : `B` exécuté seulement si `A` réussit
  - `A || B` : `B` exécuté seulement si `A` échoue
  - chaînes mixtes (`A && B || C`) gérées correctement
- Gestion rigoureuse des descripteurs : **un seul pipe ouvert à la fois**
  dans le parent, donc des centaines de pipes avec très peu de fds ouverts.

> Remarque : les redirections de fichiers (`>`, `<`, `>>`) ne sont pas
> implémentées car `open` n'est **pas** dans la liste des fonctions
> autorisées par l'énoncé. Seuls `dup2` (autorisée) permettrait un
> redirection `2>&1`, mais les redirections de fichiers sont impossibles
> dans le cadrage du sujet.

## Structure

```
exam_rank_04/
├── Makefile               build + test
├── microshell.h           structures (t_cmd, t_group, t_sep, t_groups)
├── parser.c               découpage de la ligne de commande (| ; && ||)
├── microshell.c           exécution, pipelines, cd, erreurs, main
├── subjects/
│   ├── microshell_subject.en.txt
│   └── microshell_subject.fr.txt
└── tests/
    ├── test_framework.h   framework de tests
    ├── test_parser.c      tests unitaires du parseur
    └── run_tests.sh       tests d'intégration (binaire réel)
```

## Build

```bash
make          # construit ./microshell (-Wall -Wextra -Werror)
```

## Utilisation

```bash
./microshell /bin/ls "|" /usr/bin/grep microshell ";" /bin/echo i love my microshell
```

## Tests

```bash
make test
```

**33 tests, 100 % passants** :
- **11 tests unitaires** (`tests/test_parser.c`) : découpage `|`, `;`,
  `&&`, `||`, pipelines longs, `cd`, args multiples.
- **22 tests d'intégration** (`tests/run_tests.sh`) : exécution réelle du
  binary sur `echo`, pipelines, `cd` (succès/erreurs), `execve` échoué,
  propagation du code de sortie, `&&`/`||` (short-circuit), **250 pipes
  avec `ulimit -n 30`**.

## Conformité

Les fichiers `parser.c` et `microshell.c` n'utilisent **aucune** fonction
hors de la liste autorisée. Les helpers string (`my_strlen`) et la
comparaison des séparateurs sont implémentés à la main (pas de
`strlen`/`strtok`/`strdup`/`printf`/`open`/`read`). Les macros
`WIFEXITED`/`WEXITSTATUS`/`WIFSIGNALED`/`WTERMSIG` sont des macros
(non des fonctions) et `environ` est une variable (l'énoncé l'exige).
