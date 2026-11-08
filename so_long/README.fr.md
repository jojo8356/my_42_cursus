*This project has been created as part of the 42 curriculum by jojo8356.*

# so_long

Un petit jeu 2D : le joueur ramasse tous les collectibles de la carte, puis
rejoint la sortie. Écrit en C avec la miniLibX, conformément à la Norme.

`English` — voir [README.md](README.md).

## Sommaire

- [Le sujet](#le-sujet)
- [Ce que fait le programme](#ce-que-fait-le-programme)
- [Compilation](#compilation)
- [Exécution](#exécution)
- [Format de carte](#format-de-carte)
- [Bonus](#bonus)
- [Tests](#tests)
- [Notes de conception](#notes-de-conception)
- [Structure du projet](#structure-du-projet)
- [Ressources](#ressources)

## Le sujet

Le sujet officiel est livré dans [`docs/`](docs/) :

| Fichier | Contenu |
|---|---|
| [`docs/en.subject.pdf`](docs/en.subject.pdf) | le PDF officiel, tel que distribué par 42 |
| [`docs/en.subject.md`](docs/en.subject.md) | transcription anglaise, pour la recherche |
| [`docs/fr.subject.md`](docs/fr.subject.md) | transcription française |

Les règles auxquelles l'implémentation est tenue sont, en résumé :

- le programme prend une carte `.ber` comme unique argument ;
- la carte utilise cinq symboles : `0` vide, `1` mur, `C` collectible, `E`
  sortie, `P` départ du joueur ;
- elle doit être rectangulaire, fermée par des murs, et contenir au moins un `C`,
  exactement un `E` et exactement un `P` ;
- le joueur se déplace avec `W`, `A`, `S` et `D`, ne peut pas entrer dans un mur,
  doit ramasser tous les `C` avant que la sortie ne soit utilisable, et gagne en
  atteignant `E` ;
- le nombre courant de mouvements est affiché dans le shell à chaque mouvement ;
- `ESC` et la croix rouge de la fenêtre ferment la fenêtre et quittent
  proprement ;
- toute carte malformée produit `Error` suivi d'un message explicite.

À noter que le sujet n'exige explicitement **pas** de vérifier qu'un chemin
existe entre le joueur et la sortie — et cette implémentation ne l'impose pas
non plus, car refuser une carte que le sujet déclare valide serait un bug.

## Ce que fait le programme

- Charge et valide la carte **avant** de toucher à la bibliothèque graphique,
  ainsi une carte invalide ne fait jamais clignoter une fenêtre.
- Dessine chaque tuile depuis un XPM 64×64, puis les acteurs par-dessus.
- Déplace le joueur sur les quatre axes, en refusant les murs et en refusant la
  sortie tant qu'il reste des collectibles.
- Affiche le compteur de mouvements dans le shell (`1`, `2`, `3`, … une par
  ligne).
- Bascule la sortie sur son sprite allumé une fois le dernier collectible
  ramassé, comme signal visuel que le niveau peut être quitté.
- Libère tout à la sortie : la carte, les images, la fenêtre et le display.

## Compilation

```sh
make            # binaire obligatoire : ./so_long
make bonus      # binaire bonus :      ./so_long_bonus
make re         # fclean + all
make clean      # objets
make fclean     # objets + binaires
```

La miniLibX est attendue comme bibliothèque système (`-lmlx -lXext -lX11 -lm`),
c'est ainsi qu'elle est installée sur les clusters 42. Pour la compiler depuis
ses sources, pointer la variable `MLX_DIR` vers un dépôt et ajouter le chemin
d'inclusion :

```sh
make MLX_INC="-I mlx" MLX_LIB="mlx/libmlx.a -lXext -lX11 -lm"
```

Compiler le binaire graphique exige les en-têtes de développement X11
(`libx11-dev`, `libxext-dev`) — voir [Tests](#tests) pour ce qui se passe sans.

## Exécution

```sh
./so_long maps/valid/basic.ber
./so_long_bonus maps/valid/enemies.ber
```

| Touche | Action |
|---|---|
| `W` | haut |
| `A` | gauche |
| `S` | bas |
| `D` | droite |
| `ESC` | quitter |

Une carte d'exemple est fournie dans [`maps/`](maps/).

## Format de carte

```
1111111
1P00C01
1011011
1000CE1
1111111
```

| Symbole | Sens |
|---|---|
| `0` | sol vide |
| `1` | mur |
| `C` | collectible |
| `E` | sortie (exactement une) |
| `P` | départ du joueur (exactement un) |
| `X` | ennemi — **bonus uniquement** |

Le binaire obligatoire refuse `X` : ce n'est pas l'un des cinq symboles du
chapitre IV. Le binaire bonus l'accepte, et c'est la seule différence de parsing.

Chaque rejet imprime `Error` sur la première ligne et une raison précise sur la
seconde, par exemple :

```
Error
maps/invalid/ragged.ber: map is not rectangular
```

Le message va sur **stdout** : le chapitre IV ne nomme aucun flux, et le testeur
communautaire (voir plus bas) décide qu'une carte a été refusée en faisant
passer le programme dans `grep Error`, qui lit stdout.

## Bonus

Les trois bonus du chapitre V sont implémentés, dans des fichiers
`_bonus.{c,h}` et derrière la règle `bonus` séparée du Makefile, comme l'exige le
chapitre III.

**1. Patrouilles d'ennemis.** Chaque `X` de la carte devient un ennemi qui fait
les cent pas sur sa ligne, en faisant demi-tour contre les murs. Le contact fait
perdre la partie, dans les deux sens : le joueur qui marche sur un ennemi, et
l'ennemi qui marche sur le joueur.

**2. Animation de sprites.** Le joueur a deux frames, l'ennemi aussi ; la frame
avance au rythme de la boucle de jeu, donc les sprites continuent de bouger même
quand le joueur reste immobile. La boucle dessine sur un timer et pas seulement
sur une pression de touche, c'est ce qui rend l'animation visible.

**3. Compteur de mouvements à l'écran.** Le nombre de mouvements est dessiné en
haut à gauche de la fenêtre avec `mlx_string_put`. Le compteur shell reste aussi,
car c'est une exigence obligatoire — le bonus est un ajout, pas un remplacement.

## Tests

```sh
bash run_tests.sh              # compile, puis toutes les couches
bash run_tests.sh game         # une seule couche
bash run_tests.sh --no-build   # réutilise les binaires précédents
```

La suite compte six couches, 106 vérifications au total, plus les 44
vérifications d'un testeur tiers :

| Couche | Vérifs | Contenu |
|---|---:|---|
| `parse` | 38 | les dix règles de carte du chapitre IV, plus les 41 cartes du corpus |
| `game` | 16 | mouvements, murs, condition de sortie, victoire, `ESC`, rendu, hooks |
| `bonus` | 18 | ennemis, animation, compteur à l'écran, règles toujours appliquées |
| `memory` | 12 | ASan + UBSan sur chaque carte : aucune fuite, aucun crash |
| `compliance` | 22 | règles de Norme, Makefile, assets, sujet, READMEs |
| `community` | 44 | `augustobecker/so_long_tester`, exécuté sans modification |

### Le problème du headless

Un projet graphique est difficile à tester sur une machine sans écran, et c'est
exactement la situation en CI. La suite le résout sans modifier une seule ligne
du jeu :

- [`tests/shim/mlx.h`](tests/shim/mlx.h) et
  [`tests/shim/mlx_shim.c`](tests/shim/mlx_shim.c) implémentent l'API de la
  miniLibX fidèlement — mêmes prototypes, même sémantique — et journalisent
  chaque appel dans `$MLX_SHIM_LOG` au lieu de dessiner.
- `tests/build.sh` compile les **vraies** sources contre ce shim, avec les mêmes
  `-Wall -Wextra -Werror`, et lit sa liste de sources depuis le `Makefile` pour
  que le build de test ne puisse jamais diverger du build réel.
- Deux variables d'environnement transforment la boucle interactive en
  exécution scriptée : `MLX_SHIM_FRAMES` (nombre de frames) et `MLX_SHIM_KEYS`
  (quelles touches envoyer, une par frame).

Cela permet de jouer une partie gagnante complète, de perdre contre un ennemi,
et de vérifier quel sprite a été blitté où — ce qu'une capture d'écran ne dirait
pas de façon fiable. Ce que la suite ne peut pas faire, c'est vérifier les pixels
eux-mêmes : elle prouve que la bonne image est allée sur la bonne tuile, pas que
la tuile est jolie.

Sur une machine avec X11, `make && make bonus` construit les vrais binaires, et
cette suite exerce le même code avec un calepin à la place d'un écran.

### Le testeur tiers

[`augustobecker/so_long_tester`](https://github.com/augustobecker/so_long_tester)
est conservé verbatim dans
[`tests/community/augustobecker/`](tests/community/augustobecker/), avec ses 13
scripts de contrôle et ses 41 cartes. `tests/cases/60-community.sh` le place dans
un répertoire temporaire (ses scripts attendent le binaire un niveau au-dessus de
`tests/`) et lance les treize contrôles : **44 [OK], 0 [KO]**.

Deux manques de l'environnement sont contournés, et le lanceur l'annonce :

- `valgrind` n'est pas installable ici, donc `tests/tools/valgrind` est un
  substitut qui exécute la commande en ignorant les options de valgrind. Il ne
  vérifie **pas** la mémoire. Ce travail est fait pour de vrai par la couche
  `memory`, qui lance AddressSanitizer et UBSan sur le même corpus de 41 cartes.
- Le binaire est celui du build headless, faute de serveur X.

### Règle de conception des tests

Chaque nombre de mouvements attendu et chaque rejet attendu est dérivé de
[`tests/tools/reference_model.py`](tests/tools/reference_model.py), une
implémentation indépendante des règles du chapitre IV. On modifie une attente en
lançant ce modèle, jamais en ajustant le nombre jusqu'à ce que le test passe au
vert.

## Notes de conception

**La grille n'est jamais modifiée.** La position du joueur vit dans l'état du jeu,
pas dans la carte. L'alternative — écrire `'0'` sur la tuile que le joueur
quitte — fonctionne pour les collectibles mais détruit la tuile de sortie la
première fois que le joueur marche dessus, ce qui est le bug classique de
`so_long`. Garder la grille intacte signifie que la sortie est toujours un `E` à
la fin, et que le rendu est une fonction pure de l'état du jeu.

**Le bonus n'est pas un fork.** Les binaires obligatoire et bonus partagent leur
parsing, leur validation, leurs hooks et leur nettoyage ; les différences sont
derrière trois petits fichiers (`bonus_charset`, `bonus_frame`,
`enemy_bonus`). Un bonus qui réimplémente le jeu est un bonus avec ses propres
bugs, et le chapitre V ne note le bonus que si la partie obligatoire est
parfaite.

**La validation a lieu avant l'ouverture de la fenêtre.** Une carte refusée ne
crée jamais de connexion X, ce que la suite vérifie directement : une carte
invalide ne doit produire aucun `mlx_init` dans le journal du shim.

## Structure du projet

```
so_long/
├── assets/          neuf sprites XPM 64×64, deux frames par entité animée
├── docs/            le sujet : PDF plus transcriptions anglaise et française
├── include/         so_long.h
├── libft/           libft vendue, compilée par son propre Makefile
├── maps/            cartes d'exemple
├── src/             sources de la partie obligatoire
│   └── bonus/       les trois bonus, en fichiers _bonus.c
├── tests/
│   ├── build.sh          build headless des deux binaires
│   ├── cases/            les six couches de test
│   ├── community/        augustobecker/so_long_tester, verbatim
│   ├── shim/             le substitut de la miniLibX
│   └── tools/            modèle de référence, substitut de valgrind
├── Makefile
└── run_tests.sh
```

## Ressources

- [42Paris/minilibx-linux](https://github.com/42Paris/minilibx-linux) — les
  sources de la miniLibX, et la référence de son API.
- [augustobecker/so_long_tester](https://github.com/augustobecker/so_long_tester)
  — le testeur communautaire utilisé par la couche `community`.
- Les sprites de `assets/` ont été générés pour ce projet (pixel art plat,
  64×64), et ne proviennent pas d'un tiers : il n'y a donc aucune attribution à
  porter.

### Usage de l'IA

Une assistance par IA a été utilisée pour écrire ce projet et sa suite de tests.
Concrètement : l'organisation des sources et les couches de test ont été
ébauchées avec un assistant, puis vérifiées en les exécutant — chaque chiffre
rapporté ci-dessus vient d'une exécution réelle de `bash run_tests.sh`, pas de la
description de l'assistant. Le sujet lui-même a été transcrit depuis le PDF
officiel. Les décisions de conception, et les erreurs qui subsistent, sont celles
de l'auteur.
