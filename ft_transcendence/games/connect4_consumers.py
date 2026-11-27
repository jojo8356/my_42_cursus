"""
WebSocket consumer for live Connect-4 games.

Synchronizes an authoritative Connect-4 board between two connected clients
(or a client and the server-side AI). Every client joined to the same session
receives the same broadcast state from the shared ``GameRoom``.
"""
from channels.generic.websocket import AsyncJsonWebsocketConsumer

from .room import get_room, get_room_kind


class Connect4Consumer(AsyncJsonWebsocketConsumer):
    """Sync a Connect-4 game between two clients (or client + AI)."""

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.session_id = None
        self.side = None
        self.room = None

    async def connect(self):
        self.session_id = self.scope["url_route"]["kwargs"].get("session_id")
        self.side = self.scope["url_route"]["kwargs"].get("side", "left")
        kind = get_room_kind(self.session_id) or "connect4"
        self.room = get_room(self.session_id, kind=kind)
        await self.channel_layer.group_add(self.room.group_name, self.channel_name)
        await self.accept()
        await self.send_json({"type": "state", "state": self.room.state()})

    async def disconnect(self, close_code):
        if self.room is not None:
            await self.channel_layer.group_discard(
                self.room.group_name, self.channel_name
            )

    async def receive_json(self, content, **kwargs):
        msg_type = content.get("type")
        if self.room is None:
            return
        if msg_type == "drop":
            col = int(content.get("column", -1))
            if self.room.drop(col):
                await self.room_broadcast()
        elif msg_type == "step":
            await self.room_broadcast()

    async def room_broadcast(self):
        await self.channel_layer.group_send(
            self.room.group_name,
            {"type": "state.broadcast", "state": self.room.state()},
        )

    async def state_broadcast(self, event):
        await self.send_json({"type": "state", "state": event["state"]})
