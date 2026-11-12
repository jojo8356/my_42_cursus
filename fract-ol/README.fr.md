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
