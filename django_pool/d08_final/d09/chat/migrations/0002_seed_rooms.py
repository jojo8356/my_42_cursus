from django.db import migrations


ROOMS = ["Général", "Random", "Tech"]


def seed(apps, schema_editor):
    ChatRoom = apps.get_model('chat', 'ChatRoom')
    if ChatRoom.objects.exists():
        return
    for name in ROOMS:
        ChatRoom.objects.create(name=name)


def unseed(apps, schema_editor):
    ChatRoom = apps.get_model('chat', 'ChatRoom')
    ChatRoom.objects.filter(name__in=ROOMS).delete()


class Migration(migrations.Migration):

    dependencies = [
        ('chat', '0001_initial'),
    ]

    operations = [
        migrations.RunPython(seed, unseed),
    ]
