# Cub3D

> **Projet Rank 04 — Raycasting (Wolfenstein 3D)**  
> 42 School · Implementation complète **avec bonus**.

Cub3D est un moteur de **raycasting 3D** (effet Wolfenstein 3D) écrit en C,
utilisant **miniLibX42** pour le rendu. Le projet parse une carte `.cub`,
lance les rayons depuis la position du joueur, et affiche un mur texture,
un sol/plafond colorés, ainsi que les éléments du **bonus** (portes
animées, sprites animés, minimap, contrôle souris).

---

## Fonctionnalités

### Mandatory
- **Parser `.cub`** : textures `NO/SO/WE/EA`, couleurs `F`/`C` (RGB),
  carte `0/1/N/S/E/W` (carte toujours en dernière position).
- **Validation stricte** : textures obligatoires, joueur unique,
  carte fermée (flood-fill), couleurs RGB valides.
- **Raycasting DDA** avec correction de fish-eye (distance perpendiculaire).
- **Rendu texture murale** par côté (N/S/E/W).
- **Mouvement & rotation** du joueur avec collision murs.
- **Orientation** selon `N/S/E/W` (direction + plan).

### Bonus
- **Portes `D`** : animation d'ouverture/fermeture, ouverture à la touche `E`,
  bloquent le passage quand fermées.
- **Sprites `P` (fixe) et `A` (animé)** : rendu avec profondeur
  (tri décroissant), animation d'alternance des frames `A1`/`A2`.
- **Minimap** (touche `M`).
- **Sol/plafond texturés** (textures bonus).
- **Contrôle souris** (rotation).

---

## Structure

```
cub3d/
├── include/cub3d.h       — structures, constantes, prototypes
├── src/
│   ├── main.c             — point d'entrée
│   ├── parser.c           — parsing .cub + validation
│   ├── textures.c         — chargement XPM (NO/SO/WE/EA/DO/SP/A1/A2)
│   ├── player.c           — joueur (déplacement, rotation, collisions)
│   ├── raycasting.c       — DDA + resolution des murs
│   ├── render.c           — rendu murs, sol, sprites, portes, minimap
│   ├── hooks.c           — boucles de jeu, clavier, gestion fenêtre
│   └── utils.c            — allocations (xmalloc/xcalloc/xrealloc)
├── textures/              — textures XPM 16x16
├── maps/
│   ├── test.cub           — carte mandatory (joueur N)
│   ├── bonus.cub          — carte bonus (porte D, sprites P/A)
│   └── invalid_no_textures.cub — carte invalide (tests)
├── tests/
│   ├── test_framework.h   — framework d'assertions
│   ├── run_tests.sh       — orchestrateur des tests headless
│   ├── test_parser.c      — parsing & validation
│   ├── test_raycasting.c  — DDA & distances
│   ├── test_player.c      — mouvement & collisions
│   ├── test_textures.c    — chargement XPM
│   ├── test_doors.c       — portes (bonus)
│   ├── test_sprites.c     — sprites (bonus)
│   ├── test_integration.c — lancement complet
│   └── test_robustness.c  — cas limites, stress
└── Makefile
```

---

## Compilation

Le build **réel** utilise **miniLibX42** (nécessite X11) :

```bash
make            # → cub3D
make bonus      # → lien symbolique cub3D_bonus
make re         # rebuild complet
```

> **Note :** dans un environnement sans X11/miniLibX (CI, sandbox),
> le lien réel échoue. Utilisez `make test` pour le **build headless**
> via le shim miniLibX.

## Utilisation

```bash
./cub3D maps/test.cub        # mandatory
./cub3D_bonus maps/bonus.cub # bonus (ou ./cub3D maps/bonus.cub)
```

Le bonus est activé par détection du nom du binaire (`bonus` dans `argv[0]`)
ou en passant une carte contenant des `D`/`P`/`A`.

---

## Tests

```bash
make test
```

**8 suites headless, 188 tests, tous passant** (via le shim miniLibX
`tests/shim/mlx_shim.c` qui lit les XPM réels et log les opérations) :

| Suite | Tests | Couverture |
|-------|------:|------------|
| test_parser | 45 | parsing .cub, validation, couleurs, portes, sprites |
| test_raycasting | 20 | DDA, distances perpendiculaires, cast_rays |
| test_player | 18 | initialisation, rotation, déplacement, collisions |
| test_textures | 24 | chargement XPM, textures bonus, libération |
| test_doors | 15 | collecte, ouverture/fermeture, blocage |
| test_sprites | 13 | collecte, types, tri, animation |
| test_integration | 17 | game_init, rendu, touches, destruction |
| test_robustness | 26 | cartes 3x3/21x21, couleurs limites, stress |

**Total : 188 tests, 100 % réussis.**

---

## Format de carte `.cub`

```
NO textures/north.xpm
SO textures/south.xpm
WE textures/west.xpm
EA textures/east.xpm
# bonus : DO textures/door.xpm / SP textures/sprite.xpm / A1 / A2
F 100,120,80        # couleur sol (R,G,B)
C 30,30,60          # couleur plafond

1111111            # carte (toujours en dernière position)
1N00001
1000001
1111111
```

- `0` espace, `1` mur, `N/S/E/W` joueur (orientation),
- `D` porte (bonus, doit être entourée de 2 murs),
- `P` sprite fixe / `A` sprite animé (bonus).
