from django.contrib.auth.models import AbstractUser
from django.db import models


class CustomUser(AbstractUser):
    """Utilisateur du site, avec une reputation calculée a partir de ses Tips."""

    reputation = models.IntegerField(default=0)

    def __str__(self):
        return self.username

    def recompute_reputation(self):
        """Recalcule la reputation : +5 par upvote, -2 par downvote recus."""
        total = 0
        for tip in self.tips.all():
            total += 5 * tip.upvotes.count()
            total -= 2 * tip.downvotes.count()
        self.reputation = total
        self.save(update_fields=['reputation'])
        self.sync_permissions()
        return total

    def sync_permissions(self):
        """Accorde ou retire les permissions selon la reputation courante."""
        from django.contrib.auth.models import Permission

        wanted = {
            'can_downvote': self.reputation >= 15,
            'delete_tip': self.reputation >= 30,
        }
        for codename, granted in wanted.items():
            try:
                perm = Permission.objects.get(codename=codename)
            except Permission.DoesNotExist:
                continue
            if granted and not self.user_permissions.filter(pk=perm.pk).exists():
                self.user_permissions.add(perm)
            elif not granted and self.user_permissions.filter(pk=perm.pk).exists():
                self.user_permissions.remove(perm)


class Tip(models.Model):
    content = models.TextField()
    author = models.ForeignKey(CustomUser, on_delete=models.CASCADE, related_name='tips')
    date = models.DateTimeField(auto_now_add=True)
    upvotes = models.ManyToManyField(CustomUser, related_name='upvoted_tips', blank=True)
    downvotes = models.ManyToManyField(CustomUser, related_name='downvoted_tips', blank=True)

    class Meta:
        permissions = [
            ('can_downvote', 'Can downvote tips'),
        ]

    @property
    def upvote_count(self):
        return self.upvotes.count()

    @property
    def downvote_count(self):
        return self.downvotes.count()

    def __str__(self):
        return self.content[:50]
