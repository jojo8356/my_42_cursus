*FDF — Wireframe 3D terrain renderer (42, Rank 02, Graphic branch)*

---

## English

### Overview

**FDF** is a wireframe 3D terrain renderer that reads height-map data from `.fdf` files and renders it as an isometric wireframe using MiniLibX. The project is part of the 42 curriculum (Rank 02, Graphic branch).

### Features

**Mandatory:**
- Read and parse `.fdf` height-map files (grid of integer heights)
- Render terrain as wireframe using MiniLibX
- Isometric projection (30° / 45° angles)
- Color gradient based on terrain height (min → max)
- 5 color modes: gradient, grayscale, blue, cyan, rainbow
- Keyboard controls (WASD movement, arrows rotation, scroll zoom, C color cycle, R reset, ESC quit)

**Bonus:**
- 3D rotation around X, Y, and Z axes
- Smooth pan/translate in 3D space
- Zoom in/out with clamping (0.1 – 10.0)
- Multiple color palettes (gradient, grayscale, blue, cyan, rainbow)
- HSL → RGB rainbow color mapping
- RGB lerp interpolation for smooth gradients
- Info display (file name, grid size, z range, scale)
- Mouse scroll wheel zoom
- Robust file parsing with error handling (ragged grids rejected, empty files rejected)

### File Format

`.fdf` files contain a grid of integer heights, one row per line, space-separated:

```
0 0 0
1 2 0
0 0 1
```

Each row must have the same number of values (square or rectangular grids).

### Controls

| Key | Action |
|-----|--------|
| W / S | Pan up / down |
| A / D | Pan left / right |
| Arrow Left / Right | Rotate around Y axis |
| Arrow Up / Down | Rotate around X axis |
| Scroll wheel | Zoom in / out |
| C | Cycle color mode |
| R | Reset view |
| ESC | Quit |

### Build

```bash
cd ~/my_42_cursus/fdf
ln -s ../../mlx mlx  # if not already linked
make
```

### Run

```bash
./fdf tests/maps/basic.fdf
./fdf tests/maps/peak.fdf
./fdf tests/maps/stair.fdf
```

### Test Maps

- `tests/maps/basic.fdf` — 3×3 flat terrain
- `tests/maps/flat.fdf` — flat plane
- `tests/maps/stair.fdf` — staircase pattern
- `tests/maps/peak.fdf` — central peak
- `tests/maps/ragged.fdf` — ragged grid (should be rejected)
- `tests/maps/empty.fdf` — empty file (should be rejected)
- `tests/maps/single.fdf` — single point

### Tests

Headless unit tests (no X11 required):

```bash
./tests/run_tests.sh
```

5 test suites, 248 tests total:

| Suite | Tests | Coverage |
|-------|-------|----------|
| test_parse | 73 | File parsing, grid validation, min/max z |
| test_math | 52 | Rotation matrices, isometric projection, transform pipeline |
| test_draw | 48 | Bresenham lines, put_pixel, draw_grid |
| test_color | 39 | RGB lerp, 5 color modes, rainbow HSL→RGB |
| test_hooks | 36 | Zoom clamping, translate, rotate |

### Architecture

```
fdf/
├── includes/
│   └── fdf.h              # Central header: types, constants, prototypes
├── src/
│   ├── main.c             # Entry point, mlx init, event loop
│   ├── parse.c            # .fdf file parsing, grid validation
│   ├── math.c             # Rotation matrices, isometric projection, transform pipeline
│   ├── color.c            # RGB lerp, 5 color modes, rainbow HSL→RGB
│   ├── draw.c             # Bresenham lines, put_pixel, draw_grid
│   ├── render.c           # MiniLibX rendering pipeline
│   └── hooks.c            # Event handlers (keyboard, mouse)
├── tests/
│   ├── test_framework.h   # Lightweight test framework
│   ├── test_parse.c       # Parsing tests
│   ├── test_math.c        # Math/projection tests
│   ├── test_draw.c        # Drawing tests
│   ├── test_color.c       # Color tests
│   ├── test_hooks.c       # Event handler logic tests
│   ├── test_mlx_stub.c    # MiniLibX stubs for headless testing
│   ├── run_tests.sh       # Test orchestrator
│   └── maps/              # Test map files
├── Makefile
└── README.md
```

### Key Design Decisions

1. **Separate pure logic from mlx** — `math.c`, `color.c`, `parse.c`, `draw.c` have no mlx dependencies, enabling headless testing.
2. **Isometric projection** — standard simplified formula: `sx = (x-y)*cos(30°)*scale`, `sy = (x+y)*sin(30°)*scale - z*scale`.
3. **Color modes** — 5 modes cycling with 'C' key: gradient (red→green), grayscale (white→black), blue (blue→white), cyan (cyan→white), rainbow (HSL→RGB).
4. **Robust parsing** — rejects ragged grids, empty files, non-integer values; clamps to `MAX_GRID_SIZE` (1000×1000).
5. **Zoom clamping** — scale bounded to [0.1, 10.0] to prevent degenerate views.

---

## Français

### Présentation

**FDF** est un rendu 3D fil de fer d'un terrain qui lit des données de carte de hauteur depuis des fichiers `.fdf` et les affiche en fil de fer isométrique via MiniLibX. Le projet fait partie du cursus 42 (Rank 02, filière Graphique).

### Fonctionnalités

**Obligatoires :**
- Lire et analyser des fichiers `.fdf` (grille de hauteurs entières)
- Afficher le terrain en fil de fer avec MiniLibX
- Projection isométrique (angles 30° / 45°)
- Dégradé de couleur selon la hauteur du terrain (min → max)
- 5 modes de couleur : gradient, niveaux de gris, bleu, cyan, rainbow
- Contrôles clavier (WASD déplacement, flèches rotation, molette zoom, C cycle couleur, R reset, ESC quit)

**Bonus :**
- Rotation 3D autour des axes X, Y et Z
- Translation fluide dans l'espace 3D
- Zoom avant/arrière avec bornage (0.1 – 10.0)
- Palettes de couleurs multiples (gradient, gris, bleu, cyan, rainbow)
- Conversion HSL → RVB pour le rainbow
- Interpolation RVB (lerp) pour des dégradés fluides
- Affichage d'infos (nom du fichier, taille de la grille, plage z, échelle)
- Zoom par molette de la souris
- Analyse robuste avec gestion d'erreurs (grilles inégales rejetées, fichiers vides rejetés)

### Format de fichier

Les fichiers `.fdf` contiennent une grille de hauteurs entières, une ligne par ligne, séparées par des espaces :

```
0 0 0
1 2 0
0 0 1
```

Chaque ligne doit avoir le même nombre de valeurs (grilles carrées ou rectangulaires).

### Contrôles

| Touche | Action |
|--------|--------|
| W / S | Déplacer haut / bas |
| A / D | Déplacer gauche / droite |
| Flèche Gauche / Droite | Rotation autour de Y |
| Flèche Haut / Bas | Rotation autour de X |
| Molette | Zoom avant / arrière |
| C | Changer le mode de couleur |
| R | Réinitialiser la vue |
| ESC | Quitter |

### Compilation

```bash
cd ~/my_42_cursus/fdf
ln -s ../../mlx mlx  # si pas déjà lié
make
```

### Exécution

```bash
./fdf tests/maps/basic.fdf
./fdf tests/maps/peak.fdf
./fdf tests/maps/stair.fdf
```

### Cartes de test

- `tests/maps/basic.fdf` — terrain plat 3×3
- `tests/maps/flat.fdf` — plan plat
- `tests/maps/stair.fdf` — motif en escalier
- `tests/maps/peak.fdf` — pic central
- `tests/maps/ragged.fdf` — grille inégale (doit être rejetée)
- `tests/maps/empty.fdf` — fichier vide (doit être rejeté)
- `tests/maps/single.fdf` — point unique

### Tests

Tests unitaires sans GUI (sans X11 requis) :

```bash
./tests/run_tests.sh
```

5 suites de tests, 248 tests au total :

| Suite | Tests | Couverture |
|-------|-------|-----------|
| test_parse | 73 | Analyse de fichiers, validation de grille, min/max z |
| test_math | 52 | Matrices de rotation, projection isométrique, pipeline de transformation |
| test_draw | 48 | Lignes de Bresenham, put_pixel, draw_grid |
| test_color | 39 | Lerp RVB, 5 modes de couleur, rainbow HSL→RVB |
| test_hooks | 36 | Bortage du zoom, translation, rotation |

### Architecture

```
fdf/
├── includes/
│   └── fdf.h              # Header central : types, constantes, prototypes
├── src/
│   ├── main.c             # Point d'entrée, init mlx, boucle d'événements
│   ├── parse.c            # Analyse de fichiers .fdf, validation de grille
│   ├── math.c             # Matrices de rotation, projection isométrique, pipeline
│   ├── color.c            # Lerp RVB, 5 modes de couleur, rainbow HSL→RVB
│   ├── draw.c             # Lignes de Bresenham, put_pixel, draw_grid
│   ├── render.c           # Pipeline de rendu MiniLibX
│   └── hooks.c            # Gestionnaires d'événements (clavier, souris)
├── tests/
│   ├── test_framework.h   # Framework de test léger
│   ├── test_parse.c       # Tests d'analyse
│   ├── test_math.c        # Tests math/projection
│   ├── test_draw.c        # Tests de dessin
│   ├── test_color.c       # Tests de couleur
│   ├── test_hooks.c       # Tests de logique d'événements
│   ├── test_mlx_stub.c    # Stubs MiniLibX pour tests sans GUI
│   ├── run_tests.sh       # Orchestrateur de tests
│   └── maps/              # Fichiers de cartes de test
├── Makefile
└── README.md
```

### Décisions de conception clés

1. **Séparation logique pure et mlx** — `math.c`, `color.c`, `parse.c`, `draw.c` n'ont aucune dépendance mlx, permettant des tests sans GUI.
2. **Projection isométrique** — formule simplifiée standard : `sx = (x-y)*cos(30°)*scale`, `sy = (x+y)*sin(30°)*scale - z*scale`.
3. **Modes de couleur** — 5 modes cyclés avec la touche 'C' : gradient (rouge→vert), gris (blanc→noir), bleu (bleu→blanc), cyan (cyan→blanc), rainbow (HSL→RVB).
4. **Analyse robuste** — rejette les grilles inégales, les fichiers vides, les valeurs non entières ; bornage à `MAX_GRID_SIZE` (1000×1000).
5. **Bornage du zoom** — échelle bornée entre [0.1, 10.0] pour éviter les vues dégénérées.
