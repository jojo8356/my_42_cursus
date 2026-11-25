"""
Tournament models: Tournament, Participant, Match.
"""
from django.conf import settings
from django.db import models
from django.utils import timezone


class Tournament(models.Model):
    OPEN = "open"
    RUNNING = "running"
    FINISHED = "finished"
    STATUS_CHOICES = [(OPEN, "Open"), (RUNNING, "Running"), (FINISHED, "Finished")]

    name = models.CharField(max_length=128)
    status = models.CharField(max_length=10, choices=STATUS_CHOICES, default=OPEN)
    winner = models.ForeignKey(
        settings.AUTH_USER_MODEL, null=True, blank=True,
        related_name="won_tournaments", on_delete=models.SET_NULL,
    )
    created_at = models.DateTimeField(default=timezone.now)
    # bracket snapshot: list of rounds, each a list of [p1, p2] player ids
    bracket = models.JSONField(default=list, blank=True)

    def __str__(self):
        return self.name

    def start(self):
        self.status = self.RUNNING
        self.save()

    def finish(self, winner):
        self.status = self.FINISHED
        self.winner = winner
        self.save()


class Participant(models.Model):
    tournament = models.ForeignKey(Tournament, related_name="participants", on_delete=models.CASCADE)
    user = models.ForeignKey(settings.AUTH_USER_MODEL, related_name="tournament_entries", on_delete=models.CASCADE)
    alias = models.CharField(max_length=32)  # alias resets each tournament
    seed = models.IntegerField(default=0)
    score = models.IntegerField(default=0)
    eliminated = models.BooleanField(default=False)

    class Meta:
        unique_together = ("tournament", "user")

    def __str__(self):
        return f"{self.alias} ({self.user}) in {self.tournament}"


class Match(models.Model):
    tournament = models.ForeignKey(Tournament, related_name="match_records", on_delete=models.CASCADE)
    player1 = models.ForeignKey(
        settings.AUTH_USER_MODEL, null=True, blank=True,
        related_name="tournament_matches_p1", on_delete=models.SET_NULL,
    )
    player2 = models.ForeignKey(
        settings.AUTH_USER_MODEL, null=True, blank=True,
        related_name="tournament_matches_p2", on_delete=models.SET_NULL,
    )
    winner = models.ForeignKey(
        settings.AUTH_USER_MODEL, null=True, blank=True,
        related_name="tournament_matches_won", on_delete=models.SET_NULL,
    )
    score1 = models.IntegerField(default=0)
    score2 = models.IntegerField(default=0)
    round_index = models.IntegerField(default=0)
    played_at = models.DateTimeField(default=timezone.now)

    class Meta:
        ordering = ["round_index", "played_at"]

    def __str__(self):
        p1 = self.player1.username if self.player1 else "?"
        p2 = self.player2.username if self.player2 else "?"
        return f"{p1} vs {p2} (r{self.round_index})"
