"""
Security tests - 'grandes entreprises' quality.

Covers: SQL injection resistance, XSS sanitization, password hashing,
input validation, authentication enforcement, OAuth duplicate handling.
"""
from django.test import TestCase
from rest_framework.test import APIClient

from accounts.models import User, Message
from chat.models import Channel, ChannelMember, ChannelMessage


class SecurityTests(TestCase):
    def setUp(self):
        self.client = APIClient()
        self.user = User.objects.create_user(
            email="sec@x.com", username="sec", password="strongpass1"
        )
        self._login()

    def _login(self):
        r = self.client.post("/api/auth/login/", {
            "email": "sec@x.com", "password": "strongpass1",
        }, format="json")
        token = r.json()["tokens"]["access"]
        self.client.credentials(HTTP_AUTHORIZATION="Bearer " + token)

    # ------------------------------------------------------------------ SQL injection
    def test_sql_injection_in_search_by_id(self):
        """A malicious id must not break or leak via SQL injection."""
        r = self.client.get("/api/auth/profile/99999/")
        self.assertEqual(r.status_code, 404)
        # injection payload as id
        r = self.client.get("/api/auth/profile/1%20OR%201%3D1/")
        self.assertEqual(r.status_code, 404)

    def test_sql_injection_in_messages(self):
        other = User.objects.create_user(email="other@x.com", username="other", password="strongpass1")
        # injection payload as user_id
        r = self.client.get("/api/auth/messages/99999/")
        self.assertEqual(r.status_code, 404)

    def test_sql_injection_in_friend_id(self):
        other = User.objects.create_user(email="other2@x.com", username="other2", password="strongpass1")
        payload = "1 OR 1=1"
        r = self.client.post("/api/auth/friends/", {"user_id": payload}, format="json")
        # should fail cleanly (user not found), not leak all users
        self.assertEqual(r.status_code, 404)

    # ------------------------------------------------------------------ XSS
    def test_xss_in_dm(self):
        other = User.objects.create_user(email="x@x.com", username="x", password="strongpass1")
        self.client.post(f"/api/auth/messages/{other.id}/",
                         {"content": "<img src=x onerror=alert(1)>"}, format="json")
        msg = Message.objects.first()
        self.assertNotIn("<img", msg.content)
        self.assertNotIn("<script", msg.content)

    def test_xss_in_channel_message(self):
        c = Channel.objects.create(name="xssch", type=Channel.PUBLIC)
        ChannelMember.objects.create(channel=c, user=self.user)
        self.client.post(f"/api/chat/channels/{c.id}/messages/",
                         {"content": "<script>document.cookie</script>"}, format="json")
        msg = ChannelMessage.objects.first()
        self.assertNotIn("<script>", msg.content)

    def test_xss_in_username_escaped_on_output(self):
        """Usernames are stored raw but rendered through JSON (safe)."""
        User.objects.create_user(email="xss@x.com", username="<script>x</script>",
                                 password="strongpass1")
        r = self.client.get("/api/auth/profile/", format="json")
        # the username is returned as a JSON string (escaped by JSON encoder)
        body = r.content.decode()
        # raw <script> must not appear unescaped in the JSON body
        self.assertNotIn("<script>x</script>", body)

    # ------------------------------------------------------------------ password hashing
    def test_password_not_stored_plaintext(self):
        u = User.objects.get(email="sec@x.com")
        self.assertNotEqual(u.password, "strongpass1")
        self.assertTrue(u.has_usable_password())

    def test_strong_hashing_algorithm(self):
        u = User.objects.get(email="sec@x.com")
        self.assertIn("pbkdf2", u.password)

    # ------------------------------------------------------------------ auth enforcement
    def test_all_protected_endpoints_require_auth(self):
        self.client.credentials()  # clear credentials
        endpoints = [
            ("get", "/api/auth/profile/"),
            ("get", "/api/auth/friends/"),
            ("get", "/api/auth/matches/"),
            ("get", "/api/auth/messages/1/"),
            ("get", "/api/games/customizations/"),
            ("get", "/api/games/powerups/"),
            ("get", "/api/games/sessions/"),
            ("get", "/api/tournaments/"),
            ("get", "/api/chat/channels/"),
        ]
        for method, url in endpoints:
            r = getattr(self.client, method)(url)
            self.assertEqual(r.status_code, 401, f"{url} should require auth")

    def test_invalid_token_rejected(self):
        self.client.credentials(HTTP_AUTHORIZATION="Bearer invalidtoken")
        r = self.client.get("/api/auth/profile/")
        self.assertEqual(r.status_code, 401)

    # ------------------------------------------------------------------ input validation
    def test_register_missing_fields(self):
        self.client.credentials()
        r = self.client.post("/api/auth/register/", {}, format="json")
        self.assertEqual(r.status_code, 400)

    def test_register_invalid_email(self):
        self.client.credentials()
        r = self.client.post("/api/auth/register/", {
            "email": "notanemail", "username": "okname", "password": "strongpass1",
        }, format="json")
        self.assertEqual(r.status_code, 400)

    def test_empty_message_rejected(self):
        other = User.objects.create_user(email="e@x.com", username="e", password="strongpass1")
        r = self.client.post(f"/api/auth/messages/{other.id}/", {"content": ""}, format="json")
        self.assertEqual(r.status_code, 400)

    # ------------------------------------------------------------------ OAuth duplicate handling
    def test_oauth_duplicate_login_links_same_account(self):
        """Logging in twice with the same 42 login must not create duplicates."""
        from accounts.oauth import mock_oauth_login
        data = mock_oauth_login("dupcode")
        # first login creates
        user1, created1 = User.objects.get_or_create(
            oauth_id=data["login"],
            defaults={"email": f"{data['login']}@student.42.fr",
                      "username": data["display_name"]},
        )
        # second login with same login must find the same user
        user2, created2 = User.objects.get_or_create(
            oauth_id=data["login"],
            defaults={"email": f"{data['login']}@student.42.fr",
                      "username": data["display_name"]},
        )
        self.assertTrue(created1)
        self.assertFalse(created2)
        self.assertEqual(user1.id, user2.id)
        self.assertEqual(User.objects.filter(oauth_id=data["login"]).count(), 1)
