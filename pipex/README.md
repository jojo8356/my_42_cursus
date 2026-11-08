*This project has been created as part of the 42 curriculum by jojo8356.*

<div align="center">

# pipex

**Reproduce the UNIX pipe with `fork`, `dup2` and `execve` — mandatory part and both bonuses.**

![42](https://img.shields.io/badge/42-Rank%2002-black?style=flat-square)
![C](https://img.shields.io/badge/C-98-blue?style=flat-square)
![Norm](https://img.shields.io/badge/Norm-v4.1-success?style=flat-square)
![Tests](https://img.shields.io/badge/tests-131%20checks%20%2B%2071%20third--party-brightgreen?style=flat-square)
![Leaks](https://img.shields.io/badge/leaks-0-success?style=flat-square)

[Français](#-version-française) · [English](#-english-version)

</div>

---

## 🇬🇧 English version

### The subject in one paragraph

`pipex` must behave exactly like the shell pipeline it mimics:

```sh
$> ./pipex infile "ls -l" "wc -l" outfile      #  < infile ls -l | wc -l > outfile
$> ./pipex infile "grep a1" "wc -w" outfile    #  < infile grep a1 | wc -w > outfile
```

The bonus adds two things the shell also knows: **N pipes**
(`./pipex f1 cmd1 cmd2 cmd3 f2`) and **here_doc**
(`./pipex here_doc LIMITER cmd1 cmd2 file`, i.e. `cmd1 << LIMITER | cmd2 >> file`).

The full transcription of the PDF is in [`docs/en.subject.md`](docs/en.subject.md)
and [`docs/fr.subject.md`](docs/fr.subject.md); the PDF itself is
[`docs/en.subject.pdf`](docs/en.subject.pdf) (v2).

### Build and run

```sh
make            # mandatory: ./pipex infile cmd1 cmd2 outfile
make bonus      # bonus: N pipes + here_doc
make re         # full rebuild
make clean      # objects
make fclean     # objects + binary + libft
```

The Makefile compiles with `cc -Wall -Wextra -Werror`, builds `libft/` through
its own Makefile first, and **never relinks**: `make` twice in a row does nothing
the second time. The mandatory and bonus objects live in two separate trees
(`obj/mandatory`, `obj/bonus`) so that a bonus binary can never be linked out of
mandatory objects by accident.

### What it does, precisely

| Behaviour | Why it matters |
| :--- | :--- |
| Each redirection is done by the child that owns it | The shell fails `< infile` for cmd1 only, and `> outfile` for cmd2 only. Opening in the parent makes both commands run with a redirect the shell would have refused. |
| A failing cmd1 does **not** stop cmd2 | bash has no `pipefail`: `< nope cat \| wc -l > out` writes `0` and exits `0`. `pipex` does the same. |
| Exit status = the **last** command's | Again the shell's `$?`. A bogus cmd1 with a working cmd2 still returns 0. |
| `outfile` is `O_TRUNC` in the mandatory and multi-pipe forms | `>` truncates. |
| `outfile` is `O_APPEND` with here_doc | The subject says `>>`. This is the detail most implementations get wrong. |
| here_doc is read by the parent, before any fork | Several children reading the same stdin would steal each other's lines. |
| The parent closes each pipe's write end immediately | Otherwise the reader never gets EOF and the programme hangs: the classic pipex deadlock. |
| `PATH` lookup refuses directories | `access(X_OK)` succeeds on a directory, `execve` then fails with `EACCES`; the shell says "command not found" instead. |
| `strdup` is reimplemented | It is not in the subject's list of allowed external functions. |

### The bonus, completely

**Multiple pipes** — one `fork` per command, `N-1` pipes, the previous read end
becomes the next stdin. Tested with 1, 2, 3, 4, 5, 6, 8 and 10 commands, and with
400 KB pushed through 7 pipes to prove there is no deadlock.

**here_doc** — `./pipex here_doc LIMITER cmd1 cmd2 ... file` behaves like
`cmd1 << LIMITER \| cmd2 \| ... >> file`:

- the limiter line is consumed and never transmitted;
- `ENDX` does not stop a here_doc looking for `END`;
- EOF before the limiter does not hang;
- the body is written through an anonymous pipe, so **no temporary file** is
  left on disk (nothing to `unlink`);
- the output file is opened in `O_APPEND`.

### Tests

```sh
./run_tests.sh              # everything
./run_tests.sh --quick      # layers 1-4, no sanitizer rebuild
./run_tests.sh --no-community
./run_tests.sh --layer bonus
```

| Layer | Checks | What it proves |
| :--- | :---: | :--- |
| **static** | 18 | 42 headers (80 columns, filename, author), Norm basics (tabs, ≤80 columns, ≤25 lines/function, ≤5 functions/file, no ternary/for/switch/do-while), external functions against the subject's list, imported symbols via `nm`, Makefile rules, no relink, both flavours build |
| **mandatory** | 36 | differential testing **against bash**: byte-exact output, exit statuses, stderr shape, missing infile, unwritable outfile, wrong argument counts, PATH lookup, env inheritance, truncation, 100 KB through one pipe |
| **bonus** | 27 | chains of 1→10 commands, byte-exact chains, 400 KB through 7 pipes, exit status, failing middle command, here_doc body/limiter/appending, empty body, `ENDX` vs `END`, EOF before limiter, here_doc read from a pipe, argument errors, and a check that each command really runs in its own process |
| **robustness** | 29 | hostile argument counts, empty/whitespace commands, directory as infile/outfile, a command killed by `SIGSEGV`, a reader closing the pipe early (SIGPIPE), no zombies, 5 concurrent runs, 64 KB of binary data, 2000-line here_doc |
| **memory** | 21 | ASan + UBSan + LeakSanitizer on **both** binaries, on success **and** error paths (missing infile, unwritable outfile, unknown command, empty command, here_doc errors) |
| **community** | 71 | three third-party testers, kept verbatim |

Total: **131 checks of our own** plus **71 third-party checks**, i.e. 202
passing assertions across the six layers.

### Third-party testers (kept verbatim)

Each lives in `tests/community/<author>/` exactly as its author published it.
The only edit is the path to the project — an edit their own README asks for
("ADJUST PATH TO PIPEX DIRECTORY IF NECESSARY" in michmos, `PROJECT_DIRECTORY`
in vfurmane's `config.vars`).

| Tester | Checks | Result |
| :--- | :---: | :---: |
| [vfurmane/pipex-tester](https://github.com/vfurmane/pipex-tester) | 31 | ✅ 31/31 |
| [michmos/42_pipex_tester](https://github.com/michmos/42_pipex_tester) | 28 | ✅ 28/28 |
| [Yoo0lh/pipex_tester_42](https://github.com/Yoo0lh/pipex_tester_42) | 12 | ✅ 12/12 |

They earned their place: they found four real defects, all fixed here.

1. **A failing `< infile` aborted the whole chain.** The first version opened
   the input file in the parent and returned on error. bash keeps going:
   `< nope cat | wc -l > out` writes `0` and returns 0. The redirection now
   happens in the child, as in the shell.
2. **A failing `> outfile` skipped cmd1.** Same cause, same fix: the output file
   is opened by the last child, so the earlier commands still run and the timing
   of `sleep 3` is preserved (michmos compares execution times).
3. **An unwritable outfile still inherited pipex's own stdin.** With
   `here_doc`-style input attached, the last command consumed the parent's
   stream instead of an empty one. The fallback is now `/dev/null`.
4. **`./pipex in cat cat` created a file called `cat`.** Four arguments were
   accepted in bonus mode, so `cat` was taken as the output file. The
   argument count is now validated *before* anything is opened.

### How the error messages are shaped

The subject says "handle the errors like the shell command". A shell prints, for
a failing command:

```
pipex: infile: No such file or directory
pipex: nosuchcmd: command not found
```

The same wording is used, and `px_error()` is the only function in the project
that writes to stderr — one place to change, one place to audit.

### Project layout

```
pipex/
├── Makefile                  NAME, all, bonus, clean, fclean, re
├── include/
│   ├── pipex.h               mandatory API
│   └── pipex_bonus.h         bonus API (+ t_px)
├── src/
│   ├── main.c                mandatory entry point
│   ├── main_bonus.c          bonus entry point
│   ├── pipex_run.c           one pipe, two commands
│   ├── pipex_exec.c          PATH resolution + execve, shell-like errors
│   ├── pipex_path.c          PATH parsing, px_strdup (strdup is forbidden)
│   ├── pipex_split.c         command line -> argv
│   ├── pipex_error.c         px_error(), px_usage()
│   └── bonus/
│       ├── pipex_run_bonus.c   argument parsing, N-command chain driver
│       ├── pipex_child_bonus.c the child side (fds, redirections, exec)
│       └── pipex_here_doc_bonus.c  << LIMITER reader
├── libft/                    the 42 library, compiled by its own Makefile
├── docs/                     en.subject.pdf + EN/FR transcriptions
├── tests/
│   ├── audit.py              static audit (headers, Norm, symbols, rules)
│   ├── cases/                the five suites
│   └── community/            the three third-party testers
└── run_tests.sh              orchestrator
```

### What I would do differently

The bonus driver went through three rewrites because I started from the fd
plumbing instead of from the shell's semantics. Writing the five bash reference
commands first (in a scratch file, before any C) would have avoided two of them:
the "failing infile keeps going" rule and the "last command wins" rule are both
visible in one line of `bash -c`.

### AI usage

Claude (via Arena.ai Agent Mode) was used to: transcribe the PDF subject,
scaffold the test suites and the orchestrator, and review the fd-closing logic.
Every C file was read, built, run under `-Wall -Wextra -Werror` and under
ASan/UBSan, and every claim in this README is backed by a check that runs in
`./run_tests.sh`. The three third-party testers were run unmodified — that is
the point of keeping them verbatim.

---

## 🇫🇷 Version française

### Le sujet en un paragraphe

`pipex` doit se comporter exactement comme le pipeline shell qu'il imite :

```sh
$> ./pipex infile "ls -l" "wc -l" outfile      #  < infile ls -l | wc -l > outfile
$> ./pipex infile "grep a1" "wc -w" outfile    #  < infile grep a1 | wc -w > outfile
```

Le bonus ajoute ce que le shell sait faire aussi : **N pipes**
(`./pipex f1 cmd1 cmd2 cmd3 f2`) et **here_doc**
(`./pipex here_doc LIMITER cmd1 cmd2 fichier`, soit `cmd1 << LIMITER | cmd2 >> fichier`).

La transcription intégrale du PDF est dans [`docs/fr.subject.md`](docs/fr.subject.md) ;
le PDF officiel est [`docs/fr.subject.pdf`](docs/fr.subject.pdf) (v2).

### Compiler et lancer

```sh
make            # obligatoire : ./pipex infile cmd1 cmd2 outfile
make bonus      # bonus : N pipes + here_doc
make re         # recompilation complète
make clean      # objets
make fclean     # objets + binaire + libft
```

Le Makefile compile avec `cc -Wall -Wextra -Werror`, construit `libft/` via son
propre Makefile, et **ne relink jamais** : deux `make` d'affilée ne font rien la
seconde fois. Les objets obligatoires et bonus sont dans deux arbres séparés
(`obj/mandatory`, `obj/bonus`), pour qu'un binaire bonus ne puisse jamais être
lié par accident avec des objets obligatoires.

### Ce que fait le programme, précisément

| Comportement | Pourquoi c'est important |
| :--- | :--- |
| Chaque redirection est faite par l'enfant qui la possède | Le shell fait échouer `< infile` pour cmd1 seulement, et `> outfile` pour cmd2 seulement. Ouvrir dans le parent fait tourner les deux commandes avec une redirection que le shell aurait refusée. |
| Un cmd1 en échec n'arrête **pas** cmd2 | bash n'a pas `pipefail` : `< nope cat \| wc -l > out` écrit `0` et sort avec `0`. `pipex` fait pareil. |
| Le statut de sortie est celui de la **dernière** commande | Toujours le `$?` du shell. Un cmd1 invalide avec un cmd2 correct retourne 0. |
| `outfile` en `O_TRUNC` en obligatoire et multi-pipes | `>` tronque. |
| `outfile` en `O_APPEND` avec here_doc | Le sujet dit `>>`. C'est le détail que la plupart des implémentations ratent. |
| here_doc est lu par le parent, avant tout fork | Plusieurs enfants lisant le même stdin se voleraient leurs lignes. |
| Le parent ferme immédiatement l'extrémité d'écriture de chaque pipe | Sinon le lecteur ne reçoit jamais EOF et le programme se bloque : c'est le deadlock classique de pipex. |
| La recherche dans `PATH` refuse les répertoires | `access(X_OK)` réussit sur un répertoire, puis `execve` échoue avec `EACCES` ; le shell dit alors « command not found ». |
| `strdup` est réimplémentée | Elle n'est pas dans la liste des fonctions externes autorisées du sujet. |

### Le bonus, complet

**Plusieurs pipes** — un `fork` par commande, `N-1` pipes, l'extrémité de lecture
précédente devient le stdin suivant. Testé avec 1, 2, 3, 4, 5, 6, 8 et 10
commandes, et avec 400 Ko poussés à travers 7 pipes pour prouver l'absence de
deadlock.

**here_doc** — `./pipex here_doc LIMITER cmd1 cmd2 ... fichier` se comporte comme
`cmd1 << LIMITER \| cmd2 \| ... >> fichier` :

- la ligne du limiteur est consommée et jamais transmise ;
- `ENDX` n'arrête pas un here_doc qui cherche `END` ;
- un EOF avant le limiteur ne bloque pas ;
- le corps passe par un pipe anonyme, donc **aucun fichier temporaire** n'est
  laissé sur le disque (rien à `unlink`) ;
- le fichier de sortie est ouvert en `O_APPEND`.

### Tests

| Couche | Contrôles | Ce qu'elle prouve |
| :--- | :---: | :--- |
| **static** | 18 | entêtes 42 (80 colonnes, nom de fichier, auteur), bases de la Norme (tabulations, ≤80 colonnes, ≤25 lignes/fonction, ≤5 fonctions/fichier, pas de ternaire/for/switch/do-while), fonctions externes face à la liste du sujet, symboles importés via `nm`, règles du Makefile, absence de relink, compilation des deux saveurs |
| **mandatory** | 36 | tests différentiels **contre bash** : sortie octet par octet, statuts, forme du stderr, `infile` absente, `outfile` non inscriptible, mauvais nombres d'arguments, recherche `PATH`, héritage de l'environnement, troncature, 100 Ko dans un pipe |
| **bonus** | 27 | chaînes de 1→10 commandes, chaînes exactes, 400 Ko dans 7 pipes, statut de sortie, commande du milieu en échec, corps/limiteur/ajout de here_doc, corps vide, `ENDX` vs `END`, EOF avant le limiteur, here_doc depuis un pipe, erreurs d'arguments, et une preuve que chaque commande tourne dans son propre processus |
| **robustness** | 29 | nombres d'arguments hostiles, commandes vides ou d'espaces, répertoire en `infile`/`outfile`, commande tuée par `SIGSEGV`, lecteur fermant le pipe trop tôt (SIGPIPE), absence de zombies, 5 exécutions concurrentes, 64 Ko de données binaires, here_doc de 2000 lignes |
| **memory** | 21 | ASan + UBSan + LeakSanitizer sur les **deux** binaires, sur les chemins de succès **et** d'erreur |
| **community** | 71 | trois testeurs tiers, conservés verbatim |

Total : **131 contrôles maison** plus **71 contrôles tiers**, soit 202
assertions vertes réparties sur les six couches.

### Les testeurs tiers (verbatim)

| Testeur | Contrôles | Résultat |
| :--- | :---: | :---: |
| [vfurmane/pipex-tester](https://github.com/vfurmane/pipex-tester) | 31 | ✅ 31/31 |
| [michmos/42_pipex_tester](https://github.com/michmos/42_pipex_tester) | 28 | ✅ 28/28 |
| [Yoo0lh/pipex_tester_42](https://github.com/Yoo0lh/pipex_tester_42) | 12 | ✅ 12/12 |

Ils ont mérité leur place : ils ont trouvé quatre vrais défauts, tous corrigés ici.

1. **Un `< infile` en échec interrompait toute la chaîne.** La première version
   ouvrait l'entrée dans le parent et retournait en cas d'erreur. bash continue :
   `< nope cat | wc -l > out` écrit `0` et retourne 0. La redirection se fait
   maintenant dans l'enfant, comme dans le shell.
2. **Un `> outfile` en échec sautait cmd1.** Même cause, même correction : le
   fichier de sortie est ouvert par le dernier enfant, donc les commandes
   précédentes tournent quand même et le temps d'exécution de `sleep 3` est
   préservé (michmos compare les temps).
3. **Un `outfile` non inscriptible héritait du stdin de pipex.** Avec une entrée
   de type here_doc attachée, la dernière commande consommait le flux du parent
   au lieu d'un flux vide. Le repli est désormais `/dev/null`.
4. **`./pipex in cat cat` créait un fichier nommé `cat`.** Quatre arguments
   étaient acceptés en mode bonus, donc `cat` devenait le fichier de sortie. Le
   nombre d'arguments est maintenant validé **avant** toute ouverture.

### Forme des messages d'erreur

Le sujet dit « référez-vous à la commande shell ». Un shell affiche, pour une
commande en échec :

```
pipex: infile: No such file or directory
pipex: nosuchcmd: command not found
```

La même formulation est utilisée, et `px_error()` est la seule fonction du projet
qui écrit sur stderr — un seul endroit à modifier, un seul à auditer.

### Arborescence

```
pipex/
├── Makefile                  NAME, all, bonus, clean, fclean, re
├── include/
│   ├── pipex.h               API obligatoire
│   └── pipex_bonus.h         API bonus (+ t_px)
├── src/
│   ├── main.c                point d'entrée obligatoire
│   ├── main_bonus.c          point d'entrée bonus
│   ├── pipex_run.c           un pipe, deux commandes
│   ├── pipex_exec.c          résolution PATH + execve, erreurs façon shell
│   ├── pipex_path.c          analyse du PATH, px_strdup (strdup est interdite)
│   ├── pipex_split.c         ligne de commande -> argv
│   ├── pipex_error.c         px_error(), px_usage()
│   └── bonus/
│       ├── pipex_run_bonus.c     analyse des arguments, pilote de chaîne
│       ├── pipex_child_bonus.c   côté enfant (fds, redirections, exec)
│       └── pipex_here_doc_bonus.c lecture de << LIMITER
├── libft/                    la librairie 42, compilée par son propre Makefile
├── docs/                     en.subject.pdf + transcriptions EN/FR
├── tests/
│   ├── audit.py              audit statique (entêtes, Norme, symboles, règles)
│   ├── cases/                les cinq suites
│   └── community/            les trois testeurs tiers
└── run_tests.sh              orchestrateur
```

### Ce que je referais différemment

Le pilote bonus a été réécrit trois fois parce que je suis parti de la plomberie
des descripteurs au lieu des sémantiques du shell. Écrire d'abord les cinq
commandes bash de référence (dans un fichier brouillon, avant tout C) aurait
évité deux de ces réécritures : la règle « une entrée en échec n'arrête rien » et
la règle « la dernière commande gagne » tiennent en une ligne de `bash -c`.

### Usage de l'IA

Claude (via Arena.ai Agent Mode) a servi à : transcrire le sujet PDF, échafauder
les suites de tests et l'orchestrateur, et relire la logique de fermeture des
descripteurs. Chaque fichier C a été lu, compilé, exécuté avec
`-Wall -Wextra -Werror` et sous ASan/UBSan, et chaque affirmation de ce README
est adossée à un contrôle exécuté par `./run_tests.sh`. Les trois testeurs tiers
ont tourné sans modification — c'est tout l'intérêt de les garder verbatim.
