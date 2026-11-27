"""
Comprehensive tests for the second game (module 7): Connect-4.

Covers the engine (drops, win detection, draws, turn order, undo,
serialization), the AI opponent (valid moves, no cheating, can win),
and the API (create connect4 session, drop a move, finish + match
history, matchmaking queue).
"""
from django.test import TestCase, SimpleTestCase
from rest_framework.test import APIClient

from games.connect4 import Connect4, PLAYER1, PLAYER2, EMPTY
from games.connect4_ai import Connect4AI
from games.models import GameSession
from accounts.models import User, MatchHistory


# ------------------------------------------------------------------ engine
class Connect4EngineTests(SimpleTestCase):
    def test_initial_state(self):
        g = Connect4()
        self.assertEqual(g.winner, EMPTY)
        self.assertEqual(g.current, PLAYER1)
        self.assertEqual(g.board[0][0], EMPTY)
        self.assertEqual(g.rows, 6)
        self.assertEqual(g.columns, 7)

    def test_drop_falls_to_bottom(self):
        g = Connect4()
        self.assertTrue(g.drop(0))
        self.assertEqual(g.board[5][0], PLAYER1)  # bottom row
        self.assertEqual(g.current, PLAYER2)

    def test_drop_invalid_column(self):
        g = Connect4()
        self.assertFalse(g.drop(-1))
        self.assertFalse(g.drop(7))
        self.assertFalse(g.drop("x"))

    def test_full_column_rejected(self):
        g = Connect4()
        for _ in range(6):
            g.drop(0)
        # column 0 now full at top
        self.assertFalse(g.drop(0))

    def test_horizontal_win(self):
        # P1 wins on the bottom row across columns 0-3.
        g = Connect4()
        g.drop(0)   # p1
        g.drop(4)   # p2
        g.drop(1)   # p1
        g.drop(5)   # p2
        g.drop(2)   # p1
        g.drop(6)   # p2
        g.drop(3)   # p1 -> 4 in a row on the bottom row
        self.assertEqual(g.winner, PLAYER1)

    def test_horizontal_win_direct(self):
        # Construct a board with 4 in a row on the bottom row for player 1.
        g = Connect4()
        g.board = [[EMPTY] * 7 for _ in range(6)]
        g.board[5][0] = PLAYER1
        g.board[5][1] = PLAYER1
        g.board[5][2] = PLAYER1
        g.board[5][3] = PLAYER1
        self.assertTrue(g._check_win(5, 3, PLAYER1))
        self.assertEqual(g.winning_line(PLAYER1)[0], (5, 0))

    def test_vertical_win_direct(self):
        g = Connect4()
        g.board = [[EMPTY] * 7 for _ in range(6)]
        for r in range(2, 6):
            g.board[r][2] = PLAYER2
        self.assertTrue(g._check_win(5, 2, PLAYER2))

    def test_diagonal_win_direct(self):
        g = Connect4()
        g.board = [[EMPTY] * 7 for _ in range(6)]
        g.board[5][0] = PLAYER1
        g.board[4][1] = PLAYER1
        g.board[3][2] = PLAYER1
        g.board[2][3] = PLAYER1
        self.assertTrue(g._check_win(2, 3, PLAYER1))

    def test_draw(self):
        g = Connect4()
        # Alternate filling the board so no one wins; end in a draw.
        cols = [0, 1, 2, 3, 4, 5, 6, 0, 1, 2, 3, 4, 5, 6, 0, 1, 2, 3, 4, 5, 6,
                0, 1, 2, 3, 4, 5, 6, 0, 1, 2, 3, 4, 5, 6, 0, 1, 2, 3, 4, 5, 6]
        for c in cols:
            g.drop(c)
        self.assertEqual(g.winner, 3)
        self.assertTrue(g.is_full())

    def test_undo(self):
        g = Connect4()
        g.drop(0)
        before = g.move_count
        self.assertTrue(g.undo())
        self.assertEqual(g.move_count, before - 1)
        self.assertEqual(g.board[5][0], EMPTY)
        self.assertEqual(g.current, PLAYER1)

    def test_to_state(self):
        g = Connect4()
        g.drop(0)
        s = g.to_state()
        self.assertEqual(s["rows"], 6)
        self.assertEqual(s["columns"], 7)
        self.assertEqual(s["current"], PLAYER2)
        self.assertEqual(s["winner"], EMPTY)

    def test_available_columns(self):
        g = Connect4()
        self.assertEqual(g.available_columns(), [0, 1, 2, 3, 4, 5, 6])
        for _ in range(6):
            g.drop(0)
        self.assertEqual(g.available_columns(), [1, 2, 3, 4, 5, 6])


# ------------------------------------------------------------------ AI
class Connect4AITests(SimpleTestCase):
    def test_chooses_valid_column(self):
        ai = Connect4AI(side=PLAYER2)
        g = Connect4()
        col = ai.choose_move(g)
        self.assertIn(col, g.available_columns())

    def test_ai_does_not_cheat(self):
        """The AI plays within the rules (its move is legal)."""
        ai = Connect4AI(side=PLAYER2)
        g = Connect4()
        col = ai.choose_move(g)
        self.assertTrue(g.is_valid_move(col))
        g.drop(col)
        # the piece landed in the chosen column at the bottom
        self.assertNotEqual(g.board[5][col], EMPTY)

    def test_ai_blocks_opponent_win(self):
        """If the opponent has 3 in a row, the AI blocks the 4th cell."""
        ai = Connect4AI(side=PLAYER2)
        g = Connect4()
        g.board = [[EMPTY] * 7 for _ in range(6)]
        # opponent (P1) has bottom row cols 0,1,2 -> threat at col 3
        g.board[5][0] = PLAYER1
        g.board[5][1] = PLAYER1
        g.board[5][2] = PLAYER1
        g.current = PLAYER2
        col = ai.choose_move(g)
        self.assertEqual(col, 3)

    def test_ai_takes_winning_move(self):
        """The AI wins immediately when it has 3 in a row."""
        ai = Connect4AI(side=PLAYER1)
        g = Connect4()
        g.board = [[EMPTY] * 7 for _ in range(6)]
        g.board[5][0] = PLAYER1
        g.board[5][1] = PLAYER1
        g.board[5][2] = PLAYER1
        g.current = PLAYER1
        col = ai.choose_move(g)
        self.assertEqual(col, 3)
        g.drop(col)
        self.assertEqual(g.winner, PLAYER1)

    def test_ai_full_board(self):
        ai = Connect4AI(side=PLAYER2)
        g = Connect4()
        for c in [0, 1, 2, 3, 4, 5, 6, 0, 1, 2, 3, 4, 5, 6, 0, 1, 2, 3, 4, 5, 6,
                  0, 1, 2, 3, 4, 5, 6, 0, 1, 2, 3, 4, 5, 6, 0, 1, 2, 3, 4, 5, 6]:
            g.drop(c)
        self.assertEqual(ai.choose_move(g), -1)


# ------------------------------------------------------------------ API
class Connect4ApiTests(TestCase):
    def setUp(self):
        self.client = APIClient()
        self.user = User.objects.create_user(
            email="c4@x.com", username="c4", password="strongpass1"
        )
        self.other = User.objects.create_user(
            email="c4b@x.com", username="c4b", password="strongpass1"
        )
        self._login(self.user)

    def _login(self, user):
        r = self.client.post("/api/auth/login/", {
            "email": user.email, "password": "strongpass1",
        }, format="json")
        token = r.json()["tokens"]["access"]
        self.client.credentials(HTTP_AUTHORIZATION="Bearer " + token)

    def test_create_connect4_session(self):
        r = self.client.post("/api/games/sessions/", {
            "name": "c4match", "kind": "connect4", "ai": True,
        })
        self.assertEqual(r.status_code, 201)
        session = GameSession.objects.get(name="c4match")
        self.assertEqual(session.kind, GameSession.CONNECT4)
        self.assertIn("board", r.json()["state"])
        self.assertEqual(r.json()["kind"], "connect4")

    def test_create_invalid_kind(self):
        r = self.client.post("/api/games/sessions/", {
            "name": "bad", "kind": "bogus",
        })
        self.assertEqual(r.status_code, 400)

    def test_default_kind_is_pong(self):
        r = self.client.post("/api/games/sessions/", {"name": "pong1"})
        self.assertEqual(r.status_code, 201)
        session = GameSession.objects.get(name="pong1")
        self.assertEqual(session.kind, GameSession.PONG)

    def test_drop_move(self):
        s = GameSession.objects.create(
            name="c4drop", kind=GameSession.CONNECT4,
            player1=self.user, status=GameSession.PLAYING,
        )
        r = self.client.post(f"/api/games/sessions/{s.id}/move/",
                             {"column": 3, "board": [[0] * 7 for _ in range(6)],
                              "current": 1, "winner": 0, "move_count": 0},
                             format="json")
        self.assertEqual(r.status_code, 200)
        state = r.json()["state"]
        self.assertEqual(state["board"][5][3], PLAYER1)
        self.assertEqual(state["current"], PLAYER2)

    def test_drop_invalid_column(self):
        s = GameSession.objects.create(
            name="c4bad", kind=GameSession.CONNECT4,
            player1=self.user, status=GameSession.PLAYING,
        )
        r = self.client.post(f"/api/games/sessions/{s.id}/move/",
                             {"column": 99, "board": [[0] * 7 for _ in range(6)],
                              "current": 1, "winner": 0, "move_count": 0},
                             format="json")
        self.assertEqual(r.status_code, 400)

    def test_drop_on_pong_session_rejected(self):
        s = GameSession.objects.create(
            name="pongdrop", kind=GameSession.PONG,
            player1=self.user, status=GameSession.PLAYING,
        )
        r = self.client.post(f"/api/games/sessions/{s.id}/move/", {"column": 0})
        self.assertEqual(r.status_code, 400)

    def test_finish_session_records_history(self):
        s = GameSession.objects.create(
            name="c4finish", kind=GameSession.CONNECT4,
            player1=self.user, player2=self.other, status=GameSession.PLAYING,
        )
        r = self.client.post(f"/api/games/sessions/{s.id}/finish/",
                             {"winner": self.user.id}, format="json")
        self.assertEqual(r.status_code, 200)
        self.user.refresh_from_db()
        self.other.refresh_from_db()
        self.assertEqual(self.user.wins, 1)
        self.assertEqual(self.other.losses, 1)
        self.assertTrue(MatchHistory.objects.filter(
            user=self.user, result="win").exists())
        self.assertTrue(MatchHistory.objects.filter(
            user=self.other, result="loss").exists())
        s.refresh_from_db()
        self.assertEqual(s.status, GameSession.FINISHED)

    def test_finish_invalid_winner(self):
        s = GameSession.objects.create(
            name="c4badfin", kind=GameSession.CONNECT4,
            player1=self.user, player2=self.other, status=GameSession.PLAYING,
        )
        r = self.client.post(f"/api/games/sessions/{s.id}/finish/",
                             {"winner": 999}, format="json")
        self.assertEqual(r.status_code, 400)

    def test_connect4_matchmaking(self):
        r = self.client.post("/api/games/connect4/matchmaking/", {})
        self.assertEqual(r.status_code, 200)
        self.assertFalse(r.json()["matched"])
        # second player joins -> match created
        self._login(self.other)
        r = self.client.post("/api/games/connect4/matchmaking/", {})
        self.assertTrue(r.json()["matched"])
        self.assertIn("session_id", r.json())
        session = GameSession.objects.get(id=r.json()["session_id"])
        self.assertEqual(session.kind, GameSession.CONNECT4)
