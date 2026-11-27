"""
WebSocket consumer for live Connect-4 games.

Synchronizes a Connect-4 board between two connected clients (or a client
and the server-side AI). Mirrors the Pong GameConsumer pattern: each
connected client drives the shared, authoritative Connect4 engine; the
server steps it and broadcasts the state.
"""
import json

from channels.generic.websocket import AsyncJsonWebsocketConsumer

from .connect4 import Connect4, PLAYER1, PLAYER2
from .connect4_ai import Connect4AI


class Connect4Consumer(AsyncJsonWebsocketConsumer):
    """Sync a Connect-4 game between two clients (or client + AI)."""

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.session_id = None
        self.side = None
        self.game = None
        self.ai = None
        self.ai_side = None

    async def connect(self):
        self.session_id = self.scope["url_route"]["kwargs"].get("session_id")
        self.side = self.scope["url_route"]["kwargs"].get("side", "left")
        self.game = Connect4()
        # If the connecting client is on the right side, the left side is
        # played by the AI (vs-AI mode), mirroring the Pong consumer.
        self.ai_side = "left" if self.side == "right" else "right"
        self.ai = Connect4AI(side=PLAYER1 if self.ai_side == "left" else PLAYER2)
        await self.accept()
        await self.send_json({"type": "state", "state": self.game.to_state()})

    async def disconnect(self, close_code):
        pass

    async def receive_json(self, content, **kwargs):
        msg_type = content.get("type")
        if msg_type == "drop":
            col = int(content.get("column", -1))
            if self.game.winner == 0 and self.game.drop(col):
                # If the AI side is to move and the game is not over, let
                # the AI reply so the game stays responsive in vs-AI mode.
                if (self.game.winner == 0 and self.ai is not None
                        and self.game.current == self.ai.side):
                    self.ai.act(self.game)
                await self.broadcast_state()
        elif msg_type == "step":
            # The authoritative loop also runs on each client; here we
            # re-broadcast the current state.
            await self.broadcast_state()

    async def broadcast_state(self):
        await self.send_json({"type": "state", "state": self.game.to_state()})
