*This project has been created as part of the 42 curriculum by jojo8356.*

<div align="center">

# push_swap — version française

**Trier une pile avec deux piles et onze instructions — et battre le benchmark.**

![42](https://img.shields.io/badge/42-Rank%2002-black?style=flat-square)
![C](https://img.shields.io/badge/C-98-blue?style=flat-square)
![Benchmark](https://img.shields.io/badge/100%20valeurs-583%20%2F%20700-brightgreen?style=flat-square)
![Benchmark](https://img.shields.io/badge/500%20valeurs-5208%20%2F%205500-brightgreen?style=flat-square)
![Tests](https://img.shields.io/badge/tests-388%20v%C3%A9rifications-brightgreen?style=flat-square)

[English](README.md)

</div>

---

# push_swap — version française

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
