"""
Comprehensive tests for the chat API.

Covers: channel creation (public/private/password), joining with password
verification, membership, channel messages, and XSS sanitization.
"""
from django.test import TestCase
from rest_framework.test import APIClient

from accounts.models import User
from chat.models import Channel, ChannelMember, ChannelMessage


class ChatApiTests(TestCase):
    def setUp(self):
        self.client = APIClient()
        self.user = User.objects.create_user(
            email="c@x.com", username="c", password="strongpass1"
        )
        self.other = User.objects.create_user(
            email="c2@x.com", username="c2", password="strongpass1"
        )
        self._login(self.user)

    def _login(self, user):
        r = self.client.post("/api/auth/login/", {
            "email": user.email, "password": "strongpass1",
        }, format="json")
        token = r.json()["tokens"]["access"]
        self.client.credentials(HTTP_AUTHORIZATION="Bearer " + token)

    def test_create_public_channel(self):
        r = self.client.post("/api/chat/channels/", {"name": "general", "type": "public"})
        self.assertEqual(r.status_code, 201)
        self.assertTrue(Channel.objects.filter(name="general").exists())

    def test_create_password_channel(self):
        r = self.client.post("/api/chat/channels/", {
            "name": "secret", "type": "password", "password": "pw123",
        })
        self.assertEqual(r.status_code, 201)
        c = Channel.objects.get(name="secret")
        self.assertEqual(c.type, Channel.PASSWORD)

    def test_create_channel_invalid_name(self):
        r = self.client.post("/api/chat/channels/", {"name": ""})
        self.assertEqual(r.status_code, 400)

    def test_create_channel_invalid_type(self):
        r = self.client.post("/api/chat/channels/", {"name": "x", "type": "bogus"})
        self.assertEqual(r.status_code, 400)

    def test_join_password_channel_correct(self):
        self.client.post("/api/chat/channels/", {
            "name": "pwchan", "type": "password", "password": "pw123",
        })
        c = Channel.objects.get(name="pwchan")
        # other user joins with correct password
        self._login(self.other)
        r = self.client.post(f"/api/chat/channels/{c.id}/join/", {"password": "pw123"})
        self.assertEqual(r.status_code, 200)

    def test_join_password_channel_wrong(self):
        self.client.post("/api/chat/channels/", {
            "name": "pwchan2", "type": "password", "password": "pw123",
        })
        c = Channel.objects.get(name="pwchan2")
        self._login(self.other)
        r = self.client.post(f"/api/chat/channels/{c.id}/join/", {"password": "wrong"})
        self.assertEqual(r.status_code, 403)

    def test_post_message_requires_membership(self):
        c = Channel.objects.create(name="mem", type=Channel.PUBLIC)
        ChannelMember.objects.create(channel=c, user=self.user)
        # other user is not a member
        self._login(self.other)
        r = self.client.post(f"/api/chat/channels/{c.id}/messages/", {"content": "hi"})
        self.assertEqual(r.status_code, 403)

    def test_post_and_get_message(self):
        c = Channel.objects.create(name="msgchan", type=Channel.PUBLIC)
        ChannelMember.objects.create(channel=c, user=self.user)
        r = self.client.post(f"/api/chat/channels/{c.id}/messages/", {"content": "hello"})
        self.assertEqual(r.status_code, 201)
        r = self.client.get(f"/api/chat/channels/{c.id}/messages/")
        self.assertEqual(r.status_code, 200)
        self.assertEqual(len(r.json()["messages"]), 1)

    def test_channel_message_xss_sanitized(self):
        c = Channel.objects.create(name="xss", type=Channel.PUBLIC)
        ChannelMember.objects.create(channel=c, user=self.user)
        self.client.post(f"/api/chat/channels/{c.id}/messages/",
                         {"content": "<script>alert(1)</script>"})
        msg = ChannelMessage.objects.first()
        self.assertNotIn("<script>", msg.content)

    def test_empty_message_rejected(self):
        c = Channel.objects.create(name="empty", type=Channel.PUBLIC)
        ChannelMember.objects.create(channel=c, user=self.user)
        r = self.client.post(f"/api/chat/channels/{c.id}/messages/", {"content": "  "})
        self.assertEqual(r.status_code, 400)

    def test_list_channels(self):
        Channel.objects.create(name="pub", type=Channel.PUBLIC)
        ChannelMember.objects.create(channel=Channel.objects.get(name="pub"), user=self.user)
        r = self.client.get("/api/chat/channels/")
        self.assertEqual(r.status_code, 200)
        self.assertIn("channels", r.json())
        self.assertIn("public", r.json())
