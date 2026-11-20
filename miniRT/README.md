# miniRT

**miniRT** est un traceur de rayons (ray tracer) CPU en C, projet **Exam Rank 05** de la 42.
Il rend des scènes 3D définies dans des fichiers `.rt` et supporte l'éclairage
Phong, les ombres, la réflexion, la réfraction, les textures, les motifs
procéduraux, l'antialiasing et le multithreading.

## Fonctionnalités

### Obligatoires
- **Primitives** : sphère (`sp`), plan (`pl`), cylindre (`cy`)
- **Éclairage** : ambiant + directionnel unique, modèle **Phong** (diffuse + spéculaire)
- **Ombres** par lancer de rayon
- **Rendu** via miniLibX, caméra perspective paramétrable (position, orientation, fov)

### Bonus
- **Cône** (`co`) et **cube** (`cu`)
- **Triangle** (Möller-Trumbore)
- **Réflexion pure** (`re:`)
- **Transparence + réfraction** (Snell) avec indice de réfraction (`tr:`, `ri:`)
- **Textures** XPM (`texture:`)
- **Motifs procéduraux** : checker, stripe, gradient, ring (`pattern:`)
- **Antialiasing** par super-échantillonnage (`-a`)
- **Multithreading** du rendu (pthreads, `-t`)
- **Skybox** dégradé
- **Spéculaire** Phong paramétrable (`sp:`, `sh:`)

## Format de scène `.rt`

```
A <ratio> <R,G,B>                         ambiance
C <x,y,z> <ox,oy,oz> <fov>               caméra (position, orientation, fov)
L <x,y,z> <ratio> <R,G,B>                lumière directionnelle

sp <x,y,z> <diametre> <R,G,B> [tags]      sphère
pl <x,y,z> <nx,ny,nz> <R,G,B> [tags]      plan
cy <x,y,z> <ax,ay,az> <diametre> <h> <R,G,B> [tags]   cylindre
co <x,y,z> <ax,ay,az> <diametre> <h> <R,G,B> [tags]   cône
cu <x,y,z> <ax,ay,az> <taille> <R,G,B> [tags]         cube
```

**Tags bonus** (à la fin d'une ligne d'objet) :

| Tag | Signification |
|-----|---------------|
| `re:<ratio>` | ratio de réflexion |
| `tr:<ratio>` | ratio de transparence |
| `ri:<idx>` | indice de réfraction |
| `sp:<ratio>` | ratio spéculaire |
| `sh:<val>` | éclat (shininess) |
| `texture:<nom>` | texture XPM (`textures/<nom>.xpm`) |
| `pattern:<type>` | motif (`checker`, `stripe`, `gradient`, `ring`) |
| `pc1:<R,G,B>` / `pc2:<R,G,B>` | couleurs du motif |

Les couleurs sont en `0..255` (ou `0..1`).

## Utilisation

```bash
make            # build réel (nécessite miniLibX / X11)
make test       # build headless + suites de tests (shim miniLibX)
./miniRT [-b] [-t] [-a] [-w W] [-h H] <scene.rt>
```

Options :
- `-b` : activer les bonus
- `-t` : multithreading
- `-a` : antialiasing
- `-w` / `-h` : taille de la fenêtre

## Structure

```
include/minrt.h     header principal (types, constantes, API)
src/
  vector.c         vecteurs 3D (add, dot, cross, reflect, refract, ...)
  matrix.c         matrices 4x4 (Gauss-Jordan, Rodrigues, ...)
  utils.c          allocations + couleurs
  objects.c        constructeurs d'objets + transforms
  intersect.c      intersections rayon-objet + scene_intersect
  lighting.c       motifs, ombres, Phong, réflexion/réfraction, trace_ray
  camera.c         caméra perspective + génération de rayons
  render.c         rendu image, skybox, antialiasing
  mt.c             multithreading (pthreads)
  texture.c        chargement/échantillonnage XPM
  parser.c         lecteur .rt
  hooks.c          hooks MLX, mouvement caméra, boucle
  main.c           entrée du programme
libft/             libft 42
tests/             suites de tests headless + shim miniLibX
scenes/            scènes .rt exemples
textures/          textures XPM
```

## Tests

Le build réel nécessite miniLibX et un serveur X. Sur une machine sans écran,
la suite de tests utilise un **shim headless** de miniLibX (`tests/shim/`) qui
conserve l'API MLX et interprète les XPM.

```bash
make test
```

9 suites : `test_vector`, `test_matrix`, `test_objects`, `test_intersect`,
`test_lighting`, `test_camera`, `test_parser`, `test_render`, `test_integration`.

## Scènes exemples

- `scenes/basic.rt` — sphère, plan, cylindre (primitives obligatoires)
- `scenes/bonus.rt` — toutes les primitives + textures + motifs + optiques
