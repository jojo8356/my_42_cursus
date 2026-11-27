"""
Authoritative game rooms for live, synchronized multi-player games.

The subject's "Remote players" module requires two distant players, each on
their own computer, to play the *same* game. To make that reliable, the
server owns the authoritative game state (a single ``PongGame`` or
``Connect4`` per session) and broadcasts it to every connected client over a
Channels group. Clients only send their inputs (paddle move, drop, power-up);
the server applies them, resolves the AI, and pushes the updated state to all
participants.

A room is stored in a process-wide cache keyed by session id. The container
runs a single daphne process (see docker-entrypoint.sh), so the in-memory
cache is shared by every consumer of a session. (A Redis-backed room could be
swapped in for multi-process deployments.)
"""
from .engine import PongGame
from .ai import PongAI
from .connect4 import Connect4, PLAYER1, PLAYER2
from .connect4_ai import Connect4AI


class GameRoom:
    """A single authoritative game session shared by all connected clients."""

    def __init__(self, session_id, kind="pong", is_ai=False,
                 enable_powerups=True, enable_custom_map=False):
        self.session_id = session_id
        self.kind = kind
        self.is_ai = is_ai
        self.group_name = f"game_{session_id}"
        self._ai_elapsed = 0.0
        self._ai = None
        if kind == "connect4":
            self.game = Connect4()
            # The AI plays the side opposite to the first human client.
            self._ai = Connect4AI(side=PLAYER2)
        else:
            self.game = PongGame(
                enable_powerups=enable_powerups,
                enable_custom_map=enable_custom_map,
            )
            self.game.start()
            if is_ai:
                self.game.right_is_ai = True
                self._ai = PongAI(side="right")

    # ------------------------------------------------------------------ state
    def state(self):
        return self.game.to_state()

    # ------------------------------------------------------------------ pong inputs
    def move(self, side, direction):
        if self.kind != "pong":
            return False
        self.game.move_paddle(side, int(direction))
        return True

    def powerup(self, side, kind):
        if self.kind != "pong":
            return False
        return self.game.apply_powerup(side, kind)

    def step(self):
        """Advance the Pong physics one tick and run the AI at 1 Hz."""
        if self.kind != "pong":
            return
        self.game.step()
        self._ai_elapsed += 1.0
        if self._ai is not None and self._ai_elapsed >= PongAI.REFRESH_RATE:
            self._ai.act(self.game, self._ai_elapsed)
            self._ai_elapsed = 0.0

    # ------------------------------------------------------------------ connect4 inputs
    def drop(self, col):
        if self.kind != "connect4":
            return False
        col = int(col)
        if self.game.winner != 0 and self.game.winner != 0:
            return False
        if not self.game.drop(col):
            return False
        # If the AI side is to move and the game is still alive, let it reply.
        if (self.game.winner == 0 and self._ai is not None
                and self.game.current == self._ai.side):
            self._ai.act(self.game)
        return True


# --------------------------------------------------------------------------- room cache
_ROOMS = {}


def get_room(session_id, kind="pong", is_ai=False,
             enable_powerups=True, enable_custom_map=False):
    """Return the shared room for a session, creating it on first use."""
    key = str(session_id)
    room = _ROOMS.get(key)
    if room is None:
        room = GameRoom(
            session_id, kind=kind, is_ai=is_ai,
            enable_powerups=enable_powerups,
            enable_custom_map=enable_custom_map,
        )
        _ROOMS[key] = room
    return room


def get_room_kind(session_id):
    room = _ROOMS.get(str(session_id))
    return room.kind if room else None


def drop_room(session_id):
    _ROOMS.pop(str(session_id), None)
