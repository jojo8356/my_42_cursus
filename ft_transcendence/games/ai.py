"""
AI opponent for Pong.

Subject constraints:
  - The AI must replicate human behavior: it only simulates keyboard input
    (move paddle up/down), never teleports.
  - It refreshes its view of the game once per second, so it must anticipate
    bounces off the top/bottom walls.
  - It uses the exact same paddle speed as a human player.
  - The A* algorithm is NOT permitted.

The AI runs the same physics as the human (a shared ``PongGame``), steps it
forward in its head to predict the ball position ~1 second ahead, then issues
a single up/down keypress per refresh, exactly like a human at 1 Hz.
"""
from .engine import PongGame


class PongAI:
    """A reactive AI that tracks the predicted ball position.

    The AI owns a private copy of the game so it can fast-forward the physics
    to predict where the ball will be on its next (1 Hz) refresh, accounting
    for wall bounces. It then moves its paddle toward that predicted y, but
    only one keypress per refresh - so its speed is capped at paddle_speed.
    """

    REFRESH_RATE = 1.0  # seconds between AI decisions (subject requirement)

    def __init__(self, side="right"):
        self.side = side

    def decide(self, game, elapsed):
        """Return a move direction: -1 (up), 0 (hold), +1 (down).

        ``elapsed`` is the time since the last AI decision. The AI only acts
        once per REFRESH_RATE, otherwise it holds (does nothing), mimicking a
        human who presses a key at ~1 Hz.
        """
        if elapsed < self.REFRESH_RATE:
            return 0

        # Predict the ball position by fast-forwarding a clone of the game.
        predicted_y = self._predict_ball_y(game, self.REFRESH_RATE)

        paddle = game.left_paddle if self.side == "left" else game.right_paddle
        paddle_height = (
            game._current_paddle_height("left") if self.side == "left"
            else game._current_paddle_height("right")
        )
        center = paddle.y + paddle_height / 2.0
        if predicted_y < center - 2:
            return -1  # move up
        if predicted_y > center + 2:
            return 1  # move down
        return 0  # already aligned

    def _predict_ball_y(self, game, seconds):
        """Fast-forward a throwaway copy of the game to predict ball.y.

        We replicate the ball trajectory analytically (constant velocity with
        wall reflections) which is exact and cheap - no search, no A*.
        """
        # work on a shallow clone of the ball state only
        y = game.ball.y
        vy = game.ball_vy
        speed = game._current_ball_speed()
        sign_vy = 1.0 if vy >= 0 else -1.0
        r = game.ball_radius

        # total vertical distance the ball travels in `seconds`
        distance = speed * seconds * abs(sign_vy) if vy != 0 else 0.0
        # If the ball has no vertical velocity, it stays on the same row.
        if vy == 0:
            return y

        direction = sign_vy
        remaining = distance
        # bounce between top (r) and bottom (height - r)
        low = r
        high = game.height - r
        span = high - low
        if span <= 0:
            return y

        # number of wall bounces within the remaining distance
        pos = y
        while remaining > 0:
            if direction > 0:
                to_top = high - pos
                if remaining <= to_top:
                    pos += remaining
                    remaining = 0
                else:
                    remaining -= to_top
                    pos = high
                    direction = -1
            else:
                to_bottom = pos - low
                if remaining <= to_bottom:
                    pos -= remaining
                    remaining = 0
                else:
                    remaining -= to_bottom
                    pos = low
                    direction = 1
        return pos

    def act(self, game, elapsed):
        """Apply the AI decision to the game (moves the paddle)."""
        direction = self.decide(game, elapsed)
        if direction != 0:
            game.move_paddle(self.side, direction)
        return direction
