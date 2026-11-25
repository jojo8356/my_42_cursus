"""
Accounts app: custom user model, profiles, friends, avatars,
match history, GDPR, two-factor authentication, OAuth 2.0 with 42.
"""
import os
import uuid

from django.contrib.auth.models import (
    AbstractBaseUser, BaseUserManager, PermissionsMixin,
)
from django.db import models
from django.utils import timezone


def default_avatar_path(instance, filename):
    """Default avatar filename kept under media/avatars."""
    ext = os.path.splitext(filename)[1].lower() or ".svg"
    return f"avatars/default_{uuid.uuid4().hex}{ext}"


class UserManager(BaseUserManager):
    use_in_migrations = True

    def _create_user(self, email, password=None, **extra):
        if not email:
            raise ValueError("The given email must be set")
        email = self.normalize_email(email)
        user = self.model(email=email, **extra)
        if password:
            user.set_password(password)
        else:
            user.set_unusable_password()
        user.save(using=self._db)
        return user

    def create_user(self, email, password=None, **extra):
        extra.setdefault("is_staff", False)
        extra.setdefault("is_superuser", False)
        return self._create_user(email, password, **extra)

    def create_superuser(self, email, password=None, **extra):
        extra.setdefault("is_staff", True)
        extra.setdefault("is_superuser", True)
        if extra.get("is_staff") is not True:
            raise ValueError("Superuser must have is_staff=True.")
        if extra.get("is_superuser") is not True:
            raise ValueError("Superuser must have is_superuser=True.")
        return self._create_user(email, password, **extra)


class User(AbstractBaseUser, PermissionsMixin):
    """Custom user. Email is the unique identifier; username is the display
    alias used for tournaments."""
    email = models.EmailField(unique=True)
    username = models.CharField(max_length=32, unique=True)
    first_name = models.CharField(max_length=64, blank=True, default="")
    last_name = models.CharField(max_length=64, blank=True, default="")
    avatar = models.FileField(upload_to=default_avatar_path, blank=True, null=True)

    # account lifecycle / GDPR
    is_active = models.BooleanField(default=True)
    is_staff = models.BooleanField(default=False)
    is_anonymized = models.BooleanField(default=False)
    date_joined = models.DateTimeField(default=timezone.now)

    # OAuth 2.0 with 42
    oauth_id = models.CharField(max_length=64, blank=True, null=True, unique=True)
    oauth_token = models.TextField(blank=True, null=True)

    # Two-Factor Authentication (TOTP secret)
    totp_secret = models.CharField(max_length=64, blank=True, null=True)
    two_factor_enabled = models.BooleanField(default=False)

    # preferred language (multi-language module)
    language = models.CharField(max_length=10, default="en")

    # stats
    wins = models.IntegerField(default=0)
    losses = models.IntegerField(default=0)

    objects = UserManager()

    USERNAME_FIELD = "email"
    REQUIRED_FIELDS = ["username"]

    def __str__(self):
        return self.username or self.email

    def save(self, *args, **kwargs):
        if not self.username:
            self.username = self.email.split("@")[0]
        super().save(*args, **kwargs)

    @property
    def display_name(self):
        return self.username

    def anonymize(self):
        """GDPR: anonymize personal data, keep the account row for integrity."""
        self.email = f"anonymized_{uuid.uuid4().hex[:8]}@anonymized.local"
        self.username = f"anon_{uuid.uuid4().hex[:6]}"
        self.first_name = ""
        self.last_name = ""
        self.avatar = None
        self.oauth_token = None
        self.is_anonymized = True
        self.save()

    def delete_account(self):
        """GDPR: permanently delete the account and all associated data."""
        Friend.objects.filter(user=self).delete()
        Friend.objects.filter(friend=self).delete()
        Block.objects.filter(user=self).delete()
        Block.objects.filter(blocked_user=self).delete()
        Message.objects.filter(sender=self).delete()
        Message.objects.filter(recipient=self).delete()
        MatchHistory.objects.filter(user=self).delete()
        MatchHistory.objects.filter(opponent=self).delete()
        ProfileVisit.objects.filter(visitor=self).delete()
        self.delete()


class Friend(models.Model):
    PENDING = "pending"
    ACCEPTED = "accepted"
    REJECTED = "rejected"
    BLOCKED = "blocked"
    STATUS_CHOICES = [(PENDING, "Pending"), (ACCEPTED, "Accepted"),
                      (REJECTED, "Rejected"), (BLOCKED, "Blocked")]

    user = models.ForeignKey(User, related_name="friend_relations", on_delete=models.CASCADE)
    friend = models.ForeignKey(User, related_name="friend_of", on_delete=models.CASCADE)
    status = models.CharField(max_length=10, choices=STATUS_CHOICES, default=PENDING)
    created_at = models.DateTimeField(auto_now_add=True)

    class Meta:
        unique_together = ("user", "friend")

    def __str__(self):
        return f"{self.user} -> {self.friend} ({self.status})"


class Block(models.Model):
    user = models.ForeignKey(User, related_name="blocking", on_delete=models.CASCADE)
    blocked_user = models.ForeignKey(User, related_name="blocked_by", on_delete=models.CASCADE)
    created_at = models.DateTimeField(auto_now_add=True)

    class Meta:
        unique_together = ("user", "blocked_user")

    def __str__(self):
        return f"{self.user} blocks {self.blocked_user}"


class Message(models.Model):
    sender = models.ForeignKey(User, related_name="sent_messages", on_delete=models.CASCADE)
    recipient = models.ForeignKey(User, related_name="received_messages", on_delete=models.CASCADE)
    content = models.TextField()
    created_at = models.DateTimeField(auto_now_add=True)

    class Meta:
        ordering = ["created_at"]

    def __str__(self):
        return f"{self.sender} -> {self.recipient}: {self.content[:30]}"


class MatchHistory(models.Model):
    """1v1 match record with dates and details (Standard user management)."""
    user = models.ForeignKey(User, related_name="matches", on_delete=models.CASCADE)
    opponent = models.ForeignKey(User, related_name="opponent_matches", on_delete=models.CASCADE)
    result = models.CharField(max_length=10)  # "win" / "loss" / "draw"
    user_score = models.IntegerField(default=0)
    opponent_score = models.IntegerField(default=0)
    played_at = models.DateTimeField(default=timezone.now)
    tournament = models.ForeignKey(
        "tournaments.Tournament", null=True, blank=True,
        related_name="matches", on_delete=models.SET_NULL,
    )

    class Meta:
        ordering = ["-played_at"]

    def __str__(self):
        return f"{self.user} vs {self.opponent} ({self.result})"


class ProfileVisit(models.Model):
    visitor = models.ForeignKey(User, related_name="visits_made", on_delete=models.CASCADE)
    visited = models.ForeignKey(User, related_name="visited_by", on_delete=models.CASCADE)
    visited_at = models.DateTimeField(auto_now_add=True)

    def __str__(self):
        return f"{self.visitor} visited {self.visited}"


class TokenBlacklist(models.Model):
    """Revoked refresh tokens (logout). Stores a hash of the refresh token."""
    token_hash = models.CharField(max_length=128, unique=True)
    user = models.ForeignKey(User, related_name="revoked_tokens", on_delete=models.CASCADE)
    created_at = models.DateTimeField(default=timezone.now)

    def __str__(self):
        return f"revoked token for {self.user}"
