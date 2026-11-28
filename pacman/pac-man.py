#!/usr/bin/env python3
"""
Pacman — Ghosts! More ghosts! — command-line entry point.

Usage:
    python3 pac-man.py [config.json]

Loads a JSON config (defaults to ``src/config.json``), builds each level and
runs the game in a terminal UI rendered with ``curses``.
"""
from __future__ import annotations

import curses
import os
import random
import sys
import time
from collections.abc import Sequence

# Make the src/ package importable when running this file directly.
_HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(_HERE, "src"))

from config import (  # noqa: E402
    GameConfig,
    LevelConfig,
    load_config,
    resolve_highscore_path,
)
from game import Game  # noqa: E402
from highscore import (  # noqa: E402
    NAME_MAX,
    HighscoreStore,
    validate_name,
)
from maze import Maze, NORTH, EAST, SOUTH, WEST  # noqa: E402
from model import (  # noqa: E402
    Direction,
    GameStatus,
    Ghost,
    GhostKind,
    Pacgum,
    Pacman,
    Position,
)

# Direction mapping for the arrow keys and WASD.
KEY_DIRECTIONS: dict[int, Direction] = {
    curses.KEY_UP: Direction.NORTH,
    curses.KEY_DOWN: Direction.SOUTH,
    curses.KEY_LEFT: Direction.WEST,
    curses.KEY_RIGHT: Direction.EAST,
    ord("w"): Direction.NORTH,
    ord("z"): Direction.NORTH,
    ord("s"): Direction.SOUTH,
    ord("a"): Direction.WEST,
    ord("q"): Direction.WEST,
    ord("d"): Direction.EAST,
}


def build_level(
    config: GameConfig,
    level_index: int,
) -> tuple[Game, LevelConfig]:
    """Build a maze, its actors and collectibles for one configured level."""
    if not config.levels:
        raise ValueError("at least one level must be configured")
    if level_index < len(config.levels):
        level = config.levels[level_index]
    else:
        level = config.levels[-1]
    maze = Maze(level.width, level.height, seed=level.seed, perfect=False)

    height, width = maze.height, maze.width
    pacman = Pacman(Position(height // 2, width // 2), Direction.EAST)
    ghost_kinds = [
        GhostKind.BLINKY,
        GhostKind.PINKY,
        GhostKind.INKY,
        GhostKind.CLYDE,
    ]
    ghost_positions = [
        Position(0, 0),
        Position(0, width - 1),
        Position(height - 1, width - 1),
        Position(height - 1, 0),
    ]
    ghosts = [
        Ghost(kind, position, Direction.SOUTH)
        for kind, position in zip(ghost_kinds, ghost_positions)
    ]

    rng = random.Random(level.seed)
    possible_positions = maze.pacgums_positions
    count = min(level.pacgums * 4, len(possible_positions))
    chosen = rng.sample(possible_positions, count) if count else []
    super_count = max(1, count // 20) if count else 0
    pacgums = [
        Pacgum(Position(row, col), is_super=index < super_count)
        for index, (row, col) in enumerate(chosen)
    ]

    game = Game(
        maze,
        pacman,
        ghosts,
        pacgums,
        points_per_pacgum=config.points_per_pacgum,
        points_per_super_pacgum=config.points_per_super_pacgum,
        points_per_ghost=config.points_per_ghost,
        lives=config.lives,
        level_time=level.time * 60,
        rng=rng,
    )
    return game, level


def draw(screen: curses.window, game: Game, paused: bool = False) -> None:
    """Render the current game state to the curses screen."""
    screen.clear()
    maze = game.maze
    cell_width, cell_height = 2, 1
    origin_row, origin_col = 1, 1

    for row in range(maze.height):
        for col in range(maze.width):
            cell = maze.get(row, col)
            y = origin_row + row * cell_height
            x = origin_col + col * cell_width
            if cell.has_wall(NORTH) and row > 0:
                screen.addstr(y, x, "-")
            if cell.has_wall(WEST) and col > 0:
                screen.addstr(y, x, "|")
            if cell.has_wall(SOUTH) and row < maze.height - 1:
                screen.addstr(y + cell_height, x, "-")
            if cell.has_wall(EAST) and col < maze.width - 1:
                screen.addstr(y, x + cell_width, "|")

    for pacgum in game.pacgums:
        y = origin_row + pacgum.position.row * cell_height
        x = origin_col + pacgum.position.col * cell_width
        screen.addstr(y, x, "*" if pacgum.is_super else ".")

    if game.fruit_present and game.fruit_position is not None:
        y = origin_row + game.fruit_position.row * cell_height
        x = origin_col + game.fruit_position.col * cell_width
        screen.addstr(y, x, "F")

    player_y = origin_row + game.pacman.position.row * cell_height
    player_x = origin_col + game.pacman.position.col * cell_width
    screen.addstr(player_y, player_x, "P")

    ghost_symbols = {
        GhostKind.BLINKY: "R",
        GhostKind.PINKY: "p",
        GhostKind.INKY: "B",
        GhostKind.CLYDE: "O",
    }
    for ghost in game.ghosts:
        ghost_y = origin_row + ghost.position.row * cell_height
        ghost_x = origin_col + ghost.position.col * cell_width
        if ghost.mode.value == "frightened":
            symbol = "o"
        elif ghost.mode.value == "dead":
            symbol = "x"
        else:
            symbol = ghost_symbols[ghost.kind]
        screen.addstr(ghost_y, ghost_x, symbol)

    screen.addstr(
        0,
        0,
        f"Score: {game.score}  Lives: {game.lives}  "
        f"Level: {game.level}  Time: {game.timer // 60}s  "
        f"Pellets: {game._remaining_pacgums()}",
    )
    screen.addstr(
        maze.height * cell_height + origin_row,
        0,
        "Arrows/WASD: move  Space/Esc: pause  F1: cheat",
    )
    if paused:
        screen.addstr(maze.height // 2, 0, "PAUSED — press Space or Esc")
    screen.refresh()


def _show_final_screen(screen: curses.window, game: Game) -> None:
    """Show the final result and wait for one acknowledgement key."""
    screen.clear()
    if game.status == GameStatus.VICTORY:
        message = f"YOU WIN! Final score: {game.score}"
    else:
        message = f"GAME OVER  Score: {game.score}"
    screen.addstr(2, 2, message)
    screen.addstr(4, 2, "Press any key to exit.")
    screen.refresh()
    screen.timeout(-1)
    screen.getch()


def run(screen: curses.window, config: GameConfig) -> Game:
    """Run the curses game loop and return the finished game session."""
    max_level = len(config.levels)
    level_index = 0
    game, _level = build_level(config, level_index)
    paused = False
    cheat_enabled = False

    # A zero timeout makes getch non-blocking. The sleep sets the tick rate.
    screen.timeout(0)
    while game.status == GameStatus.PLAYING:
        draw(screen, game, paused=paused)
        key = screen.getch()

        if key in KEY_DIRECTIONS:
            game.set_input(KEY_DIRECTIONS[key])
        elif key in (ord(" "), 27):
            paused = not paused
        elif key in (curses.KEY_F1, ord("f"), ord("F")):
            cheat_enabled = not cheat_enabled
            if cheat_enabled:
                game.lives = 99

        if not paused:
            game.step()

        current_status = GameStatus(game.status.value)
        if current_status == GameStatus.LEVEL_COMPLETE:
            if game.is_victory(max_level):
                game.status = GameStatus.VICTORY
            else:
                level_index += 1
                next_game, _level = build_level(config, level_index)
                game.start_next_level(next_game)

        current_status = GameStatus(game.status.value)
        if current_status in (GameStatus.GAME_OVER, GameStatus.VICTORY):
            break
        time.sleep(1 / 60)

    _show_final_screen(screen, game)
    return game


def prompt_player_name(screen: curses.window, score: int) -> str | None:
    """Read a validated high-score name; Escape cancels registration."""
    name = ""
    message = "Enter a name (1-10 letters, digits or spaces):"
    screen.timeout(-1)
    while True:
        screen.clear()
        screen.addstr(2, 2, f"New high score: {score}")
        screen.addstr(4, 2, f"Name: {name}")
        screen.addstr(6, 2, message)
        screen.addstr(8, 2, "Press Enter to save or Esc to skip.")
        screen.refresh()

        key = screen.getch()
        if key == 27:
            return None
        if key in (10, 13):
            try:
                return validate_name(name)
            except ValueError:
                message = (
                    "Please enter a non-empty name of at most 10 characters."
                )
                continue
        if key in (8, 127, getattr(curses, "KEY_BACKSPACE", 263)):
            name = name[:-1]
        elif (
            32 <= key <= 126
            and len(name) < NAME_MAX
            and (key == ord(" ") or chr(key).isalnum())
        ):
            name += chr(key)


def main(argv: Sequence[str]) -> int:
    """Load configuration, run the game and persist a new high score."""
    config_path = (
        argv[1]
        if len(argv) > 1
        else os.path.join(_HERE, "src", "config.json")
    )
    config = load_config(config_path)

    score_path = resolve_highscore_path(config.highscore_filename, _HERE)
    store = HighscoreStore(score_path)
    store.load()

    final_game = curses.wrapper(run, config)
    if store.is_new_record(final_game.score):
        player_name = curses.wrapper(prompt_player_name, final_game.score)
        if player_name is not None:
            store.add(player_name, final_game.score)
            store.save()
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
