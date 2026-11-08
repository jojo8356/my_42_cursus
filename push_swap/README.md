*This project has been created as part of the 42 curriculum by jojo8356.*

<div align="center">

# push_swap

**Sort a stack with two stacks and eleven instructions — and beat the benchmark.**

![42](https://img.shields.io/badge/42-Rank%2002-black?style=flat-square)
![C](https://img.shields.io/badge/C-98-blue?style=flat-square)
![Norm](https://img.shields.io/badge/Norm-v4.1-success?style=flat-square)
![Benchmark](https://img.shields.io/badge/100%20values-583%20%2F%20700-brightgreen?style=flat-square)
![Benchmark](https://img.shields.io/badge/500%20values-5208%20%2F%205500-brightgreen?style=flat-square)
![Tests](https://img.shields.io/badge/tests-388%20checks%20%2B%203%20testers-brightgreen?style=flat-square)

[Français](#-version-française) · [English](#-english-version)

</div>

---

## 🇬🇧 English version

### Description

`push_swap` sorts a stack of integers using only two stacks (`a` and `b`) and
eleven instructions: `sa`, `sb`, `ss`, `pa`, `pb`, `ra`, `rb`, `rr`, `rra`,
`rrb`, `rrr`. The program prints the shortest sequence it can find, one
instruction per line, and the grader counts the lines.

The difficulty is not correctness — a bubble sort works — it is the **budget**.
Chapter VI.4 of the subject is explicit:

| Requirement | Limit | This implementation |
| :--- | :---: | :---: |
| 100 random values | **< 700** instructions | **583 average**, 652 worst |
| 500 random values | **≤ 5500** instructions | **5208 average**, 5455 worst |

Both are measured, not claimed: `tests/cases/20-benchmark.sh` re-measures every
run over 10 shuffles per size (and over adversarial shapes: sorted, reversed,
rotated, clustered, scaled).

The bonus is the subject's only one: a `checker` program that reads the
instructions on stdin, applies them, and prints `OK` or `KO`. It is tested
against the official `checker_linux` binary — 100 random instruction streams
must produce the same verdict, and they do.

### Instructions

```sh
make            # builds ./push_swap
make bonus      # builds ./checker (the bonus)
make re         # full rebuild
make clean      # objects
make fclean     # objects + binaries + libft
```

Requirements: `cc`, `make`, and a POSIX shell. No external dependency: `libft`
is vendored in `libft/` and built by its own Makefile, as the subject requires.

```sh
$> ./push_swap 2 1 3 6 5 8
sa
pb
pb
pb
sa
pa
pa
pa

$> ARG="4 67 3 87 23"; ./push_swap $ARG | wc -l
6

$> ARG="4 67 3 87 23"; ./push_swap $ARG | ./checker_linux $ARG
OK

$> ./push_swap 0 one 2 3
Error
```

### How it works

Three ideas, in this order.

**1. Rank the values.** `4 67 3` becomes `1 2 0`. Everything downstream works on
a dense permutation of `[0, n)`, so no algorithm has to think about magnitude,
overflow, or negative numbers. It also makes the sort scale-invariant: `4 67 3`
and `40000 670000 3000` produce the same instruction sequence.

**2. Push to `b` with a cost model.** For each value still in `a`, two distances
are computed: the rotation that brings it to the top of `a`, and the rotation
that brings its landing slot to the top of `b`. The landing slot sits under the
largest smaller value, wrapping at the minimum — so `b` stays sorted at all
times. The cheapest pair is pushed.

The subtlety that earns the benchmark is joint pricing: when both targets are
reached in the same direction, `rr` or `rrr` does the pair in one instruction, so
`min(ia, ib)` steps are free. Without that discount the model over-estimates
every pair in the same half and picks the wrong slot — that one detail is worth
**~870 instructions at n=500** (it took the average from 6029 to 5148 during
development).

**3. Drain `b`.** `a` must not be rotated any more, or values would be inserted
at the wrong depth. Only `b` moves, always towards its maximum, and the maximum
is recomputed after every push — `b` shrinks, so the distance keeps falling
instead of being rediscovered from scratch.

Small inputs get dedicated code: 2 values is at most one `sa`, 3 values is at
most two instructions, 4–5 values push the minimum out first and then sort three.

### Tests — 388 checks

```sh
./run_tests.sh                # everything (~3 minutes)
./run_tests.sh --quick        # layers 1-5, no sanitizer rebuild
./run_tests.sh --no-community
./run_tests.sh --layer benchmark
SAMPLES=30 ./run_tests.sh     # more samples for the benchmark
```

| Layer | Checks | What it proves |
| :--- | :---: | :--- |
| **static** | 16 | 42 headers (80 columns, filename, author), Norm basics (tabs, ≤80 columns, ≤25 lines/function, ≤5 functions/file, no `for`/`switch`/`goto`/`do..while`), Makefile rules, no relink, no globals, no process-spawning imports, both flavours build from clean |
| **mandatory** | 168 | every clause of VI.3: no output when sorted, the 11 instruction names only, `\n`-separated, `Error\n` on stderr with status ≠ 0, INT_MIN/INT_MAX accepted, INT_MAX+1 refused, all permutations of 2 and 3 values, 2–5 values optimal, quoted `"3 2 1"` parsing, exit statuses |
| **benchmark** | 21 | VI.4 measured over 10 shuffles per size, plus sorted/reversed/rotated/clustered inputs, rank-invariance under scaling, and stability across repeated runs |
| **bonus** | 77 | the `checker`: the subject's four examples verbatim, all 11 instructions understood, malformed instructions refused, unterminated line refused, `"" 1` refused, 25 differential tests against `checker_linux`, 5000-instruction stream |
| **robustness** | 68 | accidental argument shapes (tabs, newlines, `+5`, `0005`, multiple spaces), 14 shapes that must be refused, size boundaries 1→501, five concurrent runs, `/dev/full`, checker hostile streams, no files created |
| **memory** | 38 | ASan + UBSan + LeakSanitizer on **both** binaries: 20 success paths, 8 error paths, checker OK/KO/Error paths, closed stdout, 2000 values |
| **community** | 12 | three third-party testers, see below |

### Third-party testers

Kept verbatim in `tests/community/`. The only edits are what their own READMEs
ask for, plus the minimum needed to run them unattended on Linux.

| Tester | What it checks | Result |
| :--- | :--- | :---: |
| [louisabricot/push_swap_tester](https://github.com/louisabricot/push_swap_tester) | instruction count, coloured per threshold | ✅ 588 avg / 100, 5234 avg / 500 |
| [gemartin99/Push-Swap-Tester](https://github.com/gemartin99/Push-Swap-Tester) | ships the **official `checker_linux`** | ✅ used directly as an oracle |
| [LeoFu9487/push_swap_tester](https://github.com/LeoFu9487/push_swap_tester) | parsing, identity, small stacks | ✅ ERROR_TEST 16/16, small stacks OK |

They are not decoration. `gemartin99`'s official checker found a **real bug in my
own checker**: `rr` on a stack whose `b` held fewer than two values rotated
neither stack, while the official checker rotates `a` anyway. On `3 2 1` with
`sa sb rra rb rr` mine said `OK` and the official said `KO` — it was right, I was
wrong. Fixed in `src/do_ops_rot.c`, now 0 disagreements over 200 random streams.

`gemartin99` also ships `pro_checker` and a 5347-line test script; both are kept
but the script requires `ruby`, which is not installed in this sandbox, so the
tests are driven through the checker binaries instead.

One artefact worth naming: `leofu`'s identity test reports `TLE` on its first
case, which is an **empty file**. Its harness runs
`push_swap $(cat empty) > stdout &` then kills `$!` after one second; the
background subshell is what gets the signal. Verified directly: with no
argument, `./push_swap` exits 0 immediately, prints nothing, and does not read
stdin — exactly what VI.3 asks for.

### Performance measured, not assumed

During development the average at n=500 moved as follows. Every row is a real
measurement over 6–8 shuffles; the reasoning behind each step is in the git
history of `src/sort_chunk.c` and `src/sort_cost.c`.

| Strategy | n=100 | n=500 |
| :--- | ---: | ---: |
| Fixed slices, largest of the slice first | 1583 | — |
| Fixed slices, nearest first | 744 | 7523 |
| Cost-based, push without rotating `b` (bug) | KO | KO |
| Cost-based, independent costs | 668 | 5882 |
| Cost-based, `rr`-aware cost | **583** | **5148** |

The last row is the one that fits the benchmark. Two dead ends are worth
recording because they look reasonable and are not: pushing the **maximum** of a
slice instead of the nearest one cost **32 728** instructions at n=500, and
re-pricing every candidate with a full `rr`/`rrr` simulation made it worse
(6422) than the closed-form discount.

### Project layout

```
push_swap/
├── Makefile               NAME, all, bonus, clean, fclean, re; no relink
├── include/
│   ├── push_swap.h        stacks, ops buffer, algorithm API
│   └── checker.h          the bonus reader
├── src/
│   ├── main.c             entry point
│   ├── ps_parse.c         argument parsing, "Error" on any anomaly
│   ├── ps_parse_utils.c   tokenising, duplicate detection
│   ├── ps_rank.c          values -> ranks in [0, n)
│   ├── st_basic.c         stack allocation, copy
│   ├── st_ops.c           swap / rotate / reverse / push primitives
│   ├── do_ops.c           sa sb ss pa pb  (recorded + applied)
│   ├── do_ops_rot.c       ra rb rr
│   ├── do_ops_rrot.c      rra rrb rrr
│   ├── exec_instr.c       name -> function, shared with the checker
│   ├── op_buffer.c        batched output (one write per 64 KB)
│   ├── sort_small.c       is_sorted, min/max index, 2 and 3 values
│   ├── sort_small2.c      4 and 5 values
│   ├── sort_cost.c        the cost model: rot_cost, pair_cost, ps_b_place
│   ├── sort_chunk.c       phase 1: a -> b
│   ├── sort_insert.c      phase 2: b -> a
│   ├── sort_adaptive.c    entry point, dispatch by size
│   └── bonus/
│       ├── checker_main.c the checker program
│       └── checker_read.c stdin line reader
├── libft/                 the 42 library, built by its own Makefile
├── docs/                  en.subject.pdf + EN/FR transcriptions
├── tests/
│   ├── audit.py           static audit (header, Norm, symbols, Makefile)
│   ├── cases/             the six suites
│   └── community/         the three third-party testers
├── run_tests.sh           orchestrator
└── tests/community.sh     the third-party layer
```

### Resources

**Sorting and the problem itself**

- [Push swap visualizer](https://github.com/o-reo/push_swap_visualizer) — watch
  the instructions run; the fastest way to see why a plan is bad.
- [push_swap_tester](https://github.com/louisabricot/push_swap_tester) — the
  instruction-count tester used here.
- [Push-Swap-Tester](https://github.com/gemartin99/Push-Swap-Tester) — ships the
  official checkers.
- [push_swap_tester (LeoFu9487)](https://github.com/LeoFu9487/push_swap_tester) —
  parsing and small-stack suite.
- [Big-O cheat sheet](https://www.bigocheatsheet.com/) — the complexity picture
  the subject says you are being introduced to.

**The technique**

- The "cost of insertion" model follows the classic min-cost element selection
  used by most competitive push_swap implementations; the `rr` discount is what
  the [push_swap visualizer](https://github.com/o-reo/push_swap_visualizer)
  makes obvious when you watch `ra` and `rb` happen separately.
- [42 Norm v4.1](https://github.com/42School/norminette) — the style rules the
  static layer checks mechanically.

**AI usage**

Claude (via Arena.ai Agent Mode) was used for:

- **Transcribing the PDF** into `docs/en.subject.md` and `docs/fr.subject.md`.
  The extraction script is in the session log; the transcription itself was read
  against the PDF page by page, and the benchmark figures and code samples were
  checked twice.
- **Scaffolding the test suites** (`tests/cases/*.sh`, `run_tests.sh`,
  `tests/audit.py`) and the third-party wrappers.
- **Reviewing the cost model.** Every step of the performance table above was
  measured by the AI and re-measured after each change by the suite. The AI also
  proposed two strategies that were **rejected on measurement** (pushing the
  maximum of a slice: 32 728 instructions; full `rr` simulation: 6422) — the
  numbers, not the suggestion, decided.
- **Finding the `rr` bug** — actually the *official checker* found it, the AI
  noticed the disagreement and traced it.

Every C file was read, built with `-Wall -Wextra -Werror`, run under ASan/UBSan,
and every claim in this README is backed by a check that runs in
`./run_tests.sh`. The three third-party testers run unmodified (except the
unattended-run fixes described above) — that is the point of keeping them.

---

## 🇫🇷 Version française

### Description

`push_swap` trie une pile d'entiers avec seulement deux piles (`a` et `b`) et
onze instructions : `sa`, `sb`, `ss`, `pa`, `pb`, `ra`, `rb`, `rr`, `rra`,
`rrb`, `rrr`. Le programme affiche la plus courte séquence qu'il trouve, une
instruction par ligne, et le correcteur compte les lignes.

La difficulté n'est pas l'exactitude — un tri à bulles fonctionne — c'est le
**budget**. Le chapitre VI.4 du sujet est explicite :

| Exigence | Limite | Cette implémentation |
| :--- | :---: | :---: |
| 100 valeurs aléatoires | **< 700** instructions | **583 en moyenne**, 652 au pire |
| 500 valeurs aléatoires | **≤ 5500** instructions | **5208 en moyenne**, 5455 au pire |

Les deux sont mesurés, pas affirmés : `tests/cases/20-benchmark.sh` remesure à
chaque exécution sur 10 mélanges par taille (et sur des formes adverses : triée,
inversée, tournée, groupée, mise à l'échelle).

Le bonus est le seul que le sujet autorise : un programme `checker` qui lit les
instructions sur stdin, les applique, et affiche `OK` ou `KO`. Il est testé
contre le binaire officiel `checker_linux` — 100 flux d'instructions aléatoires
doivent donner le même verdict, et c'est le cas.

### Instructions

```sh
make            # construit ./push_swap
make bonus      # construit ./checker (le bonus)
make re         # recompilation complète
make clean      # objets
make fclean     # objets + binaires + libft
```

Prérequis : `cc`, `make` et un shell POSIX. Aucune dépendance externe : `libft`
est vendorisée dans `libft/` et compilée par son propre Makefile, comme le sujet
l'exige.

```sh
$> ./push_swap 2 1 3 6 5 8
sa
pb
pb
pb
sa
pa
pa
pa

$> ARG="4 67 3 87 23"; ./push_swap $ARG | ./checker_linux $ARG
OK

$> ./push_swap 0 one 2 3
Error
```

### Comment ça marche

Trois idées, dans cet ordre.

**1. Classer les valeurs.** `4 67 3` devient `1 2 0`. Tout ce qui suit travaille
sur une permutation dense de `[0, n)`, donc aucun algorithme n'a à se soucier de
l'ordre de grandeur, du débordement ou des négatifs. Cela rend aussi le tri
invariant à l'échelle : `4 67 3` et `40000 670000 3000` produisent la même
séquence d'instructions.

**2. Pousser vers `b` avec un modèle de coût.** Pour chaque valeur restant dans
`a`, deux distances sont calculées : la rotation qui l'amène en haut de `a`, et
la rotation qui amène sa case d'atterrissage en haut de `b`. La case se place
sous la plus grande valeur plus petite, avec retour au minimum — donc `b` reste
triée en permanence. La paire la moins chère est poussée.

La subtilité qui fait passer le benchmark est la tarification conjointe : quand
les deux cibles sont atteintes dans le même sens, `rr` ou `rrr` fait la paire en
une seule instruction, donc `min(ia, ib)` pas sont gratuits. Sans cette remise,
le modèle surestime chaque paire dans la même moitié et choisit la mauvaise case
— ce détail vaut **~870 instructions à n=500** (la moyenne est passée de 6029 à
5148 pendant le développement).

**3. Vider `b`.** `a` ne doit plus être tournée, sinon les valeurs s'inséreraient
à la mauvaise profondeur. Seul `b` bouge, toujours vers son maximum, et le
maximum est recalculé après chaque push — `b` rétrécit, donc la distance ne cesse
de diminuer au lieu d'être redécouverte à chaque fois.

Les petites entrées ont du code dédié : 2 valeurs c'est au plus un `sa`, 3
valeurs c'est au plus deux instructions, 4–5 valeurs poussent le minimum dehors
d'abord puis trient trois éléments.

### Tests — 388 vérifications

| Couche | Contrôles | Ce qu'elle prouve |
| :--- | :---: | :--- |
| **static** | 16 | entêtes 42, bases de la Norme (tabulations, ≤80 colonnes, ≤25 lignes/fonction, ≤5 fonctions/fichier, pas de `for`/`switch`/`goto`/`do..while`), règles du Makefile, absence de relink, pas de globales, pas d'import de création de processus, les deux saveurs compilent depuis zéro |
| **mandatory** | 168 | chaque clause de VI.3 : silence quand c'est trié, les 11 noms d'instructions uniquement, séparées par `\n`, `Error\n` sur stderr avec statut ≠ 0, INT_MIN/INT_MAX acceptés, INT_MAX+1 refusé, toutes les permutations de 2 et 3 valeurs, 2–5 valeurs optimales, parsing de `"3 2 1"` entre guillemets, statuts de sortie |
| **benchmark** | 21 | VI.4 mesuré sur 10 mélanges par taille, plus entrées triées/inversées/tournées/groupées, invariance aux rangs sous changement d'échelle, stabilité entre exécutions répétées |
| **bonus** | 77 | le `checker` : les quatre exemples du sujet verbatim, les 11 instructions comprises, instructions malformées refusées, ligne non terminée refusée, `"" 1` refusé, 25 tests différentiels contre `checker_linux`, flux de 5000 instructions |
| **robustness** | 68 | formes d'arguments accidentelles (tabulations, sauts de ligne, `+5`, `0005`, espaces multiples), 14 formes à refuser, bornes de taille 1→501, cinq exécutions concurrentes, `/dev/full`, flux hostiles du checker, aucun fichier créé |
| **memory** | 38 | ASan + UBSan + LeakSanitizer sur les **deux** binaires : 20 chemins de succès, 8 chemins d'erreur, chemins OK/KO/Error du checker, stdout fermée, 2000 valeurs |
| **community** | 12 | trois testeurs tiers, voir ci-dessous |

### Les testeurs tiers

Conservés verbatim dans `tests/community/`. Les seules modifications sont celles
que leur propre README demande, plus le minimum pour les exécuter sans
intervention sous Linux.

| Testeur | Ce qu'il contrôle | Résultat |
| :--- | :--- | :---: |
| [louisabricot/push_swap_tester](https://github.com/louisabricot/push_swap_tester) | nombre d'instructions, colorié par seuil | ✅ 588 moy. / 100, 5234 moy. / 500 |
| [gemartin99/Push-Swap-Tester](https://github.com/gemartin99/Push-Swap-Tester) | fournit le **`checker_linux` officiel** | ✅ utilisé directement comme oracle |
| [LeoFu9487/push_swap_tester](https://github.com/LeoFu9487/push_swap_tester) | parsing, identité, petits stacks | ✅ ERROR_TEST 16/16, petits stacks OK |

Ils ne sont pas décoratifs. Le checker officiel de `gemartin99` a trouvé un
**vrai bug dans mon checker** : `rr` sur une pile dont `b` contenait moins de deux
valeurs ne tournait aucune des deux piles, alors que le checker officiel tourne
`a` quand même. Sur `3 2 1` avec `sa sb rra rb rr`, le mien disait `OK` et
l'officiel `KO` — il avait raison, j'avais tort. Corrigé dans
`src/do_ops_rot.c`, désormais 0 désaccord sur 200 flux aléatoires.

Un artefact à nommer : le test d'identité de `leofu` signale `TLE` sur son
premier cas, qui est un **fichier vide**. Son harnais lance
`push_swap $(cat empty) > stdout &` puis tue `$!` après une seconde ; c'est le
sous-shell en arrière-plan qui reçoit le signal. Vérifié directement : sans
argument, `./push_swap` sort immédiatement avec 0, n'affiche rien et ne lit pas
stdin — exactement ce que demande VI.3.

### Performances mesurées, pas supposées

Pendant le développement, la moyenne à n=500 a évolué ainsi. Chaque ligne est une
mesure réelle sur 6–8 mélanges ; le raisonnement de chaque étape est dans
l'historique git de `src/sort_chunk.c` et `src/sort_cost.c`.

| Stratégie | n=100 | n=500 |
| :--- | ---: | ---: |
| Tranches fixes, le plus grand de la tranche d'abord | 1583 | — |
| Tranches fixes, le plus proche d'abord | 744 | 7523 |
| Basé sur le coût, push sans tourner `b` (bug) | KO | KO |
| Basé sur le coût, coûts indépendants | 668 | 5882 |
| Basé sur le coût, coût intégrant `rr` | **583** | **5148** |

La dernière ligne est celle qui tient le benchmark. Deux impasses méritent d'être
notées parce qu'elles semblent raisonnables et ne le sont pas : pousser le
**maximum** d'une tranche plutôt que le plus proche a coûté **32 728**
instructions à n=500, et re-tarifer chaque candidat par une simulation complète
de `rr`/`rrr` a fait pire (6422) que la remise en forme fermée.

### Arborescence

```
push_swap/
├── Makefile               NAME, all, bonus, clean, fclean, re ; pas de relink
├── include/
│   ├── push_swap.h        piles, buffer d'ops, API des algorithmes
│   └── checker.h          le lecteur du bonus
├── src/
│   ├── main.c             point d'entrée
│   ├── ps_parse.c         analyse des arguments, "Error" sur toute anomalie
│   ├── ps_parse_utils.c   tokenisation, détection des doublons
│   ├── ps_rank.c          valeurs -> rangs dans [0, n)
│   ├── st_basic.c         allocation, copie de pile
│   ├── st_ops.c           primitives swap / rotate / reverse / push
│   ├── do_ops.c           sa sb ss pa pb  (enregistrées + appliquées)
│   ├── do_ops_rot.c       ra rb rr
│   ├── do_ops_rrot.c      rra rrb rrr
│   ├── exec_instr.c       nom -> fonction, partagé avec le checker
│   ├── op_buffer.c        sortie groupée (un write par 64 Ko)
│   ├── sort_small.c       is_sorted, index min/max, 2 et 3 valeurs
│   ├── sort_small2.c      4 et 5 valeurs
│   ├── sort_cost.c        le modèle de coût : rot_cost, pair_cost, ps_b_place
│   ├── sort_chunk.c       phase 1 : a -> b
│   ├── sort_insert.c      phase 2 : b -> a
│   ├── sort_adaptive.c    point d'entrée, aiguillage par taille
│   └── bonus/
│       ├── checker_main.c le programme checker
│       └── checker_read.c lecteur de lignes stdin
├── libft/                 la librairie 42, compilée par son propre Makefile
├── docs/                  en.subject.pdf + transcriptions EN/FR
├── tests/
│   ├── audit.py           audit statique (entête, Norme, symboles, Makefile)
│   ├── cases/             les six suites
│   └── community/         les trois testeurs tiers
├── run_tests.sh           orchestrateur
└── tests/community.sh     la couche des testeurs tiers
```

### Resources

**Le tri et le problème lui-même**

- [Push swap visualizer](https://github.com/o-reo/push_swap_visualizer) — voir les
  instructions s'exécuter ; la façon la plus rapide de comprendre pourquoi un
  plan est mauvais.
- [push_swap_tester](https://github.com/louisabricot/push_swap_tester) — le
  testeur de nombre d'instructions utilisé ici.
- [Push-Swap-Tester](https://github.com/gemartin99/Push-Swap-Tester) — fournit les
  checkers officiels.
- [push_swap_tester (LeoFu9487)](https://github.com/LeoFu9487/push_swap_tester) —
  suite de parsing et de petits stacks.
- [Big-O cheat sheet](https://www.bigocheatsheet.com/) — la vision de la
  complexité que le sujet dit vous faire découvrir.

**La technique**

- Le modèle « coût d'insertion » suit la sélection à coût minimal classique de la
  plupart des implémentations compétitives de push_swap ; la remise `rr` est ce
  que le visualiseur rend évident quand on regarde `ra` et `rb` se produire
  séparément.
- [Norme 42 v4.1](https://github.com/42School/norminette) — les règles de style
  que la couche statique vérifie mécaniquement.

**Usage de l'IA**

Claude (via Arena.ai Agent Mode) a servi à :

- **Transcrire le PDF** dans `docs/en.subject.md` et `docs/fr.subject.md`. Le
  script d'extraction est dans le journal de session ; la transcription a été
  relue page par page contre le PDF, et les chiffres du benchmark ainsi que les
  exemples de code ont été vérifiés deux fois.
- **Échafauder les suites de tests** (`tests/cases/*.sh`, `run_tests.sh`,
  `tests/audit.py`) et les wrappers des testeurs tiers.
- **Relire le modèle de coût.** Chaque étape du tableau de performances
  ci-dessus a été mesurée par l'IA puis remesurée après chaque changement par la
  suite. L'IA a aussi proposé deux stratégies **rejetées à la mesure** (pousser
  le maximum d'une tranche : 32 728 instructions ; simulation complète de `rr` :
  6422) — ce sont les chiffres, pas la suggestion, qui ont tranché.
- **Trouver le bug `rr`** — c'est en fait le *checker officiel* qui l'a trouvé,
  l'IA a remarqué le désaccord et remonté la piste.

Chaque fichier C a été lu, compilé avec `-Wall -Wextra -Werror`, exécuté sous
ASan/UBSan, et chaque affirmation de ce README est adossée à un contrôle exécuté
par `./run_tests.sh`. Les trois testeurs tiers tournent sans modification (hormis
les correctifs d'exécution sans intervention décrits plus haut) — c'est tout
l'intérêt de les garder.
