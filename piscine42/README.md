# Piscine 42 — Sujets, solutions normées et tests complets

Référence de travail pour la **Piscine C de l'école 42 (édition 2025-2026)** :
tous les sujets (PDF), une solution conforme à la Norme 42 pour chaque exercice,
et une batterie de tests automatisés.

> **Note importante** : la Piscine est conçue pour apprendre en faisant.
> Ces corrections sont une référence d'étude — comprenez chaque exercice
> par vous-même avant de regarder la solution, et respectez les règles
> d'intégrité académique de votre campus (les examens se passent sans aide).

## Contenu

| Dossier | Projet | Exercices |
|---|---|---|
| `subjects/pdf/` | Tous les sujets officiels (PDF) | 34 fichiers |
| `subjects/` | Fiches résumées (rédigées en français) | 1 par projet |
| `shell00/` | Shell 00 | ex00 → ex09 |
| `shell01/` | Shell 01 | ex01 → ex08 |
| `c00/` → `c13/` | Jours de C | ~130 fonctions/programmes |
| `rush00/` | Rush 00 — rectangle ASCII | ex00 |
| `rush01/` | Rush 01 — Skyscrapers 4x4 | ex00 |
| `rush02/` | Rush 02 — nombres en toutes lettres | ex00 |
| `bsq/` | BSQ — plus grand carré | projet final |
| `exam00/` → `exam03/` | Sujets d'examens (rank 1 à 4) | ~57 exercices |
| `tests/` | Tests automatisés (drivers + runner) | — |

## Organisation de chaque exercice (jours C)

```
c00/ex07/ft_putnbr.c     ← le livrable (seul fichier à rendre)
tests/c00_test.c         ← les tests (jamais à rendre)
tests/test_c00.sh        ← driver : compile + exécute + compare
```

Les dossiers `exNN/` ne contiennent **que** les fichiers à rendre : vous pouvez
les zipper tels quels pour la moulinette. Tout ce qui est test vit dans `tests/`.

## Lancer tous les tests

```sh
./run_all_tests.sh            # tout
./run_all_tests.sh c05        # un seul jour
./run_all_tests.sh bsq rush01 # quelques projets
```

Chaque test affiche `PASS`/`FAIL` ; le script sort avec le nombre d'échecs.

## Style de code

Les solutions respectent la **Norme 42 v3** (indentation par tabulations,
80 colonnes max, 25 lignes par fonction, 5 fonctions par fichier, pas de
boucle `for`, pas d'opérateur ternaire, etc.) avec des commentaires
pédagogiques en français.

## Sujets PDF

Les PDF de `subjects/pdf/` proviennent de miroirs publics GitHub de sujets de
piscine (éditions 2025-2026 : dépôts `andrei-bozhek/42-Piscine` et
`Imposter-zx/1337-Subject`), et de `voiddeveloper/Piscine` pour les sujets Shell.
`subjects/README.md` donne le détail, fichier par fichier, avec des résumés
reformulés de chaque exercice.
