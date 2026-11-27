"""
Comprehensive tests for the games API.

Covers: customization presets, power-up templates, game session creation,
joining, and the server-side AI decision endpoint.
"""
from django.test import TestCase
from rest_framework.test import APIClient

from accounts.models import User
from games.models import GameSession, GameCustomization, PowerUpTemplate
from games.room import get_room, drop_room


class GamesApiTests(TestCase):
    def setUp(self):
        self.client = APIClient()
        self.user = User.objects.create_user(
            email="g@x.com", username="g", password="strongpass1"
        )
        r = self.client.post("/api/auth/login/", {
            "email": "g@x.com", "password": "strongpass1",
        }, format="json")
        token = r.json()["tokens"]["access"]
        self.client.credentials(HTTP_AUTHORIZATION="Bearer " + token)

    def test_create_session(self):
        r = self.client.post("/api/games/sessions/", {"name": "match1", "ai": True})
        self.assertEqual(r.status_code, 201)
        self.assertTrue(GameSession.objects.filter(name="match1").exists())
        self.assertIn("state", r.json())

    def test_join_session(self):
        s = GameSession.objects.create(name="s1", player1=self.user)
        r = self.client.post(f"/api/games/sessions/{s.id}/join/")
        self.assertEqual(r.status_code, 200)
        s.refresh_from_db()
        self.assertEqual(s.player2, self.user)

    def test_join_full_session(self):
        u2 = User.objects.create_user(email="g2@x.com", username="g2", password="strongpass1")
        s = GameSession.objects.create(name="s2", player1=self.user, player2=u2)
        r = self.client.post(f"/api/games/sessions/{s.id}/join/")
        self.assertEqual(r.status_code, 409)

    def test_join_missing_session(self):
        r = self.client.post("/api/games/sessions/9999/join/")
        self.assertEqual(r.status_code, 404)

    def test_customizations_default(self):
        r = self.client.get("/api/games/customizations/")
        self.assertEqual(r.status_code, 200)
        self.assertIn("customizations", r.json())

    def test_create_customization(self):
        r = self.client.post("/api/games/customizations/", {
            "name": "hard", "enable_powerups": True, "enable_attacks": True,
        })
        self.assertEqual(r.status_code, 201)
        self.assertTrue(GameCustomization.objects.filter(name="hard").exists())

    def test_powerups_seeded(self):
        r = self.client.get("/api/games/powerups/")
        self.assertEqual(r.status_code, 200)
        kinds = {p["kind"] for p in r.json()["powerups"]}
        self.assertIn("speed", kinds)
        self.assertIn("size", kinds)
        self.assertIn("slow_ball", kinds)

    def test_ai_decision_endpoint(self):
        r = self.client.post("/api/games/ai/decision/", {
            "side": "right", "elapsed": 1.0,
            "left_paddle_y": 300, "right_paddle_y": 450,
            "ball_x": 600, "ball_y": 100, "ball_vx": 0, "ball_vy": 0,
        })
        self.assertEqual(r.status_code, 200)
        self.assertIn("direction", r.json())
        # ball is above paddle => AI moves up (-1)
        self.assertEqual(r.json()["direction"], -1)

    def test_ai_decision_no_action_before_refresh(self):
        r = self.client.post("/api/games/ai/decision/", {
            "side": "right", "elapsed": 0.5,
            "left_paddle_y": 300, "right_paddle_y": 300,
            "ball_x": 600, "ball_y": 100, "ball_vx": 0, "ball_vy": 0,
        })
        self.assertEqual(r.status_code, 200)
        self.assertEqual(r.json()["direction"], 0)


class GameRoomTests(TestCase):
    """Authoritative game rooms for the Remote players module.

    Two distant clients connect to the same session; the server owns the
    shared game state and broadcasts it to every participant.
    """

    def tearDown(self):
        # Clean the process-wide room cache between tests.
        from games import room as room_mod
        for key in list(room_mod._ROOMS.keys()):
            room_mod._ROOMS.pop(key, None)

    def test_room_is_shared_per_session(self):
        r1 = get_room(1, kind="pong")
        r2 = get_room(1, kind="pong")
        self.assertIs(r1, r2)
        self.assertEqual(r1.group_name, "game_1")

    def test_pong_room_move_and_step(self):
        room = get_room(2, kind="pong", is_ai=True)
        room.move("left", 1)
        room.step()
        state = room.state()
        self.assertEqual(state["state"], "playing")
        self.assertIn("ball", state)

    def test_pong_room_powerup(self):
        room = get_room(3, kind="pong")
        self.assertTrue(room.powerup("left", "speed"))
        # power-ups are accepted; the game keeps running
        self.assertTrue(room.state()["state"] in ("playing", "waiting", "over"))

    def test_connect4_room_drop_and_ai(self):
        room = get_room(4, kind="connect4", is_ai=True)
        state = room.state()
        self.assertEqual(state["current"], 1)
        # drop in column 3 -> piece lands at the bottom (row 5)
        self.assertTrue(room.drop(3))
        state = room.state()
        self.assertEqual(state["board"][5][3], 1)

    def test_connect4_room_full_column_rejected(self):
        room = get_room(5, kind="connect4")
        for _ in range(6):
            room.drop(0)
        # column 0 is now full -> the 7th drop is rejected
        self.assertFalse(room.drop(0))

    def test_drop_room_clears_cache(self):
        room = get_room(6, kind="pong")
        self.assertIsNotNone(room)
        drop_room(6)
        # a fresh room is created on next access
        fresh = get_room(6, kind="pong")
        self.assertIsNot(room, fresh)
