*A graphical 2D fractal renderer built in C using MiniLibX.*

# 🌌 Fract'ol

**Fract'ol** is a computer graphics project in the 42 curriculum. It creates a high-performance fractal renderer by translating screen coordinates into the complex plane and computing the escape trajectory of mathematical sequences ($Z = Z^2 + C$) to draw infinitely zoomable representations of the Mandelbrot, Julia, and many bonus fractals.

---

## Features

### Mandatory
- **Mandelbrot** set
- **Julia** set (with adjustable constant)
- **Mouse-wheel zoom** (infinite, follows cursor)
- **Multi-color** rendering (6 palettes + grayscale)

### Bonus (all implemented)
- **Burning Ship** fractal
- **Tricorn** (Mandelbar) fractal
- **Phoenix** fractal (with adjustable δ)
- **Phoenix variant** (Julia-style Phoenix)
- **Multibrot** fractal (power 2–10)
- **Arrow-key panning** (move view in the complex plane)
- **Cursor-following zoom**
- **Color palette cycling** (`h` / `c`)
- **Grayscale toggle** (`g`)
- **Iteration count adjustment** (`q` / `w`)
- **Reset view** (`r`)
- **Multi-threaded rendering** (up to 8 threads)
- **1077 automated tests** across 6 suites

## Controls

| Action | Key |
|--------|-----|
| Zoom in / out | Mouse scroll wheel |
| Zoom in / out (centered) | `+` / `-` |
| Move view | ▲ ▼ ◄ ► |
| Cycle color palette | `h` or `c` |
| Toggle grayscale | `g` |
| Increase / decrease iterations | `w` / `q` |
| Reset view | `r` |
| Quit | `ESC` or close window |

## Compilation

```bash
make
```

### Shortcuts

| Command | Action |
|---------|--------|
| `make m` | Launch Mandelbrot |
| `make j` | Launch Julia (-0.8, 0.156) |
| `make s` | Launch Burning Ship |
| `make t` | Launch Tricorn |
| `make p` | Launch Phoenix (0.5667, 0.0, -0.5) |
| `make mb` | Launch Multibrot (power 3) |

### Usage

```bash
./fractol mandelbrot
./fractol julia <real> <imag>
./fractol burning_ship
./fractol tricorn
./fractol phoenix <real> <imag> <delta>
./fractol "phoenix variant" <real> <imag> <delta>
./fractol multibrot [power]
```

## Tests

```bash
make test
```

1077 tests across 6 suites:

| Suite | Tests | Coverage |
|-------|-------|----------|
| `test_math` | 40 | All 7 fractal types, recurrence verification, edge cases |
| `test_color` | 29 | Palette mapping, color shift, grayscale, bounds checking |
| `test_transform` | 29 | Coordinate transforms, zoom-at, clamping, aspect ratio |
| `test_args` | 58 | Name resolution, argument parsing, error handling |
| `test_render` | 914 | Per-pixel rendering, determinism, deep zoom, full image |
| `test_thread` | 7 | Multithreaded determinism, multi-fractal threading |

All tests run headless (no X11 required).

## Architecture

```
fract-ol/
├── includes/
│   └── fract-ol.h          # Central header: types, constants, prototypes
├── src/
│   ├── main.c              # Entry point
│   ├── args.c              # Argument parsing & fractal name resolution
│   ├── init.c              # miniLibX initialisation
│   ├── hooks.c             # Event handlers (keyboard, mouse)
│   ├── render.c            # mlx-dependent rendering (render_image, redraw)
│   ├── render_core.c       # mlx-independent rendering core (testable)
│   ├── fractal_math.c      # Pure fractal iteration (testable)
│   ├── color.c             # Palettes & color mapping (testable)
│   └── transform.c         # Coordinate transforms (testable)
├── tests/
│   ├── test_framework.h    # Minimal assert framework
│   ├── test_math.c         # Fractal math correctness
│   ├── test_color.c        # Color computation
│   ├── test_transform.c    # Coordinate transforms
│   ├── test_args.c         # Argument parsing
│   ├── test_render.c       # Rendering pipeline
│   ├── test_thread.c       # Multithreading
│   └── run_tests.sh        # Test orchestrator
├── Makefile
└── README.md
```

### Key Design Decisions

- **`t_fractal` struct**: Single source of truth, passed by reference everywhere (Norm: ≤ 4 parameters, no globals)
- **Separation of concerns**: Math/color/transform logic is completely independent of miniLibX, enabling headless testing
- **Multithreading**: Image split into horizontal stripes, each rendered by one pthread. Local struct copy per thread avoids race conditions on `z_r`/`z_i`/`c_r`/`c_i`
- **Procedural palettes**: All 6 palettes generated at runtime from HSL→RGB, avoiding large static arrays

---

## README.fr.md

*Un renderer de fractales 2D graphique en C utilisant MiniLibX.*

# 🌌 Fract'ol

**Fract'ol** est un projet de computer graphics du cursus 42. Il crée un renderer de fractales performant en traduisant les coordonnées de l'écran dans le plan complexe et en calculant la trajectoire d'échappement des suites mathématiques ($Z = Z^2 + C$) pour dessiner des représentations zoomables à l'infini des fractales de Mandelbrot, Julia et de nombreuses fractales bonus.

## Caractéristiques

### Obligatoire
- **Mandelbrot**
- **Julia** (constante ajustable)
- **Zoom** à la souris (infini, suit le curseur)
- **Multi-couleurs** (6 palettes + niveaux de gris)

### Bonus (tout implémenté)
- **Burning Ship**
- **Tricorn** (Mandelbar)
- **Phoenix** (δ ajustable)
- **Phoenix variant** (style Julia)
- **Multibrot** (puissance 2–10)
- **Déplacement** avec les flèches
- **Zoom au curseur**
- **Cyclage des palettes** (`h` / `c`)
- **Mode gris** (`g`)
- **Ajustement d'itérations** (`q` / `w`)
- **Reset** (`r`)
- **Rendu multi-threadé** (jusqu'à 8 threads)
- **1077 tests automatisés** sur 6 suites

## Contrôles

| Action | Clé |
|--------|-----|
| Zoom avant / arrière | Roulette souris |
| Zoom centré | `+` / `-` |
| Déplacement | ▲ ▼ ◄ ► |
| Changer palette | `h` ou `c` |
| Mode gris | `g` |
| Itérations + / - | `w` / `q` |
| Reset | `r` |
| Quitter | `ESC` ou fermeture |

## Compilation

```bash
make
```

### Utilisation

```bash
./fractol mandelbrot
./fractol julia <real> <imag>
./fractol burning_ship
./fractol tricorn
./fractol phoenix <real> <imag> <delta>
./fractol "phoenix variant" <real> <imag> <delta>
./fractol multibrot [power]
```

## Tests

```bash
make test
```

1077 tests sur 6 suites (sans X11 requis) :

| Suite | Tests | Couverture |
|-------|-------|------------|
| `test_math` | 40 | 7 fractales, récurrences, cas limites |
| `test_color` | 29 | Palettes, décalage, gris, bornes |
| `test_transform` | 29 | Transformes, zoom, clamp, aspect |
| `test_args` | 58 | Noms, parsing, erreurs |
| `test_render` | 914 | Rendu pixel, déterminisme, zoom profond |
| `test_thread` | 7 | Déterminisme multi-thread |

## Architecture

Le code est séparé en deux couches :
- **Couche pure** (testable sans X11) : `fractal_math.c`, `color.c`, `transform.c`, `render_core.c`
- **Couche mlx** (requiert X11) : `main.c`, `init.c`, `hooks.c`, `render.c`

Cette séparation permet de tester 1077 cas sans avoir besoin d'un écran, tout en garantissant que la logique mathématique est correcte.
