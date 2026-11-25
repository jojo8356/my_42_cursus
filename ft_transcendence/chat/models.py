"""
Chat models: channels (public/private/password-protected) and messages.

Direct messages and blocking live in the accounts app (Message, Block) since
they are tied to users. Channels provide the group-chat surface.
"""
from django.conf import settings
from django.db import models
from django.utils import timezone


class Channel(models.Model):
    PUBLIC = "public"
    PRIVATE = "private"
    PASSWORD = "password"
    TYPE_CHOICES = [(PUBLIC, "Public"), (PRIVATE, "Private"), (PASSWORD, "Password")]

    name = models.CharField(max_length=64, unique=True)
    type = models.CharField(max_length=10, choices=TYPE_CHOICES, default=PUBLIC)
    password = models.CharField(max_length=128, blank=True, default="")
    created_at = models.DateTimeField(default=timezone.now)

    def __str__(self):
        return self.name


class ChannelMember(models.Model):
    channel = models.ForeignKey(Channel, related_name="members", on_delete=models.CASCADE)
    user = models.ForeignKey(settings.AUTH_USER_MODEL, related_name="channel_memberships", on_delete=models.CASCADE)
    joined_at = models.DateTimeField(auto_now_add=True)

    class Meta:
        unique_together = ("channel", "user")


class ChannelMessage(models.Model):
    channel = models.ForeignKey(Channel, related_name="messages", on_delete=models.CASCADE)
    sender = models.ForeignKey(settings.AUTH_USER_MODEL, related_name="channel_messages", on_delete=models.CASCADE)
    content = models.TextField()
    created_at = models.DateTimeField(default=timezone.now)

    class Meta:
        ordering = ["created_at"]

    def __str__(self):
        return f"[{self.channel}] {self.sender}: {self.content[:30]}"
