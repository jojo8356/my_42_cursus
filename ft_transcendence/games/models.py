"""
Games models: customization options and live game sessions.
"""
from django.conf import settings
from django.db import models


class GameCustomization(models.Model):
    """Game customization options (power-ups, attacks, maps)."""
    PRESET_DEFAULT = "default"
    PRESET_POWERUPS = "powerups"

    name = models.CharField(max_length=64)
    enable_powerups = models.BooleanField(default=True)
    enable_attacks = models.BooleanField(default=False)
    enable_custom_map = models.BooleanField(default=False)
    preset = models.CharField(max_length=20, default=PRESET_DEFAULT)

    class Meta:
        unique_together = ("name",)

    def __str__(self):
        return self.name


class PowerUpTemplate(models.Model):
    KIND_CHOICES = [
        ("speed", "Paddle speed"),
        ("size", "Paddle size"),
        ("slow_ball", "Slow ball"),
    ]
    name = models.CharField(max_length=32)
    kind = models.CharField(max_length=20, choices=KIND_CHOICES)
    duration = models.IntegerField(default=5)

    class Meta:
        unique_together = ("name", "kind")

    def __str__(self):
        return f"{self.name} ({self.kind})"


class GameSession(models.Model):
    """A live game session coordinated through websockets.

    ``kind`` distinguishes the two games required by the subject: Pong
    (default) and the second game, Connect-4 (module 7).
    """
    PONG = "pong"
    CONNECT4 = "connect4"
    KIND_CHOICES = [(PONG, "Pong"), (CONNECT4, "Connect-4")]

    OPEN = "open"
    PLAYING = "playing"
    FINISHED = "finished"
    STATUS_CHOICES = [(OPEN, "Open"), (PLAYING, "Playing"), (FINISHED, "Finished")]

    name = models.CharField(max_length=64, unique=True)
    kind = models.CharField(max_length=10, choices=KIND_CHOICES, default=PONG)
    status = models.CharField(max_length=10, choices=STATUS_CHOICES, default=OPEN)
    player1 = models.ForeignKey(
        settings.AUTH_USER_MODEL, null=True, blank=True,
        related_name="sessions_as_p1", on_delete=models.SET_NULL,
    )
    player2 = models.ForeignKey(
        settings.AUTH_USER_MODEL, null=True, blank=True,
        related_name="sessions_as_p2", on_delete=models.SET_NULL,
    )
    is_ai = models.BooleanField(default=False)
    customization = models.ForeignKey(
        GameCustomization, null=True, blank=True,
        related_name="sessions", on_delete=models.SET_NULL,
    )
    score1 = models.IntegerField(default=0)
    score2 = models.IntegerField(default=0)
    winner = models.ForeignKey(
        settings.AUTH_USER_MODEL, null=True, blank=True,
        related_name="sessions_won", on_delete=models.SET_NULL,
    )
    created_at = models.DateTimeField(auto_now_add=True)

    def __str__(self):
        return self.name
