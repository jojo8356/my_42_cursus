"""Engine-level integration tests for play, collisions and persistence."""
import random
from pathlib import Path

from game import Game
from highscore import HighscoreStore
from maze import EAST, NORTH, SOUTH, WEST, Maze
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


def _make_game(
    width: int = 10,
    height: int = 10,
    seed: int = 42,
    lives: int = 3,
    level_time: int = 100000,
) -> Game:
    maze = Maze(width, height, seed=seed, perfect=False)
    pacman = Pacman(Position(height // 2, width // 2), Direction.EAST)
    ghosts = [
        Ghost(GhostKind.BLINKY, Position(0, 0), Direction.SOUTH),
        Ghost(GhostKind.PINKY, Position(0, width - 1), Direction.SOUTH),
        Ghost(GhostKind.INKY, Position(height - 1, width - 1), Direction.WEST),
        Ghost(GhostKind.CLYDE, Position(height - 1, 0), Direction.NORTH),
    ]
    pacgums = [
        Pacgum(Position(row, col))
        for row, col in maze.pacgums_positions
    ]
    return Game(
        maze,
        pacman,
        ghosts,
        pacgums,
        lives=lives,
        level_time=level_time,
        rng=random.Random(seed),
    )


def test_full_game_clears_a_level_and_reaches_victory() -> None:
    game = _make_game()
    for ghost in game.ghosts:
        ghost.mode = GhostMode.SCATTER
    for pacgum in list(game.pacgums):
        game.pacman.position = Position(
            pacgum.position.row,
            pacgum.position.col,
        )
        game.eat_pacgum_at(
            game.pacman.position.row,
            game.pacman.position.col,
        )
    game.step()

    assert game.status == GameStatus.LEVEL_COMPLETE
    assert game.is_victory(1) is True


def test_tunnel_movement_in_game() -> None:
    game = _make_game(width=8, height=8, seed=1)
    game.pacman.position = Position(4, 0)
    game.pacman.direction = Direction.WEST
    game.set_input(Direction.WEST)
    game.step()

    assert game.pacman.position.col == game.maze.width - 1
    assert game.pacman.position.row == 4


def test_score_can_be_added_and_reloaded_from_store(tmp_path: Path) -> None:
    game = _make_game()
    for ghost in game.ghosts:
        ghost.mode = GhostMode.SCATTER
    for pacgum in list(game.pacgums):
        game.pacman.position = Position(
            pacgum.position.row,
            pacgum.position.col,
        )
        game.eat_pacgum_at(
            game.pacman.position.row,
            game.pacman.position.col,
        )
    final_score = game.score

    path = tmp_path / "hs.json"
    store = HighscoreStore(path)
    store.load()
    assert store.is_new_record(final_score) is True
    store.add("PacPlayer", final_score)
    store.save()

    reloaded = HighscoreStore(path)
    reloaded.load()
    assert reloaded.top()[0] == {"name": "PacPlayer", "score": final_score}


def test_frightened_ghost_is_eaten_returns_home_and_revives() -> None:
    game = _make_game()
    ghost = game.ghosts[0]
    ghost.mode = GhostMode.FRIGHTENED
    game.pacman.position = Position(ghost.position.row, ghost.position.col)
    game.check_collisions()

    assert ghost.mode == GhostMode.DEAD
    assert ghost.alive is False
    assert game.score == game.points_per_ghost

    home = Position(game.maze.height - 1, game.maze.width // 2)
    ghost.position = home
    game.move_ghosts()
    assert ghost.mode.value == GhostMode.CHASE.value
    assert ghost.alive is True
    assert ghost.dead_timer == 0


def test_dead_ghost_follows_path_before_revival() -> None:
    game = _make_game(width=8, height=8, seed=3)
    ghost = game.ghosts[0]
    ghost.mode = GhostMode.DEAD
    ghost.alive = False
    initial_position = Position(ghost.position.row, ghost.position.col)
    home = Position(game.maze.height - 1, game.maze.width // 2)

    game.move_ghosts()

    assert ghost.mode == GhostMode.DEAD
    assert ghost.alive is False
    assert ghost.position != initial_position
    assert ghost.position != home


def test_dead_ghost_waits_when_home_is_unreachable() -> None:
    maze = Maze(3, 3, seed=1, perfect=True)
    for row in maze.grid:
        for cell in row:
            for wall in (NORTH, EAST, SOUTH, WEST):
                cell.add_wall(wall)
    ghost = Ghost(GhostKind.BLINKY, Position(0, 0))
    game = Game(maze, Pacman(Position(1, 1)), [ghost], [])
    ghost.mode = GhostMode.DEAD
    ghost.alive = False

    game.move_ghosts()

    assert ghost.position == Position(0, 0)
    assert ghost.mode == GhostMode.DEAD
    assert ghost.alive is False


def test_dead_ghost_cannot_hurt_pacman() -> None:
    game = _make_game()
    ghost = game.ghosts[0]
    ghost.mode = GhostMode.DEAD
    game.pacman.position = Position(ghost.position.row, ghost.position.col)
    lives_before = game.lives

    game.check_collisions()

    assert game.lives == lives_before
    assert ghost.mode == GhostMode.DEAD


def test_no_unhandled_errors_over_many_ticks() -> None:
    game = _make_game(width=12, height=12, seed=7)
    directions = [
        Direction.NORTH,
        Direction.EAST,
        Direction.SOUTH,
        Direction.WEST,
    ]
    for index in range(500):
        if game.status != GameStatus.PLAYING:
            break
        game.set_input(directions[index % len(directions)])
        game.step()

    assert game.status in {
        GameStatus.PLAYING,
        GameStatus.GAME_OVER,
        GameStatus.LEVEL_COMPLETE,
        GameStatus.VICTORY,
    }


def test_timer_stops_decreasing_after_game_over() -> None:
    game = _make_game()
    game.lives = 1
    ghost = game.ghosts[0]
    game.pacman.position = Position(ghost.position.row, ghost.position.col)
    game.check_collisions()
    assert game.status == GameStatus.GAME_OVER

    timer_before = game.timer
    for _ in range(10):
        game.step()
    assert game.timer == timer_before
