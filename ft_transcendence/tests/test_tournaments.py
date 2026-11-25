"""
Comprehensive tests for the tournaments API.

Covers: create tournament, register participants with aliases, start (bracket
generation), record match results (stats + match history), matchmaking queue.
"""
from django.test import TestCase
from rest_framework.test import APIClient

from accounts.models import User, MatchHistory
from tournaments.models import Tournament, Participant, Match


class TournamentsApiTests(TestCase):
    def setUp(self):
        self.client = APIClient()
        self.users = [
            User.objects.create_user(
                email=f"t{i}@x.com", username=f"t{i}", password="strongpass1"
            )
            for i in range(4)
        ]
        self._login(self.users[0])

    def _login(self, user):
        r = self.client.post("/api/auth/login/", {
            "email": user.email, "password": "strongpass1",
        }, format="json")
        token = r.json()["tokens"]["access"]
        self.client.credentials(HTTP_AUTHORIZATION="Bearer " + token)

    def test_create_tournament(self):
        r = self.client.post("/api/tournaments/", {"name": "Champs"})
        self.assertEqual(r.status_code, 201)
        self.assertTrue(Tournament.objects.filter(name="Champs").exists())

    def test_register_participant(self):
        t = Tournament.objects.create(name="T1")
        r = self.client.post(f"/api/tournaments/{t.id}/", {"alias": "Player1"})
        self.assertEqual(r.status_code, 201)
        self.assertTrue(Participant.objects.filter(tournament=t, user=self.users[0]).exists())

    def test_register_duplicate(self):
        t = Tournament.objects.create(name="T2")
        self.client.post(f"/api/tournaments/{t.id}/", {"alias": "P"})
        r = self.client.post(f"/api/tournaments/{t.id}/", {"alias": "P"})
        self.assertEqual(r.status_code, 409)

    def test_register_invalid_alias(self):
        t = Tournament.objects.create(name="T3")
        r = self.client.post(f"/api/tournaments/{t.id}/", {"alias": ""})
        self.assertEqual(r.status_code, 400)

    def test_start_tournament_generates_bracket(self):
        t = Tournament.objects.create(name="T4")
        # register 4 players
        for i, u in enumerate(self.users):
            self._login(u)
            self.client.post(f"/api/tournaments/{t.id}/", {"alias": f"P{i}"})
        self._login(self.users[0])
        r = self.client.post(f"/api/tournaments/{t.id}/start/")
        self.assertEqual(r.status_code, 200)
        t.refresh_from_db()
        self.assertEqual(t.status, Tournament.RUNNING)
        self.assertGreater(len(t.bracket), 0)

    def test_start_needs_two_players(self):
        t = Tournament.objects.create(name="T5")
        r = self.client.post(f"/api/tournaments/{t.id}/start/")
        self.assertEqual(r.status_code, 400)

    def test_record_match_updates_stats(self):
        t = Tournament.objects.create(name="T6", status=Tournament.RUNNING)
        u1, u2 = self.users[0], self.users[1]
        r = self.client.post(f"/api/tournaments/{t.id}/match/", {
            "player1": u1.id, "player2": u2.id,
            "winner": u1.id, "score1": 5, "score2": 3,
        })
        self.assertEqual(r.status_code, 201)
        u1.refresh_from_db()
        self.assertEqual(u1.wins, 1)
        # match history recorded for both
        self.assertTrue(MatchHistory.objects.filter(user=u1, result="win").exists())
        self.assertTrue(MatchHistory.objects.filter(user=u2, result="loss").exists())

    def test_record_match_invalid_winner(self):
        t = Tournament.objects.create(name="T7", status=Tournament.RUNNING)
        u1, u2 = self.users[0], self.users[1]
        r = self.client.post(f"/api/tournaments/{t.id}/match/", {
            "player1": u1.id, "player2": u2.id, "winner": 999,
        })
        self.assertEqual(r.status_code, 400)

    def test_matchmaking_queue(self):
        r = self.client.post("/api/tournaments/matchmaking/", {})
        self.assertEqual(r.status_code, 200)
        self.assertFalse(r.json()["matched"])
        # second user joins -> match
        self._login(self.users[1])
        r = self.client.post("/api/tournaments/matchmaking/", {})
        self.assertTrue(r.json()["matched"])

    def test_list_tournaments(self):
        Tournament.objects.create(name="Listed")
        r = self.client.get("/api/tournaments/")
        self.assertEqual(r.status_code, 200)
        self.assertIn("Listed", [t["name"] for t in r.json()["tournaments"]])

    def test_tournament_detail(self):
        t = Tournament.objects.create(name="Det")
        r = self.client.get(f"/api/tournaments/{t.id}/")
        self.assertEqual(r.status_code, 200)
        self.assertEqual(r.json()["name"], "Det")
