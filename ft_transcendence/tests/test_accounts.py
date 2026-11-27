"""
Comprehensive tests for the accounts API.

Covers: registration validation, login, JWT access/refresh, OAuth 2.0 with 42
(mock provider), profile, friends, blocking, direct messages, match history,
two-factor auth, GDPR anonymization and deletion.
"""
from django.core.files.uploadedfile import SimpleUploadedFile
from django.test import TestCase
from rest_framework.test import APIClient

from accounts.models import User, Friend, Block, Message, MatchHistory


class AccountsApiTests(TestCase):
    def setUp(self):
        self.client = APIClient()

    # ------------------------------------------------------------------ register
    def test_register_success(self):
        r = self.client.post("/api/auth/register/", {
            "email": "alice@example.com", "username": "alice",
            "password": "strongpass1",
        }, format="json")
        self.assertEqual(r.status_code, 201)
        self.assertTrue(User.objects.filter(email="alice@example.com").exists())
        self.assertIn("tokens", r.json())

    def test_register_weak_password(self):
        r = self.client.post("/api/auth/register/", {
            "email": "a@b.com", "username": "ab", "password": "123",
        }, format="json")
        self.assertEqual(r.status_code, 400)

    def test_register_invalid_username(self):
        r = self.client.post("/api/auth/register/", {
            "email": "a@b.com", "username": "x", "password": "strongpass1",
        }, format="json")
        self.assertEqual(r.status_code, 400)

    def test_register_duplicate_email(self):
        User.objects.create_user(email="dup@x.com", username="dup", password="strongpass1")
        r = self.client.post("/api/auth/register/", {
            "email": "dup@x.com", "username": "dup2", "password": "strongpass1",
        }, format="json")
        self.assertEqual(r.status_code, 409)

    # ------------------------------------------------------------------ login
    def test_login_success(self):
        User.objects.create_user(email="bob@x.com", username="bob", password="strongpass1")
        r = self.client.post("/api/auth/login/", {
            "email": "bob@x.com", "password": "strongpass1",
        }, format="json")
        self.assertEqual(r.status_code, 200)
        self.assertIn("tokens", r.json())

    def test_login_wrong_password(self):
        User.objects.create_user(email="bob@x.com", username="bob", password="strongpass1")
        r = self.client.post("/api/auth/login/", {
            "email": "bob@x.com", "password": "wrong",
        }, format="json")
        self.assertEqual(r.status_code, 401)

    # ------------------------------------------------------------------ JWT
    def _auth_headers(self, user):
        r = self.client.post("/api/auth/login/", {
            "email": user.email, "password": "strongpass1",
        }, format="json")
        token = r.json()["tokens"]["access"]
        return {"HTTP_AUTHORIZATION": "Bearer " + token}

    def test_protected_requires_auth(self):
        r = self.client.get("/api/auth/profile/")
        self.assertEqual(r.status_code, 401)  # DRF returns 401 with JWT auth

    def test_profile_with_token(self):
        u = User.objects.create_user(email="c@x.com", username="c", password="strongpass1")
        r = self.client.get("/api/auth/profile/", **self._auth_headers(u))
        self.assertEqual(r.status_code, 200)
        self.assertEqual(r.json()["user"]["username"], "c")

    def test_refresh_token(self):
        u = User.objects.create_user(email="d@x.com", username="d", password="strongpass1")
        login = self.client.post("/api/auth/login/", {
            "email": "d@x.com", "password": "strongpass1",
        }, format="json").json()
        refresh = login["tokens"]["refresh"]
        r = self.client.post("/api/auth/refresh/", {"refresh": refresh}, format="json")
        self.assertEqual(r.status_code, 200)
        self.assertIn("access", r.json())

    def test_refresh_invalid(self):
        r = self.client.post("/api/auth/refresh/", {"refresh": "garbage"}, format="json")
        self.assertEqual(r.status_code, 401)

    # ------------------------------------------------------------------ OAuth
    def test_oauth_authorize_url(self):
        r = self.client.get("/api/auth/oauth/authorize/")
        self.assertEqual(r.status_code, 200)
        self.assertIn("authorization_url", r.json())

    def test_oauth_callback_creates_user(self):
        # use the mock provider path via direct function
        from accounts.oauth import mock_oauth_login
        data = mock_oauth_login("testcode")
        self.assertEqual(data["login"], "mock42user")
        # simulate the view logic
        user, created = User.objects.get_or_create(
            oauth_id=data["login"],
            defaults={"email": f"{data['login']}@student.42.fr",
                      "username": data["display_name"]},
        )
        self.assertTrue(created)
        self.assertEqual(user.email, "mock42user@student.42.fr")

    def test_oauth_callback_bad_code(self):
        from accounts.oauth import mock_oauth_login, OAuthError
        with self.assertRaises(OAuthError):
            mock_oauth_login("bad_code")

    # ------------------------------------------------------------------ profile update
    def test_update_profile(self):
        u = User.objects.create_user(email="e@x.com", username="e", password="strongpass1")
        r = self.client.put("/api/auth/profile/", {
            "username": "newname", "language": "fr",
        }, **self._auth_headers(u))
        self.assertEqual(r.status_code, 200)
        u.refresh_from_db()
        self.assertEqual(u.username, "newname")
        self.assertEqual(u.language, "fr")

    def test_update_profile_duplicate_username(self):
        User.objects.create_user(email="f@x.com", username="taken", password="strongpass1")
        u = User.objects.create_user(email="g@x.com", username="free", password="strongpass1")
        r = self.client.put("/api/auth/profile/", {"username": "taken"},
                            **self._auth_headers(u))
        self.assertEqual(r.status_code, 409)

    # ------------------------------------------------------------------ friends
    def test_friend_request_and_accept(self):
        u1 = User.objects.create_user(email="h1@x.com", username="h1", password="strongpass1")
        u2 = User.objects.create_user(email="h2@x.com", username="h2", password="strongpass1")
        # u1 sends request to u2
        r = self.client.post("/api/auth/friends/", {"user_id": u2.id},
                             **self._auth_headers(u1))
        self.assertEqual(r.json()["status"], Friend.PENDING)
        # u2 accepts
        r = self.client.post("/api/auth/friends/", {"user_id": u1.id, "accept": True},
                             **self._auth_headers(u2))
        self.assertEqual(r.json()["status"], Friend.ACCEPTED)
        # u1 sees u2 as friend
        r = self.client.get("/api/auth/friends/", **self._auth_headers(u1))
        self.assertIn(u2.id, [f["id"] for f in r.json()["friends"]])

    def test_cannot_friend_self(self):
        u = User.objects.create_user(email="i@x.com", username="i", password="strongpass1")
        r = self.client.post("/api/auth/friends/", {"user_id": u.id},
                             **self._auth_headers(u))
        self.assertEqual(r.status_code, 400)

    # ------------------------------------------------------------------ blocking
    def test_block_user(self):
        u1 = User.objects.create_user(email="j1@x.com", username="j1", password="strongpass1")
        u2 = User.objects.create_user(email="j2@x.com", username="j2", password="strongpass1")
        r = self.client.post("/api/auth/blocks/", {"user_id": u2.id},
                             **self._auth_headers(u1))
        self.assertTrue(r.json()["blocked"])
        r = self.client.delete("/api/auth/blocks/", {"user_id": u2.id},
                               **self._auth_headers(u1))
        self.assertFalse(r.json()["blocked"])

    # ------------------------------------------------------------------ messages
    def test_send_and_get_dm(self):
        u1 = User.objects.create_user(email="k1@x.com", username="k1", password="strongpass1")
        u2 = User.objects.create_user(email="k2@x.com", username="k2", password="strongpass1")
        r = self.client.post(f"/api/auth/messages/{u2.id}/", {"content": "hi"},
                             **self._auth_headers(u1))
        self.assertEqual(r.status_code, 201)
        r = self.client.get(f"/api/auth/messages/{u2.id}/", **self._auth_headers(u1))
        self.assertEqual(len(r.json()["messages"]), 1)

    def test_blocked_user_cannot_receive(self):
        u1 = User.objects.create_user(email="l1@x.com", username="l1", password="strongpass1")
        u2 = User.objects.create_user(email="l2@x.com", username="l2", password="strongpass1")
        # u2 blocks u1
        self.client.post("/api/auth/blocks/", {"user_id": u1.id},
                         **self._auth_headers(u2))
        # u1 cannot send to u2
        r = self.client.post(f"/api/auth/messages/{u2.id}/", {"content": "hi"},
                             **self._auth_headers(u1))
        self.assertEqual(r.status_code, 403)

    def test_message_xss_sanitized(self):
        u1 = User.objects.create_user(email="m1@x.com", username="m1", password="strongpass1")
        u2 = User.objects.create_user(email="m2@x.com", username="m2", password="strongpass1")
        self.client.post(f"/api/auth/messages/{u2.id}/",
                         {"content": "<script>alert(1)</script>"},
                         **self._auth_headers(u1))
        msg = Message.objects.first()
        self.assertNotIn("<script>", msg.content)

    # ------------------------------------------------------------------ match history
    def test_match_history_empty(self):
        u = User.objects.create_user(email="n@x.com", username="n", password="strongpass1")
        r = self.client.get("/api/auth/matches/", **self._auth_headers(u))
        self.assertEqual(r.status_code, 200)
        self.assertEqual(r.json()["matches"], [])

    # ------------------------------------------------------------------ 2FA
    def test_two_factor_enable_disable(self):
        u = User.objects.create_user(email="o@x.com", username="o", password="strongpass1")
        r = self.client.post("/api/auth/2fa/enable/", **self._auth_headers(u))
        self.assertTrue(r.json()["enabled"])
        u.refresh_from_db()
        self.assertTrue(u.two_factor_enabled)
        self.assertIsNotNone(u.totp_secret)
        r = self.client.post("/api/auth/2fa/disable/", **self._auth_headers(u))
        self.assertFalse(r.json()["enabled"])

    # ------------------------------------------------------------------ GDPR
    def test_anonymize(self):
        u = User.objects.create_user(email="p@x.com", username="p", password="strongpass1")
        r = self.client.post("/api/auth/gdpr/anonymize/", **self._auth_headers(u))
        self.assertTrue(r.json()["anonymized"])
        u.refresh_from_db()
        self.assertTrue(u.is_anonymized)
        self.assertNotIn("p@x.com", u.email)

    def test_delete_account_removes_data(self):
        u1 = User.objects.create_user(email="q1@x.com", username="q1", password="strongpass1")
        u2 = User.objects.create_user(email="q2@x.com", username="q2", password="strongpass1")
        Friend.objects.create(user=u1, friend=u2, status=Friend.ACCEPTED)
        Block.objects.create(user=u1, blocked_user=u2)
        Message.objects.create(sender=u1, recipient=u2, content="x")
        MatchHistory.objects.create(user=u1, opponent=u2, result="win")
        r = self.client.post("/api/auth/gdpr/delete/", **self._auth_headers(u1))
        self.assertTrue(r.json()["deleted"])
        self.assertFalse(User.objects.filter(id=u1.id).exists())
        self.assertFalse(Friend.objects.filter(user=u1).exists())
        self.assertFalse(Message.objects.filter(sender=u1).exists())
        self.assertFalse(MatchHistory.objects.filter(user=u1).exists())

    def test_password_is_hashed(self):
        u = User.objects.create_user(email="r@x.com", username="r", password="strongpass1")
        self.assertNotEqual(u.password, "strongpass1")
        self.assertTrue(u.password.startswith("pbkdf2_"))


class AvatarUploadTests(TestCase):
    """Avatar upload endpoint (Standard user management module)."""

    def setUp(self):
        self.client = APIClient()
        self.user = User.objects.create_user(
            email="av@x.com", username="av", password="strongpass1"
        )
        r = self.client.post("/api/auth/login/", {
            "email": "av@x.com", "password": "strongpass1",
        }, format="json")
        token = r.json()["tokens"]["access"]
        self.client.credentials(HTTP_AUTHORIZATION="Bearer " + token)

    def _png(self):
        return SimpleUploadedFile("avatar.png", b"\\x89PNG\\r\\n\\x1a\\n",
                                  content_type="image/png")

    def test_avatar_upload_success(self):
        r = self.client.post("/api/auth/avatar/upload/",
                             {"avatar": self._png()})
        self.assertEqual(r.status_code, 200)
        self.user.refresh_from_db()
        self.assertTrue(self.user.avatar)
        self.assertIn("avatar", r.json())

    def test_avatar_upload_invalid_type(self):
        bad = SimpleUploadedFile("avatar.txt", b"not an image",
                                 content_type="text/plain")
        r = self.client.post("/api/auth/avatar/upload/", {"avatar": bad})
        self.assertEqual(r.status_code, 400)

    def test_avatar_upload_missing_file(self):
        r = self.client.post("/api/auth/avatar/upload/", {})
        self.assertEqual(r.status_code, 400)

    def test_avatar_upload_requires_auth(self):
        self.client.credentials()
        r = self.client.post("/api/auth/avatar/upload/",
                             {"avatar": self._png()})
        self.assertIn(r.status_code, (401, 403))
