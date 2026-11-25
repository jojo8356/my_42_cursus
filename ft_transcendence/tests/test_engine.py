"""
Comprehensive tests for the Pong game engine (games/engine.py).

Covers: physics, wall bounces, paddle collisions, scoring, determinism,
power-ups, customization toggles, serialization, lifecycle.
"""
from django.test import SimpleTestCase

from games.engine import PongGame, Position


class PongEngineTests(SimpleTestCase):
    def test_initial_state_is_waiting(self):
        g = PongGame()
        self.assertEqual(g.state, "waiting")
        self.assertEqual(g.left_score, 0)
        self.assertEqual(g.right_score, 0)

    def test_start_sets_playing(self):
        g = PongGame()
        g.start()
        self.assertEqual(g.state, "playing")

    def test_ball_starts_at_center(self):
        g = PongGame()
        g.start()
        self.assertAlmostEqual(g.ball.x, g.width / 2)
        self.assertAlmostEqual(g.ball.y, g.height / 2)

    def test_serve_direction(self):
        g = PongGame()
        g.start(serve_to="left")
        self.assertLess(g.ball_vx, 0)  # moving toward left
        g.start(serve_to="right")
        self.assertGreater(g.ball_vx, 0)

    def test_paddle_speed_identical(self):
        """Subject rule: all players (and AI) have identical paddle speed."""
        g = PongGame()
        g.start()
        y0 = g.left_paddle.y
        g.move_paddle("left", 1, dt=1.0)
        self.assertAlmostEqual(g.left_paddle.y - y0, g.paddle_speed)

    def test_paddle_clamped_top(self):
        g = PongGame()
        g.start()
        g.left_paddle.y = -100
        g.move_paddle("left", -1, dt=1.0)
        self.assertEqual(g.left_paddle.y, 0)

    def test_paddle_clamped_bottom(self):
        g = PongGame()
        g.start()
        max_y = g.height - g.paddle_height
        g.left_paddle.y = max_y + 100
        g.move_paddle("left", 1, dt=1.0)
        self.assertEqual(g.left_paddle.y, float(max_y))

    def test_move_only_when_playing(self):
        g = PongGame()
        y0 = g.left_paddle.y
        g.move_paddle("left", 1, dt=1.0)  # not playing yet
        self.assertEqual(g.left_paddle.y, y0)

    def test_wall_bounce_top(self):
        g = PongGame()
        g.start()
        g.ball.y = 1  # near top
        g.ball_vy = -5
        g.ball.x = g.width / 2
        g.ball_vx = 6
        events = g.step()
        self.assertIn("bounce_top", events)
        self.assertGreaterEqual(g.ball.y, g.ball_radius)

    def test_wall_bounce_bottom(self):
        g = PongGame()
        g.start()
        g.ball.y = g.height - 1
        g.ball_vy = 5
        g.ball.x = g.width / 2
        g.ball_vx = 6
        events = g.step()
        self.assertIn("bounce_bottom", events)
        self.assertLessEqual(g.ball.y, g.height - g.ball_radius)

    def test_paddle_collision_left(self):
        g = PongGame()
        g.start()
        # place ball just left of the right side moving right into left paddle
        g.ball.x = g.left_paddle.x + g.paddle_width + 1
        g.ball.y = g.left_paddle.y + 20
        g.ball_vx = -6  # moving left (toward left paddle)
        g.ball_vy = 0
        events = g.step()
        self.assertIn("paddle_left", events)
        self.assertGreater(g.ball_vx, 0)  # reflected to the right

    def test_paddle_collision_right(self):
        g = PongGame()
        g.start()
        g.ball.x = g.right_paddle.x - 1
        g.ball.y = g.right_paddle.y + 20
        g.ball_vx = 6
        g.ball_vy = 0
        events = g.step()
        self.assertIn("paddle_right", events)
        self.assertLess(g.ball_vx, 0)

    def test_scoring_left(self):
        g = PongGame()
        g.start()
        g.ball.x = -5  # passed left paddle
        g.ball_vx = -6
        events = g.step()
        self.assertIn("score_right", events)
        self.assertEqual(g.right_score, 1)

    def test_scoring_right(self):
        g = PongGame()
        g.start()
        g.ball.x = g.width + 5
        g.ball_vx = 6
        events = g.step()
        self.assertIn("score_left", events)
        self.assertEqual(g.left_score, 1)

    def test_ball_resets_after_score(self):
        g = PongGame()
        g.start()
        g.ball.x = -5
        g.ball_vx = -6  # moving left so it scores
        g.step()
        self.assertAlmostEqual(g.ball.x, g.width / 2)

    def test_determinism(self):
        """Same initial state + moves => identical outcome."""
        def run():
            g = PongGame()
            g.start()
            g.ball.x = 400
            g.ball.y = 300
            g.ball_vx = 6
            g.ball_vy = 3
            for _ in range(20):
                g.step()
            return (g.ball.x, g.ball.y, g.left_score, g.right_score)
        self.assertEqual(run(), run())

    def test_win_condition(self):
        g = PongGame()
        g.start()
        # force left to score twice
        for _ in range(2):
            g.ball.x = g.width + 5
            g.ball_vx = 6
            g.step()
        self.assertEqual(g.state, "over")
        self.assertEqual(g.winner, "left")

    def test_powerup_speed(self):
        g = PongGame()
        g.start()
        y0 = g.left_paddle.y
        g.apply_powerup("left", "speed")
        g.move_paddle("left", 1, dt=1.0)
        # speed powerup multiplies paddle speed by 1.5
        self.assertAlmostEqual(g.left_paddle.y - y0, g.paddle_speed * 1.5)

    def test_powerup_size(self):
        g = PongGame()
        g.start()
        g.apply_powerup("right", "size")
        self.assertAlmostEqual(g._current_paddle_height("right"), g.paddle_height * 1.5)

    def test_powerup_slow_ball(self):
        g = PongGame()
        g.start()
        g.apply_powerup("left", "slow_ball")
        self.assertAlmostEqual(g._current_ball_speed(), g.ball_speed * 0.5)

    def test_powerup_disabled(self):
        g = PongGame()
        g.enable_powerups = False
        self.assertFalse(g.apply_powerup("left", "speed"))

    def test_powerup_expiry(self):
        g = PongGame()
        g.start()
        g.apply_powerup("left", "speed", duration=1)
        g.tick_powerups()  # decrements to 0, removed
        self.assertFalse(g._has_powerup("left", "speed"))

    def test_custom_map_toggle(self):
        g = PongGame()
        g.enable_custom_map = True
        self.assertTrue(g.enable_custom_map)

    def test_to_state_serialization(self):
        g = PongGame()
        g.start()
        state = g.to_state()
        for key in ("width", "height", "left_paddle", "right_paddle",
                    "ball", "left_score", "right_score", "state"):
            self.assertIn(key, state)

    def test_no_step_when_not_playing(self):
        g = PongGame()
        events = g.step()
        self.assertEqual(events, [])
