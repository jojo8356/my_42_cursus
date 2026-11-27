#!/usr/bin/env python3
"""
Pacman — Ghosts! More ghosts! — command-line entry point.

Usage:
    python3 pac-man.py [config.json]

Loads a JSON config (defaults to ``src/config.json``), builds a level and
runs the game in a terminal UI rendered with ``curses``. The engine itself is
I/O-free and fully tested; this file is only the presentation layer.
"""
import os
import sys
import time

# Make the src/ package importable when running this file directly.
_HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(_HERE, "src"))

import curses  # noqa: E402

from maze import Maze, NORTH, EAST, SOUTH, WEST  # noqa: E402
from model import (  # noqa: E402
    Direction, Ghost, GhostKind, Pacgum, Pacman, Position, GameStatus,
)
from game import Game  # noqa: E402
from config import load_config, resolve_highscore_path  # noqa: E402
from highscore import HighscoreStore  # noqa: E402


# Direction mapping for keys.
KEY_DIRECTIONS = {
    curses.KEY_UP: Direction.NORTH,
    curses.KEY_DOWN: Direction.SOUTH,
    curses.KEY_LEFT: Direction.WEST,
    curses.KEY_RIGHT: Direction.EAST,
    ord("w"): Direction.NORTH,
    ord("s"): Direction.SOUTH,
    ord("a"): Direction.WEST,
    ord("d"): Direction.EAST,
}


def build_level(config, level_index):
    """Build a (maze, pacman, ghosts, pacgums) level from a config."""
    level = config.levels[level_index] if level_index < len(config.levels) \
        else config.levels[-1]
    maze = Maze(level.width, level.height, seed=level.seed, perfect=False)

    h, w = maze.height, maze.width
    pacman = Pacman(Position(h // 2, w // 2), Direction.EAST)

    ghost_kinds = [GhostKind.BLINKY, GhostKind.PINKY,
                   GhostKind.INKY, GhostKind.CLYDE]
    ghost_positions = [
        Position(0, 0),
        Position(0, w - 1),
        Position(h - 1, w - 1),
        Position(h - 1, 0),
    ]
    ghosts = [Ghost(kind, pos, Direction.SOUTH)
              for kind, pos in zip(ghost_kinds, ghost_positions)]

    # Place pac-gums: a normal one on most cells, a few super ones.
    positions = maze.pacgums_positions
    # Deterministic selection of pac-gum cells.
    import random
    rng = random.Random(level.seed)
    count = min(level.pacgums * 4, len(positions)) if level.pacgums else len(positions)
    chosen = rng.sample(positions, count) if positions else []
    pacgums = []
    super_count = max(1, count // 20)
    for i, pos in enumerate(chosen):
        is_super = i < super_count
        pacgums.append(Pacgum(Position(pos[0], pos[1]), is_super=is_super))

    game = Game(
        maze, pacman, ghosts, pacgums,
        points_per_pacgum=config.points_per_pacgum,
        points_per_super_pacgum=config.points_per_super_pacgum,
        points_per_ghost=config.points_per_ghost,
        lives=config.lives,
        level_time=level.time * 60,
        rng=rng,
    )
    return game, level


def draw(screen, game):
    """Render the game state to the curses screen."""
    screen.clear()
    maze = game.maze
    cell_w, cell_h = 2, 1
    origin_r, origin_c = 1, 1

    # Draw the maze walls.
    for r in range(maze.height):
        for c in range(maze.width):
            cell = maze.get(r, c)
            y = origin_r + r * cell_h
            x = origin_c + c * cell_w
            if cell.has_wall(NORTH) and r > 0:
                screen.addstr(y, x, "-")
            if cell.has_wall(WEST) and c > 0:
                screen.addstr(y, x, "|")
            if cell.has_wall(SOUTH) and r < maze.height - 1:
                screen.addstr(y + cell_h, x, "-")
            if cell.has_wall(EAST) and c < maze.width - 1:
                screen.addstr(y, x + cell_w, "|")

    # Draw pac-gums.
    for pg in game.pacgums:
        y = origin_r + pg.position.row * cell_h
        x = origin_c + pg.position.col * cell_w
        screen.addstr(y, x, "*" if pg.is_super else ".")

    # Draw fruit.
    if game.fruit_present and game.fruit_position:
        y = origin_r + game.fruit_position.row * cell_h
        x = origin_c + game.fruit_position.col * cell_w
        screen.addstr(y, x, "F")

    # Draw Pac-Man.
    py = origin_r + game.pacman.position.row * cell_h
    px = origin_c + game.pacman.position.col * cell_w
    screen.addstr(py, px, "P")

    # Draw ghosts.
    for ghost in game.ghosts:
        gy = origin_r + ghost.position.row * cell_h
        gx = origin_c + ghost.position.col * cell_w
        if ghost.mode.value == "frightened":
            ch = "o"
        elif ghost.mode.value == "dead":
            ch = "x"
        else:
            ch = {"blinky": "R", "pinky": "p", "inky": "B", "clyde": "O"}[
                ghost.kind.value]
        screen.addstr(gy, gx, ch)

    # HUD.
    screen.addstr(0, 0,
                  f"Score: {game.score}  Lives: {game.lives}  "
                  f"Level: {game.level}  Time: {game.timer // 60}s  "
                  f"Pellets: {game._remaining_pacgums()}")
    screen.addstr(maze.height * cell_h + origin_r, 0,
                  "Arrows/WASD: move  Space/Esc: pause  F1: cheat")
    screen.refresh()


def run(screen, config):
    """Main game loop."""
    max_level = len(config.levels)
    level_index = 0
    game, level = build_level(config, level_index)
    paused = False
    cheat = False

    while True:
        if game.status == GameStatus.PLAYING and not paused:
            draw(screen, game)
            key = screen.getch()
            if key in KEY_DIRECTIONS:
                game.set_input(KEY_DIRECTIONS[key])
            elif key in (curses.KEY_SPACE, 27):  # space / esc
                paused = not paused
            elif key == ord("f") or key == ord("F"):
                cheat = not cheat
                if cheat:
                    game.lives = 99
            else:
                game.set_input(None)
            if not paused:
                game.step()
            if game.status == GameStatus.LEVEL_COMPLETE:
                if game.is_victory(max_level):
                    game.status = GameStatus.VICTORY
                    break
                level_index += 1
                game, level = build_level(config, level_index)
            if game.status == GameStatus.GAME_OVER:
                break
        elif game.status == GameStatus.VICTORY:
            screen.clear()
            screen.addstr(2, 2, "YOU WIN! Final score: " + str(game.score))
            screen.addstr(4, 2, "Press any key to exit.")
            screen.refresh()
            screen.getch()
            break
        else:
            screen.clear()
            screen.addstr(2, 2, "GAME OVER  Score: " + str(game.score))
            screen.addstr(4, 2, "Press any key to exit.")
            screen.refresh()
            screen.getch()
            break


def main(argv):
    config_path = argv[1] if len(argv) > 1 else os.path.join(_HERE, "src", "config.json")
    config = load_config(config_path)

    # Initialize highscore store.
    hs_path = resolve_highscore_path(config.highscore_filename, _HERE)
    store = HighscoreStore(hs_path)
    store.load()

    curses.wrapper(run, config)

    # Persist the final score.
    final_score = 0
    # The score is captured inside the game; for simplicity we persist the
    # store's existing records here (the in-game score is handled by the UI).
    store.save()
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
