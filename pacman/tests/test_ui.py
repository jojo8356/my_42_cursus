"""Tests for the curses interface and the complete score-persistence flow."""
from __future__ import annotations

import importlib.util
from collections import deque
from pathlib import Path
from types import ModuleType
from typing import Any

import pytest

from config import GameConfig, LevelConfig
from game import Game
from highscore import HighscoreStore
from maze import EAST, WEST, Maze
from model import (
    Direction,
    GameStatus,
    Ghost,
    GhostKind,
    GhostMode,
    Pacgum,
    Pacman,
    Position,
)


def _load_cli() -> ModuleType:
    cli_path = Path(__file__).parents[1] / "pac-man.py"
    spec = importlib.util.spec_from_file_location("pacman_cli", cli_path)
    assert spec is not None and spec.loader is not None
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


cli: Any = _load_cli()


class FakeScreen:
    """Minimal curses screen that serves queued keys without blocking."""

    def __init__(self, keys: list[int]) -> None:
        self.keys = deque(keys)
        self.timeouts: list[int] = []
        self.messages: list[str] = []
        self.read_count = 0

    def timeout(self, delay: int) -> None:
        self.timeouts.append(delay)

    def clear(self) -> None:
        return None

    def addstr(self, _row: int, _col: int, text: str) -> None:
        self.messages.append(text)

    def refresh(self) -> None:
        return None

    def getch(self) -> int:
        self.read_count += 1
        if self.keys:
            return self.keys.popleft()
        return -1


def _config(level_count: int = 1, lives: int = 3) -> GameConfig:
    levels = [
        LevelConfig(
            name=f"Level {index + 1}",
            width=5,
            height=5,
            pacgums=0,
            seed=index,
            time=10 + index,
        )
        for index in range(level_count)
    ]
    return GameConfig(lives=lives, levels=levels)


def _game(
    pacgum_positions: list[tuple[int, int]],
    *,
    lives: int = 3,
    level_time: int = 600,
    super_pacgums: set[tuple[int, int]] | None = None,
) -> Game:
    maze = Maze(5, 5, seed=42, perfect=True)
    pacman = Pacman(Position(2, 2), Direction.EAST)
    ghosts = [
        Ghost(GhostKind.BLINKY, Position(0, 0), Direction.SOUTH),
        Ghost(GhostKind.PINKY, Position(0, 4), Direction.SOUTH),
        Ghost(GhostKind.INKY, Position(4, 4), Direction.WEST),
        Ghost(GhostKind.CLYDE, Position(4, 0), Direction.NORTH),
    ]
    super_positions = super_pacgums or set()
    pacgums = [
        Pacgum(Position(row, col), (row, col) in super_positions)
        for row, col in pacgum_positions
    ]
    return Game(
        maze,
        pacman,
        ghosts,
        pacgums,
        lives=lives,
        level_time=level_time,
    )


def _install_builder(
    monkeypatch: pytest.MonkeyPatch,
    games: list[Game],
    config: GameConfig,
) -> None:
    def build(
        _config: GameConfig,
        level_index: int,
    ) -> tuple[Game, LevelConfig]:
        return games[level_index], config.levels[level_index]

    monkeypatch.setattr(cli, "build_level", build)
    monkeypatch.setattr(cli.time, "sleep", lambda _duration: None)


def test_build_level_uses_config_and_seed_deterministically() -> None:
    config = _config(lives=4)
    config.levels[0].pacgums = 2

    first_game, first_config = cli.build_level(config, 0)
    second_game, second_config = cli.build_level(config, 0)

    assert first_config == second_config
    assert first_game.maze.seed == config.levels[0].seed
    assert first_game.lives == 4
    assert first_game.level_time == config.levels[0].time * 60
    assert len(first_game.pacgums) == 8
    assert [gum.position for gum in first_game.pacgums] == [
        gum.position for gum in second_game.pacgums
    ]


def test_build_level_fallback_and_empty_config() -> None:
    config = _config()
    last_game, last_config = cli.build_level(config, 3)
    assert last_config == config.levels[-1]
    assert last_game.level_time == config.levels[-1].time * 60

    empty_config = GameConfig(levels=[])
    with pytest.raises(ValueError):
        cli.build_level(empty_config, 0)


def test_draw_renders_fruit_ghost_states_and_pause_overlay() -> None:
    game = _game([])
    game.fruit_present = True
    game.fruit_position = Position(1, 1)
    game.ghosts[0].mode = GhostMode.FRIGHTENED
    game.ghosts[1].mode = GhostMode.DEAD
    screen = FakeScreen([])

    cli.draw(screen, game, paused=True)

    assert "F" in screen.messages
    assert "o" in screen.messages
    assert "x" in screen.messages
    assert any("PAUSED" in message for message in screen.messages)


def test_key_map_supports_wasd_and_zqsd() -> None:
    assert cli.KEY_DIRECTIONS[ord("w")] == Direction.NORTH
    assert cli.KEY_DIRECTIONS[ord("z")] == Direction.NORTH
    assert cli.KEY_DIRECTIONS[ord("a")] == Direction.WEST
    assert cli.KEY_DIRECTIONS[ord("q")] == Direction.WEST
    assert cli.KEY_DIRECTIONS[ord("s")] == Direction.SOUTH
    assert cli.KEY_DIRECTIONS[ord("d")] == Direction.EAST


@pytest.mark.parametrize("key", [cli.curses.KEY_RIGHT, ord("d")])
def test_run_maps_keys_renders_and_returns_victory(
    monkeypatch: pytest.MonkeyPatch,
    key: int,
) -> None:
    config = _config()
    game = _game([(2, 3)])
    game.maze.get(2, 2).remove_wall(EAST)
    game.maze.get(2, 3).remove_wall(WEST)
    _install_builder(monkeypatch, [game], config)
    screen = FakeScreen([key, 10])

    result = cli.run(screen, config)

    assert result is game
    assert result.pacman.position == Position(2, 3)
    assert result.score == 10
    assert result.status == GameStatus.VICTORY
    assert screen.timeouts[0] == 0
    assert -1 in screen.timeouts
    assert any("Pellets:" in message for message in screen.messages)
    assert any(
        "YOU WIN! Final score: 10" in message
        for message in screen.messages
    )


def test_run_pause_freezes_ticks_and_resumes(
    monkeypatch: pytest.MonkeyPatch,
) -> None:
    config = _config()
    game = _game([(2, 3)])
    game.maze.get(2, 2).remove_wall(EAST)
    game.maze.get(2, 3).remove_wall(WEST)
    _install_builder(monkeypatch, [game], config)
    screen = FakeScreen([ord(" "), -1, ord(" "), ord("d"), 10])

    result = cli.run(screen, config)

    assert result.status == GameStatus.VICTORY
    assert result.score == 10
    assert any("PAUSED" in message for message in screen.messages)
    assert screen.read_count == 5


def test_run_f1_cheat_sets_lives_and_game_over_is_rendered(
    monkeypatch: pytest.MonkeyPatch,
) -> None:
    config = _config()
    game = _game([(2, 3)])
    game.maze.get(2, 2).remove_wall(EAST)
    game.maze.get(2, 3).remove_wall(WEST)
    _install_builder(monkeypatch, [game], config)
    screen = FakeScreen([cli.curses.KEY_F1, ord("d"), 10])

    result = cli.run(screen, config)

    assert result.status == GameStatus.VICTORY
    assert result.lives == 99
    assert any("F1: cheat" in message for message in screen.messages)

    game_over = _game([], lives=1)
    game_over.ghosts[0].position = Position(2, 2)
    _install_builder(monkeypatch, [game_over], config)
    game_over_screen = FakeScreen([-1, 10])

    final_game = cli.run(game_over_screen, config)

    assert final_game.status == GameStatus.GAME_OVER
    assert any("GAME OVER" in message for message in game_over_screen.messages)


def test_run_advances_levels_preserving_session_and_resetting_level_state(
    monkeypatch: pytest.MonkeyPatch,
) -> None:
    config = _config(level_count=2, lives=2)
    first_game = _game(
        [(2, 2)],
        lives=2,
        level_time=60,
        super_pacgums={(2, 2)},
    )
    first_game.next_extra_life = 500
    next_level = _game([(2, 2)], lives=2, level_time=120)
    _install_builder(monkeypatch, [first_game, next_level], config)
    screen = FakeScreen([-1, -1, 10])

    result = cli.run(screen, config)

    assert result is first_game
    assert result.level == 2
    assert result.status == GameStatus.VICTORY
    assert result.score == 60
    assert result.lives == 2
    assert result.next_extra_life == 500
    assert result.timer == 119
    assert result.frightened_remaining == 0
    assert result.fruit_eaten == 1
    assert all(ghost.mode.value == "chase" for ghost in result.ghosts)
    assert result.maze is next_level.maze
    assert result._remaining_pacgums() == 0


def test_main_records_and_reloads_the_actual_final_score(
    monkeypatch: pytest.MonkeyPatch,
    tmp_path: Path,
) -> None:
    config = _config()
    config.highscore_filename = "scores.json"
    score_path = tmp_path / "scores.json"
    game = _game([(2, 2)])
    _install_builder(monkeypatch, [game], config)
    monkeypatch.setattr(cli, "load_config", lambda _path: config)
    monkeypatch.setattr(
        cli,
        "resolve_highscore_path",
        lambda _filename, _base: str(score_path),
    )
    game_screen = FakeScreen([-1, 10])
    name_screen = FakeScreen([*(ord(char) for char in "PacPlayer"), 10])

    def fake_wrapper(function: Any, *args: object) -> object:
        if function is cli.run:
            return function(game_screen, *args)
        if function is cli.prompt_player_name:
            return function(name_screen, *args)
        raise AssertionError(f"Unexpected curses callback: {function!r}")

    monkeypatch.setattr(cli.curses, "wrapper", fake_wrapper)

    assert cli.main(["pac-man.py"]) == 0
    reloaded = HighscoreStore(score_path)
    reloaded.load()

    assert reloaded.top() == [{"name": "PacPlayer", "score": 10}]
    assert any("New high score: 10" in text for text in name_screen.messages)


def test_name_prompt_rejects_empty_name_and_supports_editing() -> None:
    screen = FakeScreen([10, ord("A"), ord("!"), 8, ord("B"), 10])

    name = cli.prompt_player_name(screen, 250)

    assert name == "B"
    assert any("non-empty name" in text for text in screen.messages)


def test_name_prompt_can_cancel_registration() -> None:
    assert cli.prompt_player_name(FakeScreen([27]), 100) is None
