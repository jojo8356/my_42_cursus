"""The authoritative game state and its command dispatcher.

Everything that mutates the shared world lives here, behind one lock: two players
can never take the same item at the same time. The dispatcher is pure apart from
the outbound queue it writes to, which makes every rule testable without a socket.

Notification model: a handler returns the reply for the acting player and queues
EVENT lines for everyone else. The acting player is always excluded from
ENTER/LEAVE events (they already got the reply) and always included in chat, per
the RFC's own example.
"""

from __future__ import annotations

import re
import threading
import time
from collections import deque
from dataclasses import dataclass, field
from typing import Any, Callable, Deque, Dict, List, Optional, Sequence, Tuple

import protocol
from protocol import Command, ProtocolError
from world import Item, Npc, Quest, Room, World

USERNAME_RE = re.compile(r"^[A-Za-z0-9_-]{1,20}$")
GROUP_NAME_RE = re.compile(r"^[A-Za-z0-9_-]{1,25}$")

BASE_DAMAGE = 10
RESPAWN_HP = 50
FLOOD_WARN = 10
FLOOD_KICK = 20


# ---------------------------------------------------------------------------
# Sessions
# ---------------------------------------------------------------------------
@dataclass
class Player:
    """One connected player and everything they carry."""

    name: str
    room_id: str
    hp: int = 100
    max_hp: int = 100
    gold: int = 0
    inventory: List[str] = field(default_factory=list)
    quests: Dict[str, Dict[str, Any]] = field(default_factory=dict)
    group: str = ""
    in_combat_with: str = ""
    dead: bool = False
    previous_room: str = ""

    @property
    def status(self) -> str:
        """The public RFC status is binary: a player is alive or dead."""
        return "dead" if self.dead or self.hp <= 0 else "alive"

    def has(self, item_id: str) -> bool:
        return item_id in self.inventory

    def count(self, item_id: str) -> int:
        return sum(1 for carried in self.inventory if carried == item_id)

    def weapon_bonus(self, items: Dict[str, Item]) -> int:
        """The best weapon bonus carried (0 when unarmed)."""
        bonuses = [items[i].damage or 0 for i in self.inventory if i in items]
        return max(bonuses) if bonuses else 0

    def armor_bonus(self, items: Dict[str, Item]) -> int:
        """The best armor carried (0 when unarmoured)."""
        bonuses = [items[i].armor or 0 for i in self.inventory if i in items]
        return max(bonuses) if bonuses else 0

    def to_dict(self) -> Dict[str, Any]:
        return {
            "name": self.name,
            "room": self.room_id,
            "hp": self.hp,
            "max_hp": self.max_hp,
            "gold": self.gold,
            "inventory": list(self.inventory),
            "group": self.group,
            "status": self.status,
        }


@dataclass
class Group:
    """A player group."""

    id: str
    leader: str
    players: List[str] = field(default_factory=list)
    invited: List[str] = field(default_factory=list)

    def to_dict(self) -> Dict[str, Any]:
        return {
            "id": self.id,
            "leader": self.leader,
            "players": list(self.players),
            "invited": list(self.invited),
        }


class Session:
    """The transport-facing half of a connection: a queue plus a rate limiter.

    It is deliberately tiny and socket-free so the dispatcher can be driven in
    tests without a network.
    """

    def __init__(self, send: Optional[Callable[[str], None]] = None,
                 clock: Callable[[], float] = time.monotonic) -> None:
        self._send = send
        self.sent: List[str] = []
        self._clock = clock
        self._commands: Deque[float] = deque()
        self.closed = False
        self.kicked = False

    def send(self, message: str) -> None:
        """Queue one line for this client."""
        if self.closed:
            return
        self.sent.append(message)
        if self._send is not None:
            try:
                self._send(message)
            except Exception:  # noqa: BLE001 - a dead socket must not break the game
                self.closed = True

    # -- flood control ---------------------------------------------------
    def record_command(self) -> str:
        """Count one command; return ``""``, ``"warn"`` or ``"kick"``."""
        now = self._clock()
        while self._commands and now - self._commands[0] >= 1.0:
            self._commands.popleft()
        self._commands.append(now)
        count = len(self._commands)
        if count > FLOOD_KICK:
            self.kicked = True
            return "kick"
        if count > FLOOD_WARN:
            return "warn"
        return ""


# ---------------------------------------------------------------------------
# Game state
# ---------------------------------------------------------------------------
class GameState:
    """The shared world plus every connected player."""

    def __init__(self, world: World, clock: Callable[[], float] = time.monotonic,
                 logger: Optional[Any] = None) -> None:
        self.world = world
        self.clock = clock
        self.logger = logger
        # room_id -> item ids currently on the ground
        self.ground: Dict[str, List[str]] = {
            room.id: list(room.items) for room in world.rooms.values()
        }
        # room_id -> live NPC instances (id -> hp)
        self.npcs: Dict[str, Dict[str, int]] = {
            room.id: {npc_id: world.npcs[npc_id].hp for npc_id in room.npcs}
            for room in world.rooms.values()
        }
        self.players: Dict[str, Player] = {}
        self.sessions: Dict[str, Session] = {}
        self.groups: Dict[str, Group] = {}
        self._quest_seq = 0
        self._dialogue_indexes: Dict[str, int] = {npc_id: 0 for npc_id in world.npcs}
        self._lock = threading.RLock()
        # (ready_at, room_id, npc_id) — a defeated enemy comes back later.
        self._respawns: List[Tuple[float, str, str]] = []
        self.respawn_delay_s = 30.0
        # Items taken from the ground come back after this delay.
        self._item_respawns: List[Tuple[float, str, str]] = []
        self.item_respawn_delay_s = 30.0
        self._player_respawns: List[Tuple[float, str]] = []
        self.player_respawn_delay_s = 3.0

    # -- timers ----------------------------------------------------------
    def _schedule_respawn(self, room_id: str, npc_id: str) -> None:
        self._respawns.append((self.clock() + self.respawn_delay_s, room_id, npc_id))

    def _schedule_item_respawn(self, room_id: str, item_id: str) -> None:
        self._item_respawns.append(
            (self.clock() + self.item_respawn_delay_s, room_id, item_id)
        )

    def tick(self) -> List[str]:
        """Fire due NPC/item/player timers and broadcast their consequences."""
        with self._lock:
            return self._tick_locked()

    def _tick_locked(self) -> List[str]:
        now = self.clock()
        emitted: List[str] = []

        due = [entry for entry in self._respawns if entry[0] <= now]
        self._respawns = [entry for entry in self._respawns if entry[0] > now]
        for _, room_id, npc_id in due:
            npc = self.world.npcs[npc_id]
            self.npcs[room_id][npc_id] = npc.max_hp
            line = protocol.event("ROOM", "NPC", "RESPAWN", npc_id)
            self.broadcast_room(room_id, line)
            emitted.append(line)

        due_items = [entry for entry in self._item_respawns if entry[0] <= now]
        self._item_respawns = [entry for entry in self._item_respawns if entry[0] > now]
        for _, room_id, item_id in due_items:
            # Each pickup schedules one replacement, including duplicate item ids.
            self.ground.setdefault(room_id, []).append(item_id)
            line = protocol.event("ROOM", "ITEM", "RESPAWN", item_id)
            self.broadcast_room(room_id, line)
            emitted.append(line)

        due_players = [entry for entry in self._player_respawns if entry[0] <= now]
        self._player_respawns = [entry for entry in self._player_respawns if entry[0] > now]
        for _, username in due_players:
            player = self.players.get(username)
            if player is not None and player.dead:
                self.respawn(player)
                line = protocol.room_combat(f"{username} respawned")
                self.broadcast_room(player.room_id, line)
                emitted.append(line)
        return emitted

    # -- helpers ---------------------------------------------------------
    def _log(self, level: str, msg: str, **fields: Any) -> None:
        if self.logger is not None:
            self.logger.log(level, msg, **fields)

    def player(self, name: str) -> Player:
        try:
            return self.players[name]
        except KeyError:
            raise ProtocolError("PLAYER_NOT_FOUND") from None

    def room_of(self, player: Player) -> Room:
        return self.world.rooms[player.room_id]

    def players_in(self, room_id: str) -> List[str]:
        return [name for name, player in self.players.items()
                if player.room_id == room_id and not player.dead]

    def npcs_in(self, room_id: str) -> List[str]:
        """Live (undefeated) NPC ids in a room, in world declaration order."""
        room = self.world.rooms.get(room_id)
        if room is None:
            return []
        alive = self.npcs.get(room_id, {})
        return [npc_id for npc_id in room.npcs if alive.get(npc_id, 0) > 0]

    def items_in(self, room_id: str) -> List[str]:
        return list(self.ground.get(room_id, []))

    # -- broadcasting ----------------------------------------------------
    def broadcast_room(self, room_id: str, line: str, exclude: Sequence[str] = ()) -> None:
        """Send ``line`` to everyone in ``room_id`` except ``exclude``."""
        for name in self.players_in(room_id):
            if name in exclude:
                continue
            session = self.sessions.get(name)
            if session is not None:
                session.send(line)

    def broadcast_all(self, line: str, exclude: Sequence[str] = ()) -> None:
        for name, session in self.sessions.items():
            if name in exclude:
                continue
            session.send(line)

    def broadcast_group(self, group_id: str, line: str, exclude: Sequence[str] = ()) -> None:
        group = self.groups.get(group_id)
        if group is None:
            return
        for name in group.players:
            if name in exclude:
                continue
            session = self.sessions.get(name)
            if session is not None:
                session.send(line)

    def stats_event(self) -> str:
        return protocol.stats_event(len(self.players))

    # -- matching --------------------------------------------------------
    def match_item_in_room(self, room_id: str, token: str) -> str:
        candidates = self.items_in(room_id)
        names = {item_id: self.world.items[item_id].name for item_id in candidates}
        return _match(token, candidates, "ITEM_NOT_FOUND", names=names)

    def match_item_in_inventory(self, player: Player, token: str) -> str:
        names = {item_id: self.world.items[item_id].name
                 for item_id in player.inventory if item_id in self.world.items}
        return _match(token, player.inventory, "ITEM_NOT_IN_INVENTORY", names=names)

    def match_npc(self, room_id: str, token: str) -> str:
        """Match a present NPC by id, id suffix, display name or substring."""
        candidates = self.npcs_in(room_id)
        names = {npc_id: self.world.npcs[npc_id].name for npc_id in candidates}
        return _match(token, candidates, "NPC_NOT_FOUND", names=names)

    # =====================================================================
    # Connection lifecycle
    # =====================================================================
    def connect(self, session: Session, username: str) -> Tuple[str, List[Tuple[str, str]]]:
        """Register a player atomically."""
        with self._lock:
            return self._connect_locked(session, username)

    def _connect_locked(self, session: Session, username: str) -> Tuple[str, List[Tuple[str, str]]]:
        """Implementation of :meth:`connect`; the lock is already held."""
        if not username:
            raise ProtocolError("BAD_REQUEST")
        if not USERNAME_RE.match(username):
            raise ProtocolError("INVALID_USER_NAME")
        if username in self.players:
            raise ProtocolError("NAME_IN_USE")

        player = Player(name=username, room_id=self.world.start_room)
        self.players[username] = player
        self.sessions[username] = session
        self._log("INFO", "player connected", type="session", player=username)

        # The room is told, and everybody gets the new online count.
        notice = protocol.room_presence("ENTER", username)
        self.broadcast_room(player.room_id, notice, exclude=[username])
        self.broadcast_all(self.stats_event())
        lines: List[Tuple[str, str]] = [(username, self.stats_event())]
        return protocol.ok("Connected"), lines

    def disconnect(self, username: str, reason: str = "quit") -> List[Tuple[str, str]]:
        """Remove a player atomically and broadcast their departure."""
        with self._lock:
            return self._disconnect_locked(username, reason)

    def _disconnect_locked(self, username: str, reason: str) -> List[Tuple[str, str]]:
        player = self.players.pop(username, None)
        session = self.sessions.pop(username, None)
        if player is None:
            return []
        if session is not None:
            session.closed = True
        # Leave the group first, so no ghost member remains.
        group_events: List[Tuple[str, str]] = []
        if player.group:
            group_events = self._leave_group(player, silent_ok=True)
        for group in self.groups.values():
            if username in group.invited:
                group.invited.remove(username)
        # Drop carried items on the ground so nothing is destroyed.
        for item_id in player.inventory:
            self.ground.setdefault(player.room_id, []).append(item_id)
        player.inventory.clear()

        leave = protocol.room_presence("LEAVE", username)
        self.broadcast_room(player.room_id, leave, exclude=[username])
        self.broadcast_all(self.stats_event(), exclude=[username])
        self._log("INFO", "player disconnected", type="session", player=username,
                  reason=reason)
        return group_events + [(f"room:{player.room_id}", leave)]

    # =====================================================================
    # Dispatcher
    # =====================================================================
    def dispatch(self, session: Session, line: str) -> Optional[str]:
        """Handle one line atomically and return the sender's reply."""
        with self._lock:
            return self._dispatch_locked(session, line)

    def _dispatch_locked(self, session: Session, line: str) -> Optional[str]:
        """Internal dispatcher; caller holds ``_lock``."""
        if len(line.encode("utf-8")) > protocol.MAX_LINE_BYTES:
            return protocol.err("LINE_TOO_LONG")
        try:
            command = protocol.parse_command(line)
        except ProtocolError as exc:
            return protocol.err(exc.symbol)
        if not command.verb:
            return None

        verdict = session.record_command()
        if verdict == "kick":
            self._log("WARN", "client kicked for flooding", type="abuse",
                      player=self.name_of(session))
            session.send(protocol.err("FLOODING"))
            return None
        if verdict == "warn":
            self._log("WARN", "command flood warning", type="abuse",
                      player=self.name_of(session))

        name = self.name_of(session)
        authenticated = name in self.players

        if not authenticated and command.verb not in protocol.PRE_AUTH_VERBS:
            return protocol.err("NOT_AUTHENTICATED")

        self._log("INFO", "command", type="command", player=name or "-",
                  ip=getattr(session, "ip", None), command=command.verb, args=command.arg)

        handler = self._HANDLERS.get(command.name) or self._HANDLERS.get(command.verb)
        if handler is None:
            if command.verb in protocol.SCOPED_VERBS and not command.scope:
                return protocol.err("MISSING_ARGUMENT")
            return protocol.err("UNKNOWN_COMMAND")
        try:
            reply = handler(self, session, command)
        except ProtocolError as exc:
            self._log("WARN", "error response", type="response", player=name or "-",
                      code=exc.code, response=str(exc))
            return protocol.err(exc.symbol)
        except Exception as exc:  # noqa: BLE001 - never kill the server on one command
            self._log("ERROR", "handler crashed", type="error", player=name or "-",
                      error=f"{type(exc).__name__}: {exc}")
            return protocol.err("BAD_REQUEST")
        self._log("INFO", "response", type="response", player=name or "-",
                  ip=getattr(session, "ip", None), response=reply)
        return reply

    def name_of(self, session: Session) -> str:
        for name, candidate in self.sessions.items():
            if candidate is session:
                return name
        return ""

    # =====================================================================
    # LOOK / STATUS / WHO / HELP
    # =====================================================================
    def handle_look(self, session: Session, command: Command) -> str:
        name = self.name_of(session)
        player = self.player(name)
        room = self.room_of(player)
        payload = protocol.room_payload(
            room.to_dict(),
            players=self.players_in(room.id),
            items=self.items_in(room.id),
            npcs=self.npcs_in(room.id),
        )
        return protocol.ok_json(payload)

    def handle_status(self, session: Session, command: Command) -> str:
        player = self.player(self.name_of(session))
        return protocol.ok_json(
            protocol.status_payload(player.hp, player.max_hp, player.status, player.gold)
        )

    def handle_who(self, session: Session, command: Command) -> str:
        return protocol.ok(f"players={len(self.players)}")

    def handle_help(self, session: Session, command: Command) -> str:
        verbs = sorted(protocol.KNOWN_VERBS)
        return protocol.ok("commands=" + ",".join(verbs))

    # =====================================================================
    # CONNECT / QUIT
    # =====================================================================
    def handle_connect(self, session: Session, command: Command) -> str:
        if self.name_of(session) in self.players:
            raise ProtocolError("ALREADY_CONNECTED")
        username = command.arg.strip()
        if not username:
            raise ProtocolError("MISSING_ARGUMENT")
        reply, _ = self.connect(session, username)
        return reply

    def handle_quit(self, session: Session, command: Command) -> str:
        name = self.name_of(session)
        if name:
            self.disconnect(name, reason="quit")
        session.closed = True
        return protocol.ok("bye")

    # =====================================================================
    # Movement
    # =====================================================================
    def handle_move(self, session: Session, command: Command) -> str:
        player = self.player(self.name_of(session))
        if player.dead:
            raise ProtocolError("PLAYER_DEAD")
        if player.in_combat_with:
            raise ProtocolError("IN_COMBAT")
        direction = command.arg.strip().lower()
        if not direction:
            raise ProtocolError("MISSING_ARGUMENT")
        room = self.room_of(player)
        if direction not in room.exits:
            raise ProtocolError("NO_EXIT")

        destination = room.exits[direction]
        old_room = player.room_id
        self.broadcast_room(old_room, protocol.room_presence("LEAVE", player.name),
                            exclude=[player.name])
        player.previous_room = old_room
        player.room_id = destination
        player.in_combat_with = ""
        self.broadcast_room(destination, protocol.room_presence("ENTER", player.name),
                            exclude=[player.name])
        self._log("INFO", "player moved", type="world", player=player.name,
                  room=destination)
        return protocol.ok(f"room={destination}")

    # =====================================================================
    # Chat
    # =====================================================================
    def handle_chat(self, session: Session, command: Command) -> str:
        player = self.player(self.name_of(session))
        scope = command.scope
        message = command.arg
        if not scope:
            raise ProtocolError("MISSING_ARGUMENT")
        if scope not in (protocol.GLOBAL_SCOPE, protocol.ROOM_SCOPE, protocol.GROUP_SCOPE):
            raise ProtocolError("BAD_REQUEST")
        if not message:
            raise ProtocolError("MISSING_ARGUMENT")
        line = protocol.chat(scope, player.name, message)
        if scope == protocol.GLOBAL_SCOPE:
            self.broadcast_all(line)
        elif scope == protocol.ROOM_SCOPE:
            self.broadcast_room(player.room_id, line)
        else:
            if not player.group:
                raise ProtocolError("NOT_IN_GROUP")
            self.broadcast_group(player.group, line)
        self._log("INFO", "chat", type="chat", player=player.name, scope=scope,
                  length=len(message))
        return protocol.ok()

    def handle_say(self, session: Session, command: Command) -> str:
        """``SAY <message>`` — shorthand for room chat."""
        command.scope = protocol.ROOM_SCOPE
        return self.handle_chat(session, command)

    # =====================================================================
    # Items
    # =====================================================================
    def handle_take(self, session: Session, command: Command) -> str:
        player = self.player(self.name_of(session))
        token = command.arg.strip()
        if not token:
            raise ProtocolError("MISSING_ARGUMENT")
        item_id = self.match_item_in_room(player.room_id, token)
        item = self.world.items[item_id]
        if not item.obtainable:
            raise ProtocolError("ITEM_NOT_FOUND")
        self.ground[player.room_id].remove(item_id)
        player.inventory.append(item_id)
        self._schedule_item_respawn(player.room_id, item_id)
        self.broadcast_room(
            player.room_id, protocol.room_item("TAKE", player.name, item_id),
            exclude=[player.name],
        )
        self._log("INFO", "item taken", type="world", player=player.name, item=item_id,
                  room=player.room_id)
        self._advance_quests(player, "fetch", item_id)
        self._advance_quests(player, "deliver", item_id)
        return protocol.ok(f"taken={item_id}")

    def handle_drop(self, session: Session, command: Command) -> str:
        player = self.player(self.name_of(session))
        token = command.arg.strip()
        if not token:
            raise ProtocolError("MISSING_ARGUMENT")
        item_id = self.match_item_in_inventory(player, token)
        player.inventory.remove(item_id)
        self.ground.setdefault(player.room_id, []).append(item_id)
        self.broadcast_room(
            player.room_id, protocol.room_item("DROP", player.name, item_id),
            exclude=[player.name],
        )
        return protocol.ok(f"dropped={item_id}")

    def handle_inventory(self, session: Session, command: Command) -> str:
        player = self.player(self.name_of(session))
        return protocol.ok_json(list(player.inventory))

    def handle_examine(self, session: Session, command: Command) -> str:
        player = self.player(self.name_of(session))
        token = command.arg.strip()
        if not token:
            raise ProtocolError("MISSING_ARGUMENT")
        try:
            item_id = self.match_item_in_room(player.room_id, token)
        except ProtocolError:
            item_id = self.match_item_in_inventory(player, token)
        return protocol.ok_json(protocol.item_payload(self.world.items[item_id].to_dict()))

    # =====================================================================
    # NPCs and quests
    # =====================================================================
    def handle_talk(self, session: Session, command: Command) -> str:
        player = self.player(self.name_of(session))
        token = command.arg.strip()
        if not token:
            raise ProtocolError("MISSING_ARGUMENT")
        npc_id = self.match_npc(player.room_id, token)
        npc = self.world.npcs[npc_id]
        if npc.dialogue:
            index = self._dialogue_indexes[npc_id] % len(npc.dialogue)
            line = npc.dialogue[index]
            self._dialogue_indexes[npc_id] = (index + 1) % len(npc.dialogue)
        else:
            line = f"{npc.name} says nothing."
        self._log("INFO", "npc talk", type="npc", player=player.name, npc=npc_id)
        return protocol.ok_json({"npc": npc.name, "dialogue": line})

    def handle_quest(self, session: Session, command: Command) -> str:
        player = self.player(self.name_of(session))
        token = command.arg.strip()
        if not token:
            raise ProtocolError("MISSING_ARGUMENT")
        npc_id = self.match_npc(player.room_id, token)

        # Deliveries are turned in to their named recipient, not necessarily the
        # quest giver. The recipient can be any present NPC.
        deliver = next((quest for quest in self.world.quests.values()
                        if quest.type == "deliver" and quest.recipient == npc_id
                        and player.quests.get(quest.id, {}).get("status") == "active"), None)
        if deliver is not None:
            if self._objective_met(player, deliver):
                return self._complete_quest(player, deliver)
            raise ProtocolError("QUEST_NOT_COMPLETE")

        npc = self.world.npcs[npc_id]
        if npc.role != "quest_giver":
            raise ProtocolError("NO_QUEST_AVAILABLE")

        quest = self._quest_for_giver(player, npc_id)
        if quest is None:
            raise ProtocolError("NO_QUEST_AVAILABLE")
        entry = player.quests.get(quest.id)

        # -- turn in a held quest ---------------------------------------
        if entry is not None:
            if entry["status"] == "completed":
                raise ProtocolError("NO_QUEST_AVAILABLE")
            if quest.type == "deliver" and quest.recipient != npc_id:
                raise ProtocolError("QUEST_NOT_COMPLETE")
            if self._objective_met(player, quest):
                return self._complete_quest(player, quest)
            raise ProtocolError("QUEST_NOT_COMPLETE")

        # -- accept ------------------------------------------------------
        if quest.requires:
            prerequisite = player.quests.get(quest.requires)
            if prerequisite is None or prerequisite["status"] != "completed":
                raise ProtocolError("NO_QUEST_AVAILABLE")
        if quest.type == "kill":
            progress = 0
        else:
            progress = min(player.count(quest.target), quest.target_count)
        player.quests[quest.id] = {"status": "active", "progress": progress}
        self._log("INFO", "quest accepted", type="quest", player=player.name,
                  quest=quest.id)
        payload = quest.to_dict()
        payload["status"] = "active"
        if quest.type in ("fetch", "deliver"):
            payload["progress"] = f"{progress}/{quest.target_count}"
        return protocol.ok_json(payload)

    def _quest_for_giver(self, player: Player, giver_id: str) -> Optional[Quest]:
        """Return the first uncompleted quest offered by ``giver_id``."""
        for quest in self.world.quests_for_giver(giver_id):
            entry = player.quests.get(quest.id)
            if entry is None or entry.get("status") == "active":
                if quest.requires:
                    prerequisite = player.quests.get(quest.requires)
                    if prerequisite is None or prerequisite.get("status") != "completed":
                        continue
                return quest
        return None

    def _objective_met(self, player: Player, quest: Quest) -> bool:
        entry = player.quests.get(quest.id) or {}
        if quest.type in ("fetch", "deliver"):
            return player.count(quest.target) >= quest.target_count
        return int(entry.get("progress", 0)) >= quest.target_count

    def _advance_quests(self, player: Player, quest_type: str, target: str) -> None:
        """Advance every active quest of ``quest_type`` whose target matches."""
        for quest_id, entry in player.quests.items():
            if entry["status"] != "active":
                continue
            quest = self.world.quests[quest_id]
            if quest.type != quest_type or quest.target != target:
                continue
            if quest_type in ("fetch", "deliver"):
                entry["progress"] = min(player.count(quest.target), quest.target_count)
            else:
                entry["progress"] = min(int(entry["progress"]) + 1, quest.target_count)
            # Progress only: the player must come back and turn the quest in with
            # `QUEST <giver>` (subject, Quest Flow step 3).

    def _complete_quest(self, player: Player, quest: Quest) -> str:
        entry = player.quests.setdefault(quest.id, {"status": "active", "progress": 0})
        entry["status"] = "completed"
        entry["progress"] = quest.target_count

        # Consume the objective.
        if quest.type in ("fetch", "deliver"):
            removed = 0
            kept: List[str] = []
            for item_id in player.inventory:
                if item_id == quest.target and removed < quest.target_count:
                    removed += 1
                    continue
                kept.append(item_id)
            player.inventory = kept
        # Grant the reward.
        for _ in range(max(1, quest.reward_count) if quest.reward else 0):
            if quest.reward and quest.reward in self.world.items:
                player.inventory.append(quest.reward)
        self._log("INFO", "quest completed", type="quest", player=player.name,
                  quest=quest.id, reward=quest.reward)
        return protocol.ok_json({
            "quest_id": quest.id,
            "status": "completed",
            "reward": quest.reward,
            "reward_count": quest.reward_count,
        })

    def handle_quests(self, session: Session, command: Command) -> str:
        player = self.player(self.name_of(session))
        payload: List[Dict[str, Any]] = []
        for quest_id, entry in player.quests.items():
            quest = self.world.quests.get(quest_id)
            item: Dict[str, Any] = {"quest_id": quest_id, "status": entry["status"]}
            if entry["status"] == "active" and quest is not None:
                progress = self._progress(player, quest)
                item["progress"] = f"{progress}/{quest.target_count}"
            payload.append(item)
        return protocol.ok_json(payload)

    def _progress(self, player: Player, quest: Quest) -> int:
        if quest.type in ("fetch", "deliver"):
            return min(player.count(quest.target), quest.target_count)
        return int((player.quests.get(quest.id) or {}).get("progress", 0))

    def handle_abandon_quest(self, session: Session, command: Command) -> str:
        player = self.player(self.name_of(session))
        token = command.arg.strip()
        if not token:
            raise ProtocolError("MISSING_ARGUMENT")
        quest_id = _match_quest(token, player.quests)
        entry = player.quests.get(quest_id)
        if entry is None or entry["status"] != "active":
            raise ProtocolError("QUEST_NOT_ACTIVE")
        player.quests.pop(quest_id, None)
        return protocol.ok_json({"quest_id": quest_id, "status": "abandoned"})

    # =====================================================================
    # Combat
    # =====================================================================
    def handle_attack(self, session: Session, command: Command) -> str:
        player = self.player(self.name_of(session))
        if player.dead:
            raise ProtocolError("PLAYER_DEAD")
        token = command.arg.strip()
        if not token:
            raise ProtocolError("MISSING_ARGUMENT")
        npc_id = self.match_npc(player.room_id, token)
        npc = self.world.npcs[npc_id]
        if not npc.hostile:
            raise ProtocolError("NPC_NOT_HOSTILE")

        damage = BASE_DAMAGE + player.weapon_bonus(self.world.items)
        current_hp = self.npcs[player.room_id].get(npc_id, npc.hp)
        target_hp = max(0, current_hp - damage)
        self.npcs[player.room_id][npc_id] = target_hp
        player.in_combat_with = npc_id

        payload: Dict[str, Any] = {
            "attacker_hp": player.hp,
            "target_hp": target_hp,
            "damage": damage,
            "npc_damage": 0,
            "status": "combat",
        }

        if target_hp == 0:
            payload["status"] = "victory"
            if npc.gold_drop:
                player.gold += npc.gold_drop
                payload["gold_earned"] = npc.gold_drop
            self.broadcast_room(
                player.room_id,
                protocol.room_combat(f"{npc.name} defeated by {player.name}",
                                     f"npc={npc_id}"),
                exclude=[player.name],
            )
            player.in_combat_with = ""
            # The enemy stays defeated until its respawn timer fires.
            self.npcs[player.room_id][npc_id] = 0
            self._advance_quests(player, "kill", npc_id)
            self._schedule_respawn(player.room_id, npc_id)
            return protocol.ok_json(payload)

        # Enemy counter-attacks in the same round.
        incoming = max(0, npc.damage - player.armor_bonus(self.world.items))
        player.hp = max(0, player.hp - incoming)
        payload["attacker_hp"] = player.hp
        payload["npc_damage"] = incoming
        self.broadcast_room(
            player.room_id,
            protocol.room_combat(f"{player.name} attacks {npc.name} for {damage} damage",
                                 f"npc={npc_id}", f"hp={target_hp}"),
            exclude=[player.name],
        )
        if player.hp == 0:
            payload.update(self._kill_player(player, npc))
        return protocol.ok_json(payload)

    def _kill_player(self, player: Player, npc: Npc) -> Dict[str, Any]:
        room_id = player.room_id
        self.broadcast_room(
            room_id, protocol.room_combat(f"{player.name} was killed by {npc.name}"),
            exclude=[player.name],
        )
        self.broadcast_room(room_id, protocol.room_presence("LEAVE", player.name),
                            exclude=[player.name])
        player.dead = True
        player.hp = 0
        player.in_combat_with = ""
        self._player_respawns.append((self.clock() + self.player_respawn_delay_s,
                                      player.name))
        payload = {
            "status": "death",
            "respawn_room": self.world.start_room,
            "respawn_hp": RESPAWN_HP,
        }
        return payload

    def handle_defend(self, session: Session, command: Command) -> str:
        player = self.player(self.name_of(session))
        if not player.in_combat_with:
            raise ProtocolError("NOT_IN_COMBAT")
        player.hp = min(player.max_hp, player.hp + 5)
        return protocol.ok_json({
            "attacker_hp": player.hp,
            "target_hp": self.npcs[player.room_id].get(player.in_combat_with, 0),
            "status": "defend",
        })

    def handle_flee(self, session: Session, command: Command) -> str:
        player = self.player(self.name_of(session))
        if not player.in_combat_with:
            raise ProtocolError("NOT_IN_COMBAT")
        player.in_combat_with = ""
        destination = player.previous_room or player.room_id
        if destination != player.room_id:
            self.broadcast_room(player.room_id,
                                protocol.room_presence("LEAVE", player.name),
                                exclude=[player.name])
            player.room_id = destination
            self.broadcast_room(destination, protocol.room_presence("ENTER", player.name),
                                exclude=[player.name])
        return protocol.ok_json({"fled": True, "room": player.room_id, "status": "fled"})

    def respawn(self, player: Player) -> List[str]:
        """Bring a dead player back at the start room with half HP."""
        with self._lock:
            return self._respawn_locked(player)

    def _respawn_locked(self, player: Player) -> List[str]:
        old_room = player.room_id
        player.dead = False
        player.hp = RESPAWN_HP
        player.room_id = self.world.start_room
        player.in_combat_with = ""
        lines = [
            protocol.room_presence("ENTER", player.name),
        ]
        self.broadcast_room(old_room, protocol.room_presence("LEAVE", player.name),
                            exclude=[player.name])
        self.broadcast_room(player.room_id, lines[0], exclude=[player.name])
        return lines

    def handle_sleep(self, session: Session, command: Command) -> str:
        player = self.player(self.name_of(session))
        if player.dead:
            raise ProtocolError("PLAYER_DEAD")
        room = self.room_of(player)
        if not (room.can_sleep or room.id == self.world.sleep_room):
            raise ProtocolError("CANNOT_SLEEP_HERE")
        player.hp = player.max_hp
        self.broadcast_room(player.room_id, protocol.event("ROOM", "SLEEP", player.name),
                            exclude=[player.name])
        return protocol.ok(f"hp={player.hp}/{player.max_hp} You feel really good now !")

    # =====================================================================
    # Groups
    # =====================================================================
    def handle_group_create(self, session: Session, command: Command) -> str:
        player = self.player(self.name_of(session))
        if player.group:
            raise ProtocolError("ALREADY_IN_GROUP")
        name = command.arg.strip() or player.name
        if not GROUP_NAME_RE.match(name):
            raise ProtocolError("INVALID_GROUP_NAME")
        group_id = name
        if group_id in self.groups:
            raise ProtocolError("GROUP_NAME_IN_USE")
        self.groups[group_id] = Group(id=group_id, leader=player.name,
                                      players=[player.name])
        player.group = group_id
        self._log("INFO", "group created", type="group", player=player.name,
                  group=group_id)
        return protocol.ok(f"group={group_id}")

    def handle_group_invite(self, session: Session, command: Command) -> str:
        player = self.player(self.name_of(session))
        if not player.group:
            raise ProtocolError("NOT_IN_GROUP")
        group = self.groups[player.group]
        if group.leader != player.name:
            raise ProtocolError("NOT_GROUP_LEADER")
        target = command.arg.strip()
        if not target:
            raise ProtocolError("MISSING_ARGUMENT")
        if target == player.name:
            raise ProtocolError("CANNOT_INVITE_SELF")
        invitee = self.players.get(target)
        if invitee is None:
            raise ProtocolError("PLAYER_NOT_FOUND")
        if invitee.group:
            raise ProtocolError("ALREADY_IN_GROUP")
        if target in group.invited:
            raise ProtocolError("ALREADY_IN_GROUP")
        group.invited.append(target)
        session_target = self.sessions.get(target)
        if session_target is not None:
            session_target.send(protocol.group_event("INVITE", player.name))
        return protocol.ok()

    def handle_group_join(self, session: Session, command: Command) -> str:
        player = self.player(self.name_of(session))
        if player.group:
            raise ProtocolError("ALREADY_IN_GROUP")
        wanted = command.arg.strip()
        if not wanted:
            raise ProtocolError("MISSING_ARGUMENT")
        group = self.groups.get(wanted)
        if group is None:
            # Accept the inviter's name too (RFC quirk #4).
            for candidate in self.groups.values():
                if wanted in candidate.invited or wanted == candidate.leader:
                    group = candidate
                    break
        if group is None:
            raise ProtocolError("GROUP_NOT_FOUND")
        if player.name not in group.invited:
            raise ProtocolError("NOT_INVITED")
        group.invited.remove(player.name)
        group.players.append(player.name)
        player.group = group.id
        self.broadcast_group(group.id, protocol.group_event("JOIN", player.name))
        return protocol.ok(f"group={group.id}")

    def handle_group_leave(self, session: Session, command: Command) -> str:
        player = self.player(self.name_of(session))
        if not player.group:
            raise ProtocolError("NOT_IN_GROUP")
        self._leave_group(player)
        return protocol.ok()

    def _leave_group(self, player: Player, silent_ok: bool = False) -> List[Tuple[str, str]]:
        group = self.groups.get(player.group)
        player.group = ""
        if group is None:
            return []
        events: List[Tuple[str, str]] = []
        if player.name in group.players:
            group.players.remove(player.name)
        if not group.players:
            self.groups.pop(group.id, None)
            return []
        leave_line = protocol.group_event("LEAVE", player.name)
        self.broadcast_group(group.id, leave_line)
        events.append((f"group:{group.id}", leave_line))
        if group.leader == player.name:
            group.leader = group.players[0]
            line = protocol.group_event("LEADER", group.leader)
            self.broadcast_group(group.id, line)
            events.append((f"group:{group.id}", line))
        return events

    def handle_group_kick(self, session: Session, command: Command) -> str:
        player = self.player(self.name_of(session))
        if not player.group:
            raise ProtocolError("NOT_IN_GROUP")
        group = self.groups[player.group]
        if group.leader != player.name:
            raise ProtocolError("NOT_GROUP_LEADER")
        target = command.arg.strip()
        if not target:
            raise ProtocolError("MISSING_ARGUMENT")
        if target == player.name:
            raise ProtocolError("CANNOT_KICK_SELF")
        if target not in group.players:
            raise ProtocolError("PLAYER_NOT_IN_GROUP")
        victim = self.players.get(target)
        group.players.remove(target)
        if victim is not None:
            victim.group = ""
            victim_session = self.sessions.get(target)
            if victim_session is not None:
                victim_session.send(protocol.group_event("KICK", player.name))
        self.broadcast_group(group.id, protocol.group_event("LEAVE", target))
        return protocol.ok()

    def handle_group_disband(self, session: Session, command: Command) -> str:
        player = self.player(self.name_of(session))
        if not player.group:
            raise ProtocolError("NOT_IN_GROUP")
        group = self.groups[player.group]
        if group.leader != player.name:
            raise ProtocolError("NOT_GROUP_LEADER")
        for member in list(group.players):
            member_player = self.players.get(member)
            if member_player is not None:
                member_player.group = ""
            member_session = self.sessions.get(member)
            if member_session is not None:
                member_session.send(protocol.group_event("DISBAND"))
        self.groups.pop(group.id, None)
        return protocol.ok()

    def handle_group_info(self, session: Session, command: Command) -> str:
        player = self.player(self.name_of(session))
        if not player.group:
            raise ProtocolError("NOT_IN_GROUP")
        return protocol.ok_json(self.groups[player.group].to_dict())

    # -- handler table ---------------------------------------------------
    _HANDLERS: Dict[str, Callable[["GameState", Session, Command], str]] = {}


def _match(token: str, candidates: Sequence[str], missing_symbol: str,
           names: Optional[Dict[str, str]] = None) -> str:
    """Match ``token`` against ids, id suffixes, display names, then substrings.

    Players type what they read: ``TALK chani`` must resolve ``npc.spice_merchant``
    because that is the name in the room description.
    """
    token_lower = token.lower().strip()
    names = names or {}

    for candidate in candidates:  # exact id
        if candidate.lower() == token_lower:
            return candidate
    for candidate in candidates:  # exact display name
        if names.get(candidate, "").lower() == token_lower:
            return candidate
    for candidate in candidates:  # id suffix (loc./npc./item. prefix dropped)
        if candidate.lower().endswith("." + token_lower):
            return candidate
    for candidate in candidates:  # display name starts with the token
        if names.get(candidate, "").lower().startswith(token_lower):
            return candidate
    matches = [candidate for candidate in candidates if token_lower in candidate.lower()]
    matches += [candidate for candidate in candidates
                if token_lower in names.get(candidate, "").lower()
                and candidate not in matches]
    if matches:
        return matches[0]
    raise ProtocolError(missing_symbol)


def _match_quest(token: str, held: Dict[str, Dict[str, Any]]) -> str:
    token_lower = token.lower()
    for quest_id in held:
        if quest_id.lower() == token_lower:
            return quest_id
    for quest_id in held:
        if token_lower in quest_id.lower():
            return quest_id
    raise ProtocolError("QUEST_NOT_ACTIVE")


GameState._HANDLERS = {
    "CONNECT": GameState.handle_connect,
    "QUIT": GameState.handle_quit,
    "LOOK": GameState.handle_look,
    "MOVE": GameState.handle_move,
    "WHO": GameState.handle_who,
    "CHAT": GameState.handle_chat,
    "SAY": GameState.handle_say,
    "TALK": GameState.handle_talk,
    "TAKE": GameState.handle_take,
    "DROP": GameState.handle_drop,
    "INVENTORY": GameState.handle_inventory,
    "EXAMINE": GameState.handle_examine,
    "ATTACK": GameState.handle_attack,
    "STATUS": GameState.handle_status,
    "QUEST": GameState.handle_quest,
    "QUESTS": GameState.handle_quests,
    "ABANDON_QUEST": GameState.handle_abandon_quest,
    "DEFEND": GameState.handle_defend,
    "FLEE": GameState.handle_flee,
    "SLEEP": GameState.handle_sleep,
    "HELP": GameState.handle_help,
    "GROUP CREATE": GameState.handle_group_create,
    "GROUP INVITE": GameState.handle_group_invite,
    "GROUP JOIN": GameState.handle_group_join,
    "GROUP LEAVE": GameState.handle_group_leave,
    "GROUP KICK": GameState.handle_group_kick,
    "GROUP DISBAND": GameState.handle_group_disband,
    "GROUP INFO": GameState.handle_group_info,
}
