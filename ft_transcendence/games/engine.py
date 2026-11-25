"""
Pong game engine - pure, deterministic, fully testable.

Implements the physics and rules of the classic Pong (1972):
  - ball travels with a velocity, bounces off top/bottom walls
  - paddles move up/down at a FIXED speed (identical for humans and AI)
  - scoring when the ball passes a paddle
  - deterministic state transitions

The frontend canvas mirrors this engine in vanilla JS. Running the engine
server-side lets us unit-test every rule, including the AI opponent.
"""
from dataclasses import dataclass, field


# Default field dimensions (logical units, not pixels).
DEFAULT_WIDTH = 800
DEFAULT_HEIGHT = 600
PADDLE_WIDTH = 12
PADDLE_HEIGHT = 100
BALL_RADIUS = 8

# Paddle speed is identical for every player and for the AI (subject rule).
DEFAULT_PADDLE_SPEED = 6.0
DEFAULT_BALL_SPEED = 6.0


@dataclass
class Position:
    x: float = 0.0
    y: float = 0.0


@dataclass
class PowerUp:
    """A power-up that can modify gameplay for one player."""
    name: str
    kind: str  # "speed", "size", "slow_ball"
    duration: int = 0  # turns remaining (0 = instant/permanent-ish)

    def __str__(self):
        return f"{self.name}({self.kind})"


@dataclass
class PongGame:
    """A deterministic Pong game state."""
    width: int = DEFAULT_WIDTH
    height: int = DEFAULT_HEIGHT
    paddle_width: int = PADDLE_WIDTH
    paddle_height: int = PADDLE_HEIGHT
    ball_radius: int = BALL_RADIUS
    paddle_speed: float = DEFAULT_PADDLE_SPEED
    ball_speed: float = DEFAULT_BALL_SPEED

    # left paddle (player 1 / human), right paddle (player 2 / human or AI)
    left_paddle: Position = field(default_factory=lambda: Position(40, 0))
    right_paddle: Position = field(default_factory=lambda: Position(0, 0))

    ball: Position = field(default_factory=lambda: Position(0, 0))
    ball_vx: float = 0.0
    ball_vy: float = 0.0

    left_score: int = 0
    right_score: int = 0

    # players: "left" / "right"; who is currently an AI
    left_is_ai: bool = False
    right_is_ai: bool = False

    # active power-ups per side
    left_powerups: list = field(default_factory=list)
    right_powerups: list = field(default_factory=list)

    # customization toggles
    enable_powerups: bool = True
    enable_custom_map: bool = False

    # game lifecycle
    state: str = "waiting"  # waiting | playing | over
    winner: str = ""  # "left" | "right" | ""
    winner_margin: int = 0

    def __post_init__(self):
        if self.ball.x == 0.0 and self.ball.y == 0.0:
            self.reset_ball(direction=1)
        self.left_paddle.y = self._clamp_paddle(self.left_paddle.y, self.left_paddle)
        self.right_paddle.y = self._clamp_paddle(self.right_paddle.y, self.right_paddle)

    # ------------------------------------------------------------------ utils
    def _clamp_paddle(self, y, paddle):
        max_y = self.height - self.paddle_height
        if y < 0:
            return 0.0
        if y > max_y:
            return float(max_y)
        return float(y)

    def _current_paddle_speed(self, side):
        if side == "left":
            for pu in self.left_powerups:
                if pu.kind == "speed":
                    return self.paddle_speed * 1.5
        else:
            for pu in self.right_powerups:
                if pu.kind == "speed":
                    return self.paddle_speed * 1.5
        return self.paddle_speed

    def _current_paddle_height(self, side):
        if side == "left":
            for pu in self.left_powerups:
                if pu.kind == "size":
                    return self.paddle_height * 1.5
        else:
            for pu in self.right_powerups:
                if pu.kind == "size":
                    return self.paddle_height * 1.5
        return self.paddle_height

    def _current_ball_speed(self):
        slow = (
            self._has_powerup("left", "slow_ball")
            or self._has_powerup("right", "slow_ball")
        )
        if slow:
            return self.ball_speed * 0.5
        return self.ball_speed

    def _has_powerup(self, side, kind):
        pu_list = self.left_powerups if side == "left" else self.right_powerups
        return any(pu.kind == kind for pu in pu_list)

    # ------------------------------------------------------------------ lifecycle
    def start(self, serve_to="left"):
        """Begin a playing round, serving the ball toward a side.

        serve_to="left" sends the ball toward the left paddle (negative vx);
        serve_to="right" sends it toward the right paddle (positive vx).
        """
        self.state = "playing"
        self.winner = ""
        self.winner_margin = 0
        direction = -1 if serve_to == "left" else 1
        self.reset_ball(direction=direction)

    def reset_ball(self, direction=1):
        """Place the ball at center, moving toward the given direction."""
        self.ball.x = self.width / 2.0
        self.ball.y = self.height / 2.0
        self.ball_vx = self._current_ball_speed() * direction
        self.ball_vy = 0.0

    def end_round(self, winner):
        self.state = "over"
        self.winner = winner
        self.winner_margin = 2

    # ------------------------------------------------------------------ input
    def move_paddle(self, side, direction, dt=1.0):
        """Move a paddle up (-1) or down (+1) by its (fixed) speed * dt."""
        if self.state != "playing":
            return
        speed = self._current_paddle_speed(side)
        if side == "left":
            self.left_paddle.y += direction * speed * dt
            self.left_paddle.y = self._clamp_paddle(
                self.left_paddle.y, self.left_paddle
            )
        else:
            self.right_paddle.y += direction * speed * dt
            self.right_paddle.y = self._clamp_paddle(
                self.right_paddle.y, self.right_paddle
            )

    def apply_powerup(self, side, kind, duration=5):
        """Apply a power-up to a side (Game customization module)."""
        if not self.enable_powerups:
            return False
        pu = PowerUp(name=kind, kind=kind, duration=duration)
        target = self.left_powerups if side == "left" else self.right_powerups
        target.append(pu)
        return True

    def tick_powerups(self):
        """Decrement power-up durations and remove expired ones."""
        for side in ("left", "right"):
            pu_list = self.left_powerups if side == "left" else self.right_powerups
            for pu in list(pu_list):
                pu.duration -= 1
                if pu.duration <= 0:
                    pu_list.remove(pu)

    # ------------------------------------------------------------------ physics
    def step(self, dt=1.0):
        """Advance the game by one tick. Returns a dict describing events."""
        events = []
        if self.state != "playing":
            return events

        # move ball
        speed = self._current_ball_speed()
        # preserve direction sign, apply current speed magnitude
        sign_vx = 1.0 if self.ball_vx >= 0 else -1.0
        sign_vy = 1.0 if self.ball_vy >= 0 else -1.0
        self.ball.x += sign_vx * speed * dt
        self.ball.y += sign_vy * speed * dt

        # top / bottom wall bounce
        if self.ball.y - self.ball_radius <= 0:
            self.ball.y = self.ball_radius
            self.ball_vy = abs(self.ball_vy)
            events.append("bounce_top")
        elif self.ball.y + self.ball_radius >= self.height:
            self.ball.y = self.height - self.ball_radius
            self.ball_vy = -abs(self.ball_vy)
            events.append("bounce_bottom")

        # paddle collisions
        if self._paddle_collision("left"):
            self.ball_vx = abs(self.ball_vx)
            self._bounce_off_paddle("left")
            events.append("paddle_left")
        if self._paddle_collision("right"):
            self.ball_vx = -abs(self.ball_vx)
            self._bounce_off_paddle("right")
            events.append("paddle_right")

        # scoring
        if self.ball.x < 0:
            self.right_score += 1
            events.append("score_right")
            self.reset_ball(direction=1)
            if self._check_win():
                self.end_round("right")
                events.append("game_over")
        elif self.ball.x > self.width:
            self.left_score += 1
            events.append("score_left")
            self.reset_ball(direction=-1)
            if self._check_win():
                self.end_round("left")
                events.append("game_over")

        self.tick_powerups()
        return events

    def _paddle_collision(self, side):
        if side == "left":
            px = self.left_paddle.x
            py = self.left_paddle.y
            ph = self._current_paddle_height("left")
        else:
            px = self.right_paddle.x
            py = self.right_paddle.y
            ph = self._current_paddle_height("right")
        bx, by, r = self.ball.x, self.ball.y, self.ball_radius
        # AABB collision between ball circle and paddle rectangle
        if bx - r < px + self.paddle_width and bx + r > px:
            if by > py and by < py + ph:
                return True
        return False

    def _bounce_off_paddle(self, side):
        """Reflect ball y-velocity based on where it hit the paddle,
        and add a little spin proportional to paddle position."""
        if side == "left":
            py = self.left_paddle.y
            ph = self._current_paddle_height("left")
        else:
            py = self.right_paddle.y
            ph = self._current_paddle_height("right")
        hit = (self.ball.y - py) / ph if ph > 0 else 0.5
        hit = max(-0.5, min(0.5, hit - 0.5))
        self.ball_vy = hit * self._current_ball_speed() * 2.0

    def _check_win(self):
        """First to WIN_MARGIN points ahead wins the round."""
        return abs(self.left_score - self.right_score) >= self.winner_margin and (
            self.left_score >= 2 or self.right_score >= 2
        )

    # ------------------------------------------------------------------ serialization
    def to_state(self):
        """Serialize the game state for websocket sync / frontend."""
        return {
            "width": self.width,
            "height": self.height,
            "paddle_width": self.paddle_width,
            "paddle_height": self.paddle_height,
            "ball_radius": self.ball_radius,
            "left_paddle": {"x": self.left_paddle.x, "y": self.left_paddle.y},
            "right_paddle": {"x": self.right_paddle.x, "y": self.right_paddle.y},
            "ball": {"x": self.ball.x, "y": self.ball.y},
            "ball_vx": self.ball_vx,
            "ball_vy": self.ball_vy,
            "left_score": self.left_score,
            "right_score": self.right_score,
            "state": self.state,
            "winner": self.winner,
            "left_is_ai": self.left_is_ai,
            "right_is_ai": self.right_is_ai,
        }
