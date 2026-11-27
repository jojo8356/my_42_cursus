"""
WebSocket consumer for live Pong games.

Synchronizes an authoritative game state between remote players (Remote
players module): every client connected to the same session joins a Channels
group and receives the same broadcast state. Clients send only their inputs
(paddle move, step, power-up); the shared ``GameRoom`` applies them, resolves
the AI, and broadcasts the updated state to all participants.
"""
from channels.generic.websocket import AsyncJsonWebsocketConsumer

from .room import get_room, get_room_kind


class GameConsumer(AsyncJsonWebsocketConsumer):
    """Sync a Pong game between two connected clients (or client + AI)."""

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.session_id = None
        self.side = None
        self.room = None

    async def connect(self):
        self.session_id = self.scope["url_route"]["kwargs"].get("session_id")
        self.side = self.scope["url_route"]["kwargs"].get("side", "left")
        self.room = get_room(self.session_id, kind=get_room_kind(self.session_id) or "pong")
        await self.channel_layer.group_add(self.room.group_name, self.channel_name)
        await self.accept()
        # Send the current authoritative state to the newly connected client.
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
        if msg_type == "move":
            side = content.get("side", self.side)
            direction = int(content.get("direction", 0))
            self.room.move(side, direction)
            await self.room_broadcast()
        elif msg_type == "step":
            self.room.step()
            await self.room_broadcast()
        elif msg_type == "powerup":
            side = content.get("side", self.side)
            kind = content.get("kind", "speed")
            self.room.powerup(side, kind)
            await self.room_broadcast()

    async def room_broadcast(self):
        """Push the authoritative state to every client in the session."""
        await self.channel_layer.group_send(
            self.room.group_name,
            {"type": "state.broadcast", "state": self.room.state()},
        )

    async def state_broadcast(self, event):
        await self.send_json({"type": "state", "state": event["state"]})
