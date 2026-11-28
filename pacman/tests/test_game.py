"""Unit tests for game rules and session-level progression."""
import random

import pytest

from game import Game
from maze import Maze
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


@pytest.fixture
def small_game() -> Game:
    """A small, deterministic game for engine tests."""
    maze = Maze(6, 6, seed=42, perfect=False)
    pacman = Pacman(Position(3, 3), Direction.EAST)
    ghosts = [
        Ghost(GhostKind.BLINKY, Position(0, 0), Direction.SOUTH),
        Ghost(GhostKind.PINKY, Position(0, 5), Direction.SOUTH),
        Ghost(GhostKind.INKY, Position(5, 5), Direction.WEST),
        Ghost(GhostKind.CLYDE, Position(5, 0), Direction.NORTH),
    ]
    pacgums = [
        Pacgum(Position(1, 1)),
        Pacgum(Position(1, 2)),
        Pacgum(Position(2, 3), is_super=True),
    ]
    return Game(
        maze,
        pacman,
        ghosts,
        pacgums,
        lives=3,
        level_time=600,
        rng=random.Random(0),
    )


def test_initial_state(small_game: Game) -> None:
    assert small_game.status == GameStatus.PLAYING
    assert small_game.score == 0
    assert small_game.lives == 3
    assert small_game.level == 1
    assert small_game._remaining_pacgums() == 3
    assert small_game.frightened_remaining == 0


def test_pacman_moves(small_game: Game) -> None:
    game = small_game
    start = game.pacman.position
    for direction in (
        Direction.NORTH,
        Direction.EAST,
        Direction.SOUTH,
        Direction.WEST,
    ):
        if game.maze.is_passable(start.row, start.col, direction):
            game.set_input(direction)
            game.step()
            assert game.pacman.position != start
            return
    pytest.skip("no passable direction from centre")


def test_pacman_does_not_move_after_death_or_game_over(
    small_game: Game,
) -> None:
    game = small_game
    start = game.pacman.position
    game.set_input(Direction.EAST)
    game.pacman.alive = False
    game.move_pacman()
    assert game.pacman.position == start

    game.pacman.alive = True
    game.status = GameStatus.GAME_OVER
    game.move_pacman()
    assert game.pacman.position == start


def test_pacman_blocked_by_wall(small_game: Game) -> None:
    game = small_game
    start = game.pacman.position
    blocked = None
    for direction in (
        Direction.NORTH,
        Direction.EAST,
        Direction.SOUTH,
        Direction.WEST,
    ):
        if not game.maze.is_passable(start.row, start.col, direction):
            blocked = direction
            break
    if blocked is None:
        pytest.skip("no blocked direction")
    game.pacman.direction = blocked
    game.set_input(blocked)
    game.move_pacman()
    assert game.pacman.position == start


def test_eat_pacgum_scores(small_game: Game) -> None:
    game = small_game
    pacgum = game.pacgums[0]
    game.pacman.position = Position(pacgum.position.row, pacgum.position.col)
    assert game.eat_pacgum_at(
        game.pacman.position.row,
        game.pacman.position.col,
    )
    assert game.score == game.points_per_pacgum
    assert game._remaining_pacgums() == 2


def test_super_pacgum_activates_frightened(small_game: Game) -> None:
    game = small_game
    super_pacgum = next(pacgum for pacgum in game.pacgums if pacgum.is_super)
    game.pacman.position = Position(
        super_pacgum.position.row,
        super_pacgum.position.col,
    )
    game.eat_pacgum_at(game.pacman.position.row, game.pacman.position.col)

    assert game.score == game.points_per_super_pacgum
    assert game.frightened_remaining == game.frightened_ticks
    for ghost in game.ghosts:
        assert ghost.mode == GhostMode.FRIGHTENED
        assert ghost.frightened_timer == game.frightened_ticks


def test_eat_frightened_ghost(small_game: Game) -> None:
    game = small_game
    ghost = game.ghosts[0]
    ghost.mode = GhostMode.FRIGHTENED
    game.pacman.position = Position(ghost.position.row, ghost.position.col)
    game.check_collisions()

    assert ghost.mode == GhostMode.DEAD
    assert ghost.alive is False
    assert game.score == game.points_per_ghost


def test_hit_chasing_ghost_loses_life(small_game: Game) -> None:
    game = small_game
    ghost = game.ghosts[0]
    ghost.mode = GhostMode.CHASE
    game.pacman.position = Position(ghost.position.row, ghost.position.col)
    lives_before = game.lives
    game.check_collisions()
    assert game.lives == lives_before - 1


def test_game_over_when_no_lives(small_game: Game) -> None:
    game = small_game
    game.lives = 1
    ghost = game.ghosts[0]
    game.pacman.position = Position(ghost.position.row, ghost.position.col)
    game.check_collisions()
    assert game.status == GameStatus.GAME_OVER


def test_timer_expires_loses_life(small_game: Game) -> None:
    game = small_game
    game.timer = 1
    lives_before = game.lives
    game.step()
    assert game.lives == lives_before - 1


def test_level_complete_when_all_pellets_eaten(small_game: Game) -> None:
    game = small_game
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


def test_next_level(small_game: Game) -> None:
    game = small_game
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

    new_level = game.next_level()

    assert new_level == 2
    assert game.status.value == GameStatus.PLAYING.value
    assert game.score == 70


def test_start_next_level_preserves_session_and_resets_level_state(
    small_game: Game,
) -> None:
    game = small_game
    game.score = 1234
    game.lives = 2
    game.next_extra_life = 5000
    game.frightened_remaining = 42
    game.fruit_eaten = 17
    game.fruit_present = True
    game.fruit_position = Position(1, 1)
    game.pacman_input = Direction.WEST
    game.ghosts[0].mode = GhostMode.DEAD
    game.ghosts[0].alive = False

    new_ghosts = [
        Ghost(GhostKind.BLINKY, Position(0, 0)),
        Ghost(GhostKind.PINKY, Position(0, 3)),
        Ghost(GhostKind.INKY, Position(3, 3)),
        Ghost(GhostKind.CLYDE, Position(3, 0)),
    ]
    new_level = Game(
        Maze(4, 4, seed=9),
        Pacman(Position(2, 2)),
        new_ghosts,
        [Pacgum(Position(1, 1))],
        lives=99,
        level_time=300,
        rng=random.Random(9),
    )

    assert game.start_next_level(new_level) == 2

    assert game.score == 1234
    assert game.lives == 2
    assert game.next_extra_life == 5000
    assert game.status == GameStatus.PLAYING
    assert game.timer == 300
    assert game.level_time == 300
    assert game.maze is new_level.maze
    assert game.pacman.position == Position(2, 2)
    assert game.pacman_input is None
    assert game.frightened_remaining == 0
    assert game.fruit_eaten == 0
    assert game.fruit_present is False
    assert game.fruit_position is None
    assert game._remaining_pacgums() == 1
    assert all(ghost.mode == GhostMode.CHASE for ghost in game.ghosts)
    assert all(ghost.alive for ghost in game.ghosts)
    assert all(ghost.dead_timer == 0 for ghost in game.ghosts)


def test_extra_life_at_threshold(small_game: Game) -> None:
    game = small_game
    game.score = game.extra_life_score
    game._check_extra_life()
    assert game.lives == 4
    game.score = 2 * game.extra_life_score
    game._check_extra_life()
    assert game.lives == 5


def test_fruit_spawn_falls_back_when_board_is_full() -> None:
    position = Position(0, 0)
    game = Game(
        Maze(1, 1),
        Pacman(position),
        [Ghost(GhostKind.BLINKY, position)],
        [Pacgum(position)],
    )

    assert game._random_empty_cell() == position


def test_fruit_spawns_and_is_eaten(small_game: Game) -> None:
    game = small_game
    game.fruit_eaten = game.fruit_threshold
    game._maybe_spawn_fruit()
    assert game.fruit_present is True
    assert game.fruit_position is not None
    game.pacman.position = Position(
        game.fruit_position.row,
        game.fruit_position.col,
    )
    score_before = game.score
    game.check_collisions()
    assert game.score == score_before + 100
    assert game.fruit_present is False
    assert game.fruit_position is None


def test_ghost_frightened_timer_expires(small_game: Game) -> None:
    game = small_game
    super_pacgum = next(pacgum for pacgum in game.pacgums if pacgum.is_super)
    game.pacman.position = Position(
        super_pacgum.position.row,
        super_pacgum.position.col,
    )
    game.eat_pacgum_at(game.pacman.position.row, game.pacman.position.col)
    assert game.frightened_remaining > 0
    for _ in range(game.frightened_ticks + 1):
        if game.status != GameStatus.PLAYING:
            break
        game.step()
    for ghost in game.ghosts:
        if ghost.mode != GhostMode.DEAD:
            assert ghost.mode != GhostMode.FRIGHTENED


def test_to_state_serialization(small_game: Game) -> None:
    state = small_game.to_state()
    assert state["status"] == "playing"
    assert state["score"] == 0
    assert state["lives"] == 3
    assert state["level"] == 1
    assert isinstance(state["ghosts"], list)
    assert state["pacgums_left"] == 3


def test_respawn_after_life_lost(small_game: Game) -> None:
    game = small_game
    ghost = game.ghosts[0]
    game.pacman.position = Position(ghost.position.row, ghost.position.col)
    game.check_collisions()

    assert game.lives == 2
    assert game.pacman.position == Position(
        game.maze.height // 2,
        game.maze.width // 2,
    )


def test_respawn_leaves_dead_ghost_returning_home(small_game: Game) -> None:
    game = small_game
    dead_ghost = game.ghosts[0]
    original_position = Position(1, 1)
    dead_ghost.position = original_position
    dead_ghost.mode = GhostMode.DEAD
    dead_ghost.alive = False

    game._respawn()

    assert dead_ghost.position == original_position
    assert dead_ghost.mode == GhostMode.DEAD
    assert dead_ghost.alive is False
