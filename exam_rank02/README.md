# Exam Rank 02

Projet **Exam Rank 02** de 42 School. Examen C couvrant les bases du langage
C : manipulation de chaînes, pointeurs, allocation mémoire, listes chaînées
simples et algorithmes classiques. L'examen sélectionne **une question
alléatoire par niveau** (level 1 à level 4) ; ce projet implémente l'ensemble
des fonctions des 4 niveaux, regroupées par dossier.

## Structure

```
exam_rank02/
├── Makefile               # build du binaire exam_rank02 + make test
├── include/
│   └── exam_rank02.h      # prototypes des 4 niveaux (+ type list_t)
├── level_1/
│   └── level_1.c          # questions level 1
├── level_2/
│   └── level_2.c          # questions level 2
├── level_3/
│   └── level_3.c          # questions level 3
├── level_4/
│   └── level_4.c          # questions level 4
├── libft/
│   ├── libft.a            # libft local (utilse par le projet)
│   └── ...
└── tests/
    ├── test_framework.h   # framework de test C léger
    ├── run_tests.sh       # orchestrateur make test
    ├── test_level_1.c
    ├── test_level_2.c
    ├── test_level_3.c
    └── test_level_4.c
```

## Questions

### Level 1 — chaînes & bases

| Fonction          | Description                                             |
|-------------------|---------------------------------------------------------|
| `ft_strlen`       | Longueur d'une chaîne.                                  |
| `ft_strcpy`       | Copier `src` vers `dest`, retourner `dest`.             |
| `ft_swap`         | Échanger les valeurs de deux entiers (par pointeurs).   |
| `first_word`      | Longueur du premier mot d'une chaîne.                  |
| `fizzbuzz`        | Imprimer 1 à 100, `Fizz`/`Buzz`/`FizzBuzz` selon les règles. |
| `ft_putstr`       | Imprimer une chaîne.                                    |
| `repeat_alpha`    | Répéter chaque lettre `n+1` fois (la n-ième lettre `n` fois). |
| `rev_print`       | Imprimer une chaîne en ordre inverse.                  |
| `rot_13`          | Appliquer un décalage ROT13 sur une chaîne.             |
| `rotone`          | Appliquer un décalage de 1 sur une chaîne.              |
| `search_and_replace` | Remplacer tous les `a` par `b` dans une chaîne.     |
| `ulstr`           | Mettre une chaîne en minuscules.                        |

### Level 2 — manipulation avancée

| Fonction           | Description                                              |
|--------------------|---------------------------------------------------------|
| `alpha_mirror`     | Retourner `az..a` puis `z..a` (miroir alphabétique).     |
| `camel_to_snake`   | Convertir une chaîne camelCase en snake_case.           |
| `do_op`            | Appliquer un opérateur (`+ - * / %`) à deux entiers.     |
| `ft_atoi`          | Convertir une chaîne en entier.                         |
| `ft_strcmp`        | Comparer deux chaînes, retourner la différence.         |
| `ft_strcspn`       | Longueur du segment avant le premier caractère de `s2`. |
| `ft_strdup`        | Dupliquer une chaîne.                                   |
| `ft_strpbrk`       | Pointeur sur la première occurrence de `s2` dans `s1`. |
| `ft_strrev`        | Inverser une chaîne.                                    |
| `ft_strspn`        | Longueur du segment initial composé de caractères de `s2`. |
| `inter`            | Nombre de paires de mots égales entre deux tableaux.   |
| `last_word`        | Longueur du dernier mot d'une chaîne.                   |
| `is_power_of_2`    | Vérifier si un entier est une puissance de 2.           |
| `max`               | Retourner le plus grand entier d'une liste d'args.      |
| `print_bits`       | Imprimer les bits d'un entier.                          |
| `reverse_bits`     | Retourner un entier avec les bits inversés.             |
| `snake_to_camel`   | Convertir une chaîne snake_case en camelCase.          |
| `swap_bits`        | Échanger les bits pairs/impairs d'un entier.            |
| `union_words`      | Union de mots de deux tableaux (sans doublons).         |
| `wdmatch`          | Vérifier si un mot correspond à un motif `*`.           |

### Level 3 — algorithmes

| Fonction          | Description                                              |
|-------------------|---------------------------------------------------------|
| `add_prime_sum`   | Somme des premiers strictement inférieurs à chaque argument. |
| `epur_str`        | Réduire les espaces multiples en un seul.                |
| `expand_str`      | Développer `a3b` en `aaab`.                             |
| `ft_atoi_base`    | Convertir une chaîne en entier selon une base.         |
| `ft_list_size`    | Taille d'une liste chaînée.                              |
| `ft_range`        | Tableau des entiers `min..max-1`.                        |
| `ft_rrange`       | Tableau des entiers `max-1..min`.                        |
| `hidenp`          | Imprimer `s1` en masquant les caractères de `s2`.       |
| `lcm`             | Plus petit multiple commun.                             |
| `pgcd`            | Plus grand diviseur commun.                             |
| `paramsum`        | Somme des nombres positifs d'une chaîne.                |
| `print_hex`       | Imprimer un entier en hexadécimal.                      |
| `str_capitalizer` | Mettre la première lettre de chaque mot en majuscule.  |
| `rstr_capitalizer` | Mettre la dernière lettre de chaque mot en majuscule.   |
| `tab_mult`        | Tableau des multiples de `nb`.                          |

### Level 4 — mémoire & listes

| Fonction             | Description                                              |
|----------------------|---------------------------------------------------------|
| `flood_fill`         | Remplir une zone contiguë dans une grille.              |
| `fprime`             | Décomposition en facteurs premiers sous forme de chaîne. |
| `ft_itoa`            | Convertir un entier en chaîne.                          |
| `ft_list_foreach`    | Appliquer une fonction à chaque élément d'une liste.   |
| `ft_list_remove_if`  | Supprimer les nœuds correspondant à une condition.      |
| `ft_split`           | Découper une chaîne selon un séparateur.                |
| `rev_wstr`           | Inverser l'ordre des mots d'une chaîne.                 |
| `rostring`           | Faire pivoter une chaîne d'un cran à gauche.            |
| `sort_int_tab`       | Trier un tableau d'entiers (tri à bulles).              |
| `sort_list`          | Trier une liste chaînée.                                |

## Build

```bash
make          # compile le binaire exam_rank02 (cc -Wall -Wextra -Werror)
make clean
make fclean
make re
```

## Tests

Suite de tests C (4 suites, 111 tests) dans `tests/` :

```bash
make test                # compile et lance les 4 suites
# ou directement :
./tests/test_level_1
./tests/test_level_2
./tests/test_level_3
./tests/test_level_4
```

Le framework `tests/test_framework.h` fournit les macros `TEST_BEGIN`,
`TEST_SUITE`, `TEST_ASSERT`, `TEST_ASSERT_EQ`, `TEST_ASSERT_STREQ` et
`TEST_FINISH`.

## Notes

- Le projet utilise une **libft locale** (`libft/`) pour les fonctions
  utilitaires de base (`ft_strlen`, `ft_strdup`, `ft_atoi`, etc.).
- Les fonctions qui impriment (`fizzbuzz`, `repeat_alpha`, `rev_print`,
  `rot_13`, `rotone`, `ft_putstr`, `print_hex`, `hidenp`) écrivent sur
  `stdout` ; les fonctions qui transforment une chaîne le font **en place**
  (`rot_13`, `rotone`, `ulstr`, `search_and_replace`, `str_capitalizer`,
  `rstr_capitalizer`, `rostring`, `rev_wstr`, `epur_str`, `expand_str`).
- Les types `list_t` (liste chaînée simple) sont définis dans
  `include/exam_rank02.h`.
