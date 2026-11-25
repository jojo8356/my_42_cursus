"""
WebSocket consumer for live Pong games.

Synchronizes game state between remote players (Remote players module) and
runs the server-side AI when a session is vs-AI. Also handles the matchmaking
"next fight" announcements via the tournament channel.
"""
import json

from channels.generic.websocket import AsyncJsonWebsocketConsumer

from .engine import PongGame
from .ai import PongAI


class GameConsumer(AsyncJsonWebsocketConsumer):
    """Sync a Pong game between two connected clients (or client + AI)."""

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.session_id = None
        self.side = None
        self.game = None
        self.ai = None
        self.ai_elapsed = 0.0

    async def connect(self):
        self.session_id = self.scope["url_route"]["kwargs"].get("session_id")
        self.side = self.scope["url_route"]["kwargs"].get("side", "left")
        self.game = PongGame(right_is_ai=True)
        if self.side == "left":
            self.ai = PongAI(side="right")
        else:
            self.ai = PongAI(side="left")
        self.game.start()
        await self.accept()
        await self.send_json({"type": "state", "state": self.game.to_state()})

    async def disconnect(self, close_code):
        pass

    async def receive_json(self, content, **kwargs):
        msg_type = content.get("type")
        if msg_type == "move":
            # a player moved their paddle
            side = content.get("side", self.side)
            direction = int(content.get("direction", 0))
            self.game.move_paddle(side, direction)
            await self.broadcast_state()
        elif msg_type == "step":
            # advance the physics one tick (the authoritative loop runs on
            # each connected client; here we step server-side and broadcast)
            self.game.step()
            # AI acts once per second
            self.ai_elapsed += 1.0
            if self.ai_elapsed >= PongAI.REFRESH_RATE:
                self.ai.act(self.game, self.ai_elapsed)
                self.ai_elapsed = 0.0
            await self.broadcast_state()
        elif msg_type == "powerup":
            side = content.get("side", self.side)
            kind = content.get("kind", "speed")
            self.game.apply_powerup(side, kind)
            await self.broadcast_state()

    async def broadcast_state(self):
        await self.send_json({"type": "state", "state": self.game.to_state()})
