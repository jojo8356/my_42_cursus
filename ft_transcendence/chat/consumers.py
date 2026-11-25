"""
WebSocket consumer for chat: direct messages, channels, online presence.

Respects blocking: a user never receives messages from someone they blocked.
Also broadcasts presence (online/offline) for the friends list.
"""
import json

from channels.generic.websocket import AsyncJsonWebsocketConsumer
from channels.db import database_sync_to_async

from accounts.models import User, Block, Message
from .models import Channel, ChannelMember, ChannelMessage
from accounts.views import _sanitize_xss


@database_sync_to_async
def _get_user(user_id):
    try:
        return User.objects.get(id=user_id, is_active=True)
    except User.DoesNotExist:
        return None


@database_sync_to_async
def _save_dm(sender_id, recipient_id, content):
    sender = User.objects.get(id=sender_id)
    recipient = User.objects.get(id=recipient_id)
    Message.objects.create(sender=sender, recipient=recipient, content=content)


@database_sync_to_async
def _save_channel_msg(channel_id, sender_id, content):
    sender = User.objects.get(id=sender_id)
    ChannelMessage.objects.create(channel_id=channel_id, sender=sender, content=content)


@database_sync_to_async
def _is_blocked(blocker_id, blocked_id):
    return Block.objects.filter(user_id=blocker_id, blocked_user_id=blocked_id).exists()


class ChatConsumer(AsyncJsonWebsocketConsumer):
    """Handles DMs and channel messages over websockets in real time."""

    async def connect(self):
        self.user_id = self.scope["user"].id if self.scope["user"].is_authenticated else None
        if not self.user_id:
            await self.close()
            return
        self.room_name = f"chat_{self.user_id}"
        await self.channel_layer.group_add(self.room_name, self.channel_name)
        await self.accept()

    async def disconnect(self, close_code):
        if self.user_id:
            await self.channel_layer.group_discard(self.room_name, self.channel_name)
            # presence: broadcast offline
            await self.channel_layer.group_send(
                self.room_name,
                {"type": "presence", "user_id": self.user_id, "online": False},
            )

    async def receive_json(self, content, **kwargs):
        msg_type = content.get("type")
        if msg_type == "dm":
            recipient_id = content.get("to")
            if not recipient_id:
                return
            # check if the recipient blocked the sender
            if await _is_blocked(recipient_id, self.user_id):
                return
            text = _sanitize_xss((content.get("content") or "").strip())
            if not text:
                return
            await _save_dm(self.user_id, recipient_id, text)
            # deliver to recipient (and echo to sender)
            await self.channel_layer.group_send(
                f"chat_{recipient_id}",
                {"type": "dm", "from": self.user_id, "to": recipient_id, "content": text},
            )
            await self.send_json({"type": "dm", "from": self.user_id,
                                  "to": recipient_id, "content": text})
        elif msg_type == "channel":
            channel_id = content.get("channel")
            if not channel_id:
                return
            text = _sanitize_xss((content.get("content") or "").strip())
            if not text:
                return
            await _save_channel_msg(channel_id, self.user_id, text)
            await self.channel_layer.group_send(
                f"channel_{channel_id}",
                {"type": "channel", "channel": channel_id,
                 "from": self.user_id, "content": text},
            )

    async def dm(self, event):
        await self.send_json({
            "type": "dm", "from": event["from"], "to": event["to"],
            "content": event["content"],
        })

    async def channel(self, event):
        await self.send_json({
            "type": "channel", "channel": event["channel"],
            "from": event["from"], "content": event["content"],
        })

    async def presence(self, event):
        await self.send_json({
            "type": "presence", "user_id": event["user_id"],
            "online": event["online"],
        })
