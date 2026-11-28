# 🎮 Pacman — Ghosts! More ghosts!

Projet 42 — jeu Pacman en **Python 3 pur** (moteur sans I/O + interface
`curses`), accompagné d'une suite de tests couvrant le moteur et le parcours
interactif.

## Structure

```
pacman/
├── subject.txt            # Énoncé complet du projet (partie + bonus)
├── pac-man.py             # Point d'entrée + interface curses
├── requirements.txt       # pytest
├── requirements-dev.txt   # pytest, Flake8 et mypy
├── mypy.ini               # Vérification stricte des types
├── Makefile               # install / run / test / lint / typecheck
├── src/
│   ├── maze.py            # Labyrinthe : génération + tunnel
│   ├── model.py           # Types purs : Direction, Position, Pacman, Ghost…
│   ├── ghosts.py          # IA des fantômes et recherche de chemins
│   ├── game.py            # Règles, score, vies, timers et transitions
│   ├── config.py          # Parseur JSON et validation de configuration
│   ├── highscore.py       # Tableau des scores (top 10, JSON)
│   └── config.json        # Configuration par défaut
└── tests/                 # Suite pytest, dont les tests curses non bloquants
```

## Démarrage

```bash
# Dépendance minimale pour jouer
pip install -r requirements.txt

# Lancer le jeu
python3 pac-man.py src/config.json
#   ou : make run
```

Pour installer également les outils de développement :

```bash
pip install -r requirements-dev.txt
```

Contrôles : **flèches**, **WASD** ou **ZQSD** pour bouger,
**Espace / Échap** pour mettre en pause, **F1/f** pour le cheat.

## Tests et contrôles statiques

```bash
python3 -m pytest tests/ -v
python3 -m pytest tests/ --cov=src --cov=pacman_cli --cov-report=term-missing
python3 -m flake8 .
python3 -m mypy
#   ou : make check / make coverage
```

Les tests couvrent la génération, la connexité et le tunnel du labyrinthe,
les comportements chase/scatter/frightened/dead et les personnalités de
Blinky, Pinky, Inky et Clyde, les règles de score et de vies, la validation
stricte des configurations et scores, la progression réelle entre niveaux,
la conservation du score et des vies, l'interface curses contrôlée par des
entrées simulées, ainsi que l'enregistrement et le rechargement d'un score
final via le parcours de `main()`.

## Fonctionnalités

Labyrinthe généré, Pac-Man, quatre fantômes aux comportements distincts,
pac-gums, score, vies, timer, niveaux configurables, tunnel gauche/droite,
fin de partie et victoire, hauts scores persistés (top 10 JSON).

Super pac-gum, fruit bonus, vie bonus tous les 10 000 points, mode cheat,
retour à la base et réapparition des fantômes effrayés.

Le moteur (`src/game.py`) est pur et sans I/O. `pac-man.py` gère la couche de
présentation `curses`; sa boucle de jeu utilise des entrées non bloquantes et
peut être testée avec un écran simulé.
