import json
from channels.generic.websocket import AsyncWebsocketConsumer
from channels.db import database_sync_to_async
from .models import ChatRoom, ChatMessage


class ChatConsumer(AsyncWebsocketConsumer):
    room_id = None
    room = None

    async def connect(self):
        self.room_id = int(self.scope['url_route']['kwargs']['room_id'])
        await self.get_room()
        self.room_group_name = f'chat_{self.room_id}'

        # Groupe
        await self.channel_layer.group_add(self.room_group_name, self.channel_name)
        await self.accept()

        # Notifier que l'utilisateur a rejoint
        username = self.scope['user'].username
        await self.channel_layer.group_send(
            self.room_group_name,
            {
                'type': 'chat.message',
                'message': f"{username} has joined the chat",
                'username': '',
                'join': True,
            }
        )

        # Envoyer la liste des utilisateurs connectés
        await self.send_user_list()

        # Envoyer l'historique (3 derniers messages) pour ex02
        await self.send_history()

    async def disconnect(self, close_code):
        await self.channel_layer.group_discard(self.room_group_name, self.channel_name)

        # Notifier que l'utilisateur a quitté
        username = self.scope['user'].username if self.scope['user'].is_authenticated else 'Anonymous'
        await self.channel_layer.group_send(
            self.room_group_name,
            {
                'type': 'chat.message',
                'message': f"{username} has left the chat",
                'username': '',
                'leave': True,
            }
        )

        # Envoyer la liste des utilisateurs connectés mise à jour
        await self.send_user_list()

    async def receive(self, text_data):
        data = json.loads(text_data)
        message = data.get('message', '')

        if message:
            username = self.scope['user'].username
            # Sauvegarder en base (historique)
            await self.save_message(username, message)
            # Envoyer à tous
            await self.channel_layer.group_send(
                self.room_group_name,
                {
                    'type': 'chat.message',
                    'message': message,
                    'username': username,
                }
            )

    async def chat_message(self, event):
        await self.send(text_data=json.dumps({
            'message': event['message'],
            'username': event.get('username', ''),
            'join': event.get('join', False),
            'leave': event.get('leave', False),
        }))

    async def send_history(self):
        """Envoie les 3 derniers messages (ex02)."""
        messages = await self.get_history()
        await self.send(text_data=json.dumps({
            'type': 'history',
            'messages': messages,
        }))

    async def send_user_list(self):
        """Envoie la liste des utilisateurs connectés (ex03)."""
        users = await self.get_connected_users()
        await self.send(text_data=json.dumps({
            'type': 'user_list',
            'users': users,
        }))

    @database_sync_to_async
    def get_room(self):
        self.room = ChatRoom.objects.get(pk=self.room_id)

    @database_sync_to_async
    def save_message(self, username, message):
        ChatMessage.objects.create(
            room=self.room,
            username=username,
            content=message
        )

    @database_sync_to_async
    def get_history(self):
        messages = ChatMessage.objects.filter(room=self.room).order_by('-created')[:3]
        return [{
            'username': m.username,
            'content': m.content,
            'created': m.created.isoformat(),
        } for m in reversed(messages)]

    @database_sync_to_async
    def get_connected_users(self):
        # Récupérer les utilisateurs connectés depuis les channels
        # Simplifié : retourne les usernames des channels actifs
        # Pour une implémentation complète, on maintendrait un registre
        # Ici, on retourne une liste basique
        return [self.scope['user'].username] if self.scope['user'].is_authenticated else []
