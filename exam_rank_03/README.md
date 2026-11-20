# Exam Rank 03

Projet **Exam Rank 03** de 42 School (énoncé le plus récent — questions
level 1 et level 2). Examen C couvrant la manipulation de chaînes, les
descripteurs de fichiers, l'allocation mémoire et les algorithmes de
recherche/backtracking.

## Questions

### Level 1 — obligatoire

| Question      | Dossier           | Description                                            |
|---------------|-------------------|--------------------------------------------------------|
| `filter`      | `filter/`         | Lire stdin, remplacer chaque occurrence de l'argument par `*` (un `*` par caractère). |
| `ft_scanf`    | `ft_scanf/`       | Reproduire `scanf` pour `%s`, `%d`, `%c`.               |
| `get_next_line` | `get_next_line/` | Retourner une ligne lue depuis un descripteur (avec `\n`), sans fuite mémoire. |

### Level 2 — bonus

| Question       | Dossier            | Description                                                       |
|----------------|--------------------|-------------------------------------------------------------------|
| `n_queens`     | `n_queens/`        | Imprimer toutes les solutions du problème des N reines.           |
| `permutations` | `permutations/`    | Imprimer toutes les permutations d'une chaîne (ordre alphabétique). |
| `powerset`     | `powerset/`        | Imprimer tous les sous-ensembles dont la somme vaut `n`.         |
| `rip`          | `rip/`             | Retirer le minimum de parenthèses pour équilibrer, imprimer toutes les solutions. |
| `tsp`          | `tsp/`             | Longueur du plus court chemin fermé visitant toutes les villes (`%.2f`). |

## Build

```bash
make                 # build tous les programmes (Makefile top-level)
make -C filter       # build un seul programme
make -C tsp          # (nécessite -lm, déjà dans le Makefile)
make clean / fclean / re
```

Compile avec `cc -Wall -Wextra -Werror`. Chaque programme respecte sa liste
de fonctions externes autorisées (pas de libft).

## Exécution

```bash
echo 'abcdefgaaaabcdefabc' | ./filter/filter abc
./n_queens/n_queens 4
./permutations/permutations abc
./powerset/powerset 3 1 0 2 4 5 3
./rip/rip '( ( )'
./tsp/tsp < cities.txt
printf '%s\n' "%d %s" | ./ft_scanf/test_ft_scanf   # comparaison avec scanf
./get_next_line/test_gnl file.txt
```

## Tests

Suite de tests complets (Python) dans `tests/test_exam_rank_03.py` :

```bash
python3 tests/test_exam_rank_03.py
```

**90/90 tests passés.** Pour chaque question algorithmique, la sortie du
programme est comparée à une **implémentation de référence en Python**
(n_queens, permutations, powerset, rip, tsp, filter, get_next_line), ce qui
garantit une correction à 100 %. La suite vérifie aussi :
- compilation stricte (`-Wall -Wextra -Werror`),
- validation des arguments,
- cas limites (lignes plus longues que `BUFFER_SIZE`, fichiers vides,
  chaînes vides, n=0/1, etc.),
- tests de stress (génération aléatoire),
- absence de fuites mémoire (vérifiée avec AddressSanitizer sur
  `get_next_line` et `filter`).

## Structure

```
exam_rank_03/
├── filter/          filter.c, Makefile
├── ft_scanf/        ft_scanf.c, test_ft_scanf.c, Makefile
├── get_next_line/   get_next_line.c, get_next_line.h, test_gnl.c, Makefile
├── n_queens/        n_queens.c, Makefile
├── permutations/    permutations.c, Makefile
├── powerset/        powerset.c, Makefile
├── rip/             rip.c, Makefile
├── tsp/             tsp.c, Makefile
├── subjects/        énoncés récupérés (subject.txt par question)
├── tests/           test_exam_rank_03.py
├── Makefile         build top-level
└── README.md
```
