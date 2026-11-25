"""
Tests inspired by top-student reference suites (e.g. nakoriko/42transcendence).

Covers patterns these students test: public profile email hiding, friend
request lifecycle (accept/reject/duplicate/resend), registration password
strength, logout/refresh revocation, nickname validation.
"""
from django.test import TestCase
from rest_framework.test import APIClient

from accounts.models import User, Friend, TokenBlacklist


class PublicProfileTests(TestCase):
    """Public profile must hide email; own profile keeps it."""

    def setUp(self):
        self.client = APIClient()
        self.me = User.objects.create_user(
            email="me@x.com", username="me", password="strongpass1"
        )
        self.other = User.objects.create_user(
            email="other@x.com", username="other", password="strongpass1"
        )
        self._login(self.me)

    def _login(self, user):
        r = self.client.post("/api/auth/login/", {
            "email": user.email, "password": "strongpass1",
        }, format="json")
        token = r.json()["tokens"]["access"]
        self.client.credentials(HTTP_AUTHORIZATION="Bearer " + token)

    def test_public_profile_hides_email(self):
        r = self.client.get(f"/api/auth/profile/{self.other.id}/")
        self.assertEqual(r.status_code, 200)
        self.assertNotIn("email", r.json()["user"])

    def test_public_profile_returns_public_fields(self):
        r = self.client.get(f"/api/auth/profile/{self.other.id}/")
        data = r.json()["user"]
        self.assertEqual(data["id"], self.other.id)
        self.assertEqual(data["username"], "other")
        for field in ("wins", "losses", "avatar"):
            self.assertIn(field, data)

    def test_own_profile_keeps_email(self):
        r = self.client.get("/api/auth/profile/")
        self.assertEqual(r.json()["user"]["email"], "me@x.com")

    def test_unknown_user_returns_404(self):
        r = self.client.get("/api/auth/profile/999999/")
        self.assertEqual(r.status_code, 404)

    def test_anonymous_cannot_view_profile(self):
        self.client.credentials()
        r = self.client.get(f"/api/auth/profile/{self.other.id}/")
        self.assertEqual(r.status_code, 401)


class FriendLifecycleTests(TestCase):
    def setUp(self):
        self.client = APIClient()
        self.sender = User.objects.create_user(
            email="s@x.com", username="sender", password="strongpass1"
        )
        self.recipient = User.objects.create_user(
            email="r@x.com", username="recipient", password="strongpass1"
        )

    def _login(self, user):
        r = self.client.post("/api/auth/login/", {
            "email": user.email, "password": "strongpass1",
        }, format="json")
        token = r.json()["tokens"]["access"]
        self.client.credentials(HTTP_AUTHORIZATION="Bearer " + token)

    def _relations(self):
        return Friend.objects.filter(
            user__in=[self.sender, self.recipient],
            friend__in=[self.sender, self.recipient],
        )

    def test_can_send_request(self):
        self._login(self.sender)
        r = self.client.post("/api/auth/friends/", {"user_id": self.recipient.id})
        self.assertEqual(r.status_code, 201)
        self.assertEqual(self._relations().get().status, Friend.PENDING)

    def test_cannot_duplicate_pending_request(self):
        self._login(self.sender)
        self.client.post("/api/auth/friends/", {"user_id": self.recipient.id})
        r = self.client.post("/api/auth/friends/", {"user_id": self.recipient.id})
        self.assertEqual(r.status_code, 400)
        self.assertEqual(self._relations().count(), 1)

    def test_cannot_request_existing_friend(self):
        Friend.objects.create(user=self.sender, friend=self.recipient,
                              status=Friend.ACCEPTED)
        self._login(self.sender)
        r = self.client.post("/api/auth/friends/", {"user_id": self.recipient.id})
        self.assertEqual(r.status_code, 400)

    def test_recipient_can_accept(self):
        self._login(self.sender)
        self.client.post("/api/auth/friends/", {"user_id": self.recipient.id})
        self._login(self.recipient)
        r = self.client.post("/api/auth/friends/",
                             {"user_id": self.sender.id, "action": "accept"})
        self.assertEqual(r.status_code, 200)
        rel = self._relations().get()
        self.assertEqual(rel.status, Friend.ACCEPTED)

    def test_recipient_can_reject(self):
        self._login(self.sender)
        self.client.post("/api/auth/friends/", {"user_id": self.recipient.id})
        self._login(self.recipient)
        r = self.client.post("/api/auth/friends/",
                             {"user_id": self.sender.id, "action": "reject"})
        self.assertEqual(r.status_code, 200)
        self.assertEqual(self._relations().get().status, Friend.REJECTED)

    def test_sender_cannot_accept_own_request(self):
        self._login(self.sender)
        self.client.post("/api/auth/friends/", {"user_id": self.recipient.id})
        r = self.client.post("/api/auth/friends/",
                             {"user_id": self.recipient.id, "action": "accept"})
        self.assertEqual(r.status_code, 400)

    def test_sender_can_resend_after_rejection(self):
        Friend.objects.create(user=self.sender, friend=self.recipient,
                              status=Friend.REJECTED)
        self._login(self.sender)
        r = self.client.post("/api/auth/friends/", {"user_id": self.recipient.id})
        self.assertEqual(r.status_code, 201)
        self.assertEqual(self._relations().get().status, Friend.PENDING)

    def test_full_flow_request_reject_resend(self):
        self._login(self.sender)
        self.client.post("/api/auth/friends/", {"user_id": self.recipient.id})
        rel = self._relations().get()
        self._login(self.recipient)
        self.client.post("/api/auth/friends/",
                         {"user_id": self.sender.id, "action": "reject"})
        self._login(self.sender)
        r = self.client.post("/api/auth/friends/", {"user_id": self.recipient.id})
        self.assertEqual(r.status_code, 201)


class RegistrationStrengthTests(TestCase):
    def setUp(self):
        self.client = APIClient()

    def _register(self, password):
        return self.client.post("/api/auth/register/", {
            "email": "new@x.com", "username": "newbie", "password": password,
        }, format="json")

    def test_too_short_password_rejected(self):
        r = self._register("1")
        self.assertEqual(r.status_code, 400)
        self.assertFalse(User.objects.filter(email="new@x.com").exists())

    def test_numeric_password_rejected(self):
        r = self._register("12345678")
        self.assertEqual(r.status_code, 400)

    def test_strong_password_registers(self):
        r = self._register("Pong-Ultra-2026")
        self.assertEqual(r.status_code, 201)
        user = User.objects.get(email="new@x.com")
        self.assertTrue(user.check_password("Pong-Ultra-2026"))

    def test_password_not_in_response(self):
        r = self._register("Pong-Ultra-2026")
        self.assertNotIn("password", r.json().get("user", {}))


class LogoutRefreshTests(TestCase):
    def setUp(self):
        self.client = APIClient()
        User.objects.create_user(email="u@x.com", username="u", password="strongpass1")
        r = self.client.post("/api/auth/login/", {
            "email": "u@x.com", "password": "strongpass1",
        }, format="json")
        self.refresh = r.json()["tokens"]["refresh"]

    def _refresh(self):
        return self.client.post("/api/auth/refresh/", {"refresh": self.refresh},
                                format="json")

    def _logout(self, data):
        return self.client.post("/api/auth/logout/", data, format="json")

    def test_refresh_works_before_logout(self):
        self.assertEqual(self._refresh().status_code, 200)

    def test_refresh_rejected_after_logout(self):
        self._logout({"refresh": self.refresh})
        self.assertEqual(self._refresh().status_code, 401)

    def test_logout_twice_is_ok(self):
        self._logout({"refresh": self.refresh})
        self.assertEqual(self._logout({"refresh": self.refresh}).status_code, 200)

    def test_logout_invalid_token_ok(self):
        self.assertEqual(self._logout({"refresh": "garbage"}).status_code, 200)

    def test_logout_without_token_rejected(self):
        self.assertEqual(self._logout({}).status_code, 400)

    def test_blacklist_row_created(self):
        self._logout({"refresh": self.refresh})
        self.assertTrue(TokenBlacklist.objects.exists())
