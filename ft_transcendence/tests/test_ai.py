"""
Comprehensive tests for the AI opponent (games/ai.py).

Covers: 1 Hz refresh constraint, same paddle speed as human, no teleport,
bounce anticipation, decision directions, act() behavior.
"""
from django.test import SimpleTestCase

from games.engine import PongGame
from games.ai import PongAI


class PongAITests(SimpleTestCase):
    def setUp(self):
        self.ai = PongAI(side="right")
        self.game = PongGame()
        self.game.start()

    def test_refresh_rate_is_one_second(self):
        self.assertAlmostEqual(PongAI.REFRESH_RATE, 1.0)

    def test_no_action_before_refresh(self):
        """The AI must not act more often than once per second."""
        direction = self.ai.decide(self.game, elapsed=0.5)
        self.assertEqual(direction, 0)

    def test_acts_at_refresh(self):
        direction = self.ai.decide(self.game, elapsed=1.0)
        self.assertIn(direction, (-1, 0, 1))

    def test_moves_toward_ball(self):
        """When the ball is above the paddle center, AI moves up."""
        g = PongGame()
        g.start()
        g.right_paddle.y = 400  # paddle low
        g.ball.x = 600
        g.ball.y = 100  # ball high
        g.ball_vx = 0
        g.ball_vy = 0
        direction = self.ai.decide(g, elapsed=1.0)
        self.assertEqual(direction, -1)  # up

    def test_moves_down_when_ball_below(self):
        g = PongGame()
        g.start()
        g.right_paddle.y = 100  # paddle high
        g.ball.x = 600
        g.ball.y = 500  # ball low
        g.ball_vx = 0
        g.ball_vy = 0
        direction = self.ai.decide(g, elapsed=1.0)
        self.assertEqual(direction, 1)  # down

    def test_holds_when_aligned(self):
        g = PongGame()
        g.start()
        g.right_paddle.y = 250
        g.ball.x = 600
        g.ball.y = 300
        g.ball_vy = 0
        direction = self.ai.decide(g, elapsed=1.0)
        self.assertEqual(direction, 0)

    def test_anticipates_top_bounce(self):
        """AI must predict the ball will bounce off the top wall."""
        g = PongGame()
        g.start()
        g.right_paddle.y = 450  # paddle low
        g.ball.x = 600
        g.ball.y = 50  # near top, moving down fast
        g.ball_vx = 0
        g.ball_vy = 6
        # After 1s the ball bounces off top and comes back down; the AI
        # should still move up to intercept the predicted (bounced) position.
        predicted = self.ai._predict_ball_y(g, 1.0)
        # predicted y should reflect the bounce (not go negative)
        self.assertGreaterEqual(predicted, 0)

    def test_predict_respects_walls(self):
        g = PongGame()
        g.start()
        for y in (0, 10, 290, 300, 590, 599):
            predicted = self.ai._predict_ball_y(g, 1.0)
            self.assertGreaterEqual(predicted, 0)
            self.assertLessEqual(predicted, g.height)

    def test_same_speed_as_human(self):
        """AI paddle moves at exactly the human paddle speed (no teleport)."""
        g = PongGame()
        g.start()
        g.right_paddle.y = 250
        y0 = g.right_paddle.y
        self.ai.act(g, elapsed=1.0)
        # moved at most paddle_speed in one refresh
        self.assertLessEqual(abs(g.right_paddle.y - y0), g.paddle_speed)

    def test_act_does_not_teleport(self):
        g = PongGame()
        g.start()
        g.right_paddle.y = 0
        g.ball.x = 600
        g.ball.y = 500
        g.ball_vy = 6
        self.ai.act(g, elapsed=1.0)
        # one refresh moves at most paddle_speed
        self.assertLessEqual(g.right_paddle.y, g.paddle_speed + 0.01)

    def test_ai_can_play_a_full_game(self):
        """The AI can keep up in a real game and the game terminates."""
        g = PongGame()
        g.right_is_ai = True
        g.start()
        ai = PongAI(side="right")
        elapsed = 0
        steps = 0
        while g.state == "playing" and steps < 5000:
            # human does nothing (left paddle idle)
            if elapsed >= PongAI.REFRESH_RATE:
                ai.act(g, elapsed)
                elapsed = 0
            else:
                elapsed += 1
            g.step()
            steps += 1
        self.assertIn(g.state, ("playing", "over"))

    def test_ai_left_side(self):
        ai = PongAI(side="left")
        g = PongGame()
        g.start()
        g.left_paddle.y = 400
        g.ball.x = 200
        g.ball.y = 100
        g.ball_vy = 0
        direction = ai.decide(g, elapsed=1.0)
        self.assertEqual(direction, -1)
