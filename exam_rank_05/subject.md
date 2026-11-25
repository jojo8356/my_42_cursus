# Exam Rank 05 — Sujets

Le **nouveau Exam Rank 05** (2025) comprend 5 projets : 3 en **C++98** (level 0) et
2 en **C** (level 1).

## Level 0 — C++98

### vect2
Créer une classe `vect2` représentant un vecteur mathématique de dimension 2
contenant des `int`.

- Addition, soustraction et multiplication par un scalaire
  (`vect2(2,2) * 2 == vect2(4,4)`).
- `operator[]` pour accéder aux composantes (0 et 1), sans vérification de bornes.
- Imprimable avec `<<` : `std::cout << v` produit `{v[0], v[1]}`.
- Fichiers attendus : `vect2.cpp`, `vect2.hpp`.

### bigint
Créer une classe `bigint` stockant un entier **non signé** de précision arbitraire.

- Addition, comparaison et **digitshift** (décalage de chiffres en base 10 :
  `42 << 3 == 42000`, `1337 >> 2 == 13`).
- Imprimable avec `<<` (base 10), sans zéros non significatifs en tête.
- Fichiers attendus : `bigint.hpp`, `bigint.cpp`.

### polyset
Classes fournies : `bag` (abstrait), `searchable_bag` (abstrait, hérite virtuellement
de `bag`), `array_bag`, `tree_bag`.

- **Première partie** : implémenter `searchable_array_bag` (hérite de `array_bag`)
  et `searchable_tree_bag` (hérite de `tree_bag`), qui implémentent
  `searchable_bag` (méthode `has(int) const`).
- **Deuxième partie** : implémenter `set` qui wrap un `searchable_bag` et en fait
  un ensemble (empêche les doublons).
- Forme canonique orthodoxe, `const`.
- Fichiers attendus : `searchable_array_bag.{hpp,cpp}`,
  `searchable_tree_bag.{hpp,cpp}`, `set.{hpp,cpp}`.

## Level 1 — C

### Life
Simuler le **jeu de la vie** de Conway.

- `./life width height iterations`
- La configuration initiale est dessinée via des commandes stdin :
  - `w a s d` : déplacer la plume (haut, gauche, bas, droite)
  - `x` : lever/baisser la plume (commencer/arrêter de dessiner)
- La plume commence en haut à gauche. À EOF, simuler `iterations` itérations.
- Sortie : `O` (vivant) / ` ` (mort). Les cellules hors tableau sont mortes.
- Fonctions autorisées : `atoi, read, putchar, malloc, calloc, realloc, free`.

### BSQ
Trouver le **plus grand carré** sur une carte, en évitant les obstacles.

- `./bsq fichier...` (ou stdin si aucun argument).
- Première ligne : `nombre_lignes caractère_vide caractère_obstacle caractère_plein`.
- Remplacer les cellules vides par plein pour former le plus grand carré.
- En cas d'égalité : le carré le plus en haut, puis le plus à gauche.
- Plusieurs cartes : une ligne vide entre chaque solution / erreur.
- Carte invalide → `map error` sur la sortie d'erreur.
- Fonctions autorisées : `malloc, calloc, realloc, free, fopen, fclose, getline,
  fscanf, fputs, fprintf, stderr, stdout, stdin, errno`.

## Bonus implémentés

- **vect2** : division par scalaire `/`, `/=`.
- **bigint** : soustraction, multiplication, division, modulo (et surcharges `int`).
- **polyset** : accesseurs `count()`, `empty()`.
- **Life** : gestion robuste des bords, allocation vérifiée.
- **BSQ** : validation complète, lecture stdin, plusieurs cartes, DP optimal.
