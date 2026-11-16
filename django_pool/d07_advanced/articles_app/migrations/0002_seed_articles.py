from django.db import migrations
from django.contrib.auth.hashers import make_password

LOREM = ("Lorem ipsum dolor sit amet, consectetur adipiscing elit. Sed do eiusmod "
         "tempor incididunt ut labore et dolore magna aliqua.")

ARTICLES = [
    ("Le marteau universel", "Pourquoi un seul outil ne suffit jamais.", "alice"),
    ("Fabriquer une etagere", "Quelques conseils de menuiserie simple.", "alice"),
    ("Les fabriques de fabriques", "Une metaphore sur l'abstraction.", "bruno"),
    ("Acheter le bon outil", "Neuf ou occasions, ce qu'il faut regarder.", "bruno"),
    ("Ranger ses condiments", "L'essentiel de cet exercice, en resume.", "carole"),
]


def seed(apps, schema_editor):
    User = apps.get_model('auth', 'User')
    Article = apps.get_model('articles_app', 'Article')

    if Article.objects.exists():
        return

    users = {}
    for name in ('alice', 'bruno', 'carole'):
        users[name], _ = User.objects.get_or_create(
            username=name,
            defaults={'password': make_password('password123')},
        )

    for title, synopsis, author in ARTICLES:
        Article.objects.create(
            title=title,
            author=users[author],
            synopsis=synopsis,
            content=LOREM,
        )


def unseed(apps, schema_editor):
    Article = apps.get_model('articles_app', 'Article')
    Article.objects.filter(title__in=[a[0] for a in ARTICLES]).delete()


class Migration(migrations.Migration):

    dependencies = [
        ('articles_app', '0001_initial'),
    ]

    operations = [
        migrations.RunPython(seed, unseed),
    ]
