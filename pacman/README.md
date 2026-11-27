# 🎮 Pacman — Ghosts! More ghosts!

Projet 42 — jeu Pacman complet, implémenté en **Python 3 pur** (moteur
sans I/O + interface `curses`), accompagné d'une **suite de tests complète**.

## Structure

```
pacman/
├── subject.txt            # Énoncé complet du projet (partie + bonus)
├── pac-man.py             # Point d'entrée + interface curses (présentation)
├── requirements.txt       # pytest (le jeu utilise seulement la stdlib)
├── Makefile               # install / run / test / clean
├── src/
│   ├── maze.py            # Labyrinthe : génération non-parfaite + tunnel
│   ├── model.py           # Types purs : Direction, Position, Pacman, Ghost…
│   ├── ghosts.py          # IA fantômes : BFS, chasse/dispersion/peur/mort
│   ├── game.py            # Moteur de jeu (règles, score, vies, timer)
│   ├── config.py          # Parseur JSON robuste (10 niveaux par défaut)
│   ├── highscore.py       # Tableau des hauts scores (top 10, JSON)
│   └── config.json        # Configuration par défaut
└── tests/                 # Suite de tests (pytest)
    ├── test_maze.py
    ├── test_model.py
    ├── test_ghosts.py
    ├── test_game.py
    ├── test_config.py
    ├── test_highscore.py
    └── test_integration.py
```

## Démarrage

```bash
# Installer les dépendances de test
pip install -r requirements.txt

# Lancer le jeu
python3 pac-man.py src/config.json
#   ou : make run
```

Contrôles : **flèches directionnelles** ou **ZQSD/WASD** pour bouger,
**Espace / Échap** pour mettre en pause, **F1/f** pour le cheat (vies infinies).

## Tests

```bash
python3 -m pytest tests/ -v
#   ou : make test
```

La suite couvre : la génération/connexité/tunnel du labyrinthe, le modèle de
données, l'IA des fantômes (chasse, dispersion, peur, mort, cul-de-sac), le
moteur (collisions, score, vies, timer, respawn, fin de niveau), le parser de
config, la gestion des hauts scores, et des scénarios d'intégration
(progression multi-niveaux jusqu'à la victoire, tunnel en jeu, persistance du
score).

## Fonctionnalités

**Obligatoire** : labyrinthe généré (non-parfait, sans cul-de-sac), Pac-Man,
4 fantômes aux comportements distincts, pac-gums, score, vies, timer,
10 niveaux, tunnel gauche/droite, fin de partie / victoire, hauts scores
persistés (top 10 JSON).

**Bonus** : super pac-gum → fantômes effrayés (édible), fruit bonus,
vie bonus tous les 10 000 pts, mode cheat, mode mort/respawn des fantômes,
animation des états (prévu via l'interface curses).

## Implémentation

Le moteur (`src/game.py`) est **pur et sans I/O** : il ne fait aucune lecture
clavier ni écriture écran, ce qui le rend entièrement déterministe et testable.
`pac-man.py` est uniquement la couche de présentation `curses`.
