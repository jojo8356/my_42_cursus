import json

from channels.db import database_sync_to_async
from channels.generic.websocket import AsyncWebsocketConsumer

from .models import ChatMessage, ChatRoom

# Registre des utilisateurs connectes, par room (cle : nom du groupe).
CONNECTED_USERS = {}


def users_of(group_name):
    return sorted(CONNECTED_USERS.get(group_name, set()))


class ChatConsumer(AsyncWebsocketConsumer):
    room_id = None
    room = None
    room_group_name = None

    async def connect(self):
        self.room_id = int(self.scope['url_route']['kwargs']['room_id'])
        await self.get_room()
        self.room_group_name = f'chat_{self.room_id}'

        await self.channel_layer.group_add(self.room_group_name, self.channel_name)
        await self.accept()

        username = self.scope['user'].username
        CONNECTED_USERS.setdefault(self.room_group_name, set()).add(username)

        await self.channel_layer.group_send(
            self.room_group_name,
            {
                'type': 'chat.message',
                'message': f"{username} has joined the chat",
                'username': '',
                'join': True,
            }
        )
        await self.broadcast_user_list()
        await self.send_history()

    async def disconnect(self, close_code):
        if self.room_group_name is None:
            return
        username = self.scope['user'].username if self.scope['user'].is_authenticated else 'Anonymous'

        await self.channel_layer.group_discard(self.room_group_name, self.channel_name)
        CONNECTED_USERS.get(self.room_group_name, set()).discard(username)

        await self.channel_layer.group_send(
            self.room_group_name,
            {
                'type': 'chat.message',
                'message': f"{username} has left the chat",
                'username': '',
                'leave': True,
            }
        )
        await self.broadcast_user_list()

    async def receive(self, text_data):
        data = json.loads(text_data)
        message = (data.get('message') or '').strip()

        if message:
            username = self.scope['user'].username
            await self.save_message(username, message)
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

    async def chat_user_list(self, event):
        await self.send(text_data=json.dumps({
            'type': 'user_list',
            'users': event['users'],
        }))

    async def broadcast_user_list(self):
        """Diffuse la liste des utilisateurs connectes a toute la room."""
        await self.channel_layer.group_send(
            self.room_group_name,
            {
                'type': 'chat.user_list',
                'users': users_of(self.room_group_name),
            }
        )

    async def send_history(self):
        """Envoie les 3 derniers messages de la room (ex02)."""
        messages = await self.get_history()
        await self.send(text_data=json.dumps({
            'type': 'history',
            'messages': messages,
        }))

    @database_sync_to_async
    def get_room(self):
        self.room = ChatRoom.objects.get(pk=self.room_id)

    @database_sync_to_async
    def save_message(self, username, message):
        ChatMessage.objects.create(
            room=self.room,
            username=username,
            content=message,
        )

    @database_sync_to_async
    def get_history(self):
        messages = ChatMessage.objects.filter(room=self.room).order_by('-created')[:3]
        return [
            {
                'username': m.username,
                'content': m.content,
                'created': m.created.isoformat(),
            }
            for m in reversed(messages)
        ]
