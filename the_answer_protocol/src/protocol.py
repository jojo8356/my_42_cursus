"""RFC 42TAP — the wire protocol.

Pure, zero-I/O: this module knows how to turn a line of text into a Command and a
Command (or an event) into a line of text. It never touches a socket, which is
what makes the protocol exhaustively testable.

Wire rules (RFC §2.1, §4.1):
  * UTF-8, one message per line, terminated by LF (0x0A).
  * A reply is ``OK ...`` or ``ERR <code> <SYMBOL>``.
  * An event is ``EVT <SCOPE> <TYPE> <data...>`` or a chat line
    ``(GLOBAL|ROOM|GROUP) <user>: <message>``.
  * Verbs are case-insensitive; arguments keep their case.
  * JSON payloads are COMPACT (a newline would split the message) and an empty
    collection is ``[]``, never ``null``.

Inbound lines: a single trailing CR is stripped, so a peer that reads the CONNECT
grammar's ``CRLF`` literally (or a human testing through telnet) interoperates.
"""

from __future__ import annotations

import json
from dataclasses import dataclass, field
from typing import Any, Dict, Iterable, List, Optional, Sequence, Tuple

PROTOCOL_VERSION = 1
LINE_TERMINATOR = "\n"
MAX_LINE_BYTES = 1024
MAX_USERNAME = 20
MAX_GROUP_NAME = 25

GREETING = f"OK hello proto={PROTOCOL_VERSION}"


# ---------------------------------------------------------------------------
# Errors (RFC §7.2) — the codes are normative, the symbols disambiguate them
# ---------------------------------------------------------------------------
@dataclass(frozen=True)
class ErrorSpec:
    """One entry of the error table."""

    code: int
    symbol: str
    message: str

    @property
    def fatal(self) -> bool:
        """9xx errors are fatal to the session (§7.3)."""
        return self.code >= 900


ERRORS: Dict[str, ErrorSpec] = {
    # -- normative RFC 42TAP table --------------------------------------
    "NAME_IN_USE": ErrorSpec(201, "NAME_IN_USE", "username already taken"),
    "NO_EXIT": ErrorSpec(301, "NO_EXIT", "no exit in that direction"),
    "NOT_IN_GROUP": ErrorSpec(401, "NOT_IN_GROUP", "you are not in a group"),
    "ALREADY_IN_GROUP": ErrorSpec(402, "ALREADY_IN_GROUP", "already in a group"),
    "ITEM_NOT_FOUND": ErrorSpec(404, "ITEM_NOT_FOUND", "item not in the room"),
    "ITEM_NOT_IN_INVENTORY": ErrorSpec(404, "ITEM_NOT_IN_INVENTORY", "item not carried"),
    "NPC_NOT_FOUND": ErrorSpec(404, "NPC_NOT_FOUND", "npc not in the room"),
    "PLAYER_NOT_FOUND": ErrorSpec(404, "PLAYER_NOT_FOUND", "player not connected"),
    "GROUP_NOT_FOUND": ErrorSpec(404, "GROUP_NOT_FOUND", "group does not exist"),
    "PLAYER_NOT_IN_GROUP": ErrorSpec(404, "PLAYER_NOT_IN_GROUP", "player not in the group"),
    "NPC_NOT_HOSTILE": ErrorSpec(405, "NPC_NOT_HOSTILE", "npc is not hostile"),
    "NO_QUEST_AVAILABLE": ErrorSpec(406, "NO_QUEST_AVAILABLE", "no quest available"),
    "CONNECTION_FAILED": ErrorSpec(900, "CONNECTION_FAILED", "connection failed"),
    "SEND_FAILED": ErrorSpec(901, "SEND_FAILED", "send failed"),
    "FLOODING": ErrorSpec(902, "FLOODING", "rate limit exceeded"),
    # -- implementation choices (documented in the README) --------------
    "NOT_AUTHENTICATED": ErrorSpec(400, "NOT_AUTHENTICATED", "CONNECT first"),
    "BAD_REQUEST": ErrorSpec(400, "BAD_REQUEST", "malformed request"),
    "UNKNOWN_COMMAND": ErrorSpec(400, "UNKNOWN_COMMAND", "unknown command"),
    "MISSING_ARGUMENT": ErrorSpec(400, "MISSING_ARGUMENT", "missing argument"),
    "INVALID_USER_NAME": ErrorSpec(202, "INVALID_USER_NAME", "invalid username"),
    "GROUP_NAME_IN_USE": ErrorSpec(211, "GROUP_NAME_IN_USE", "group name already taken"),
    "INVALID_GROUP_NAME": ErrorSpec(212, "INVALID_GROUP_NAME", "invalid group name"),
    "NOT_GROUP_LEADER": ErrorSpec(403, "NOT_GROUP_LEADER", "only the leader may do that"),
    "CANNOT_INVITE_SELF": ErrorSpec(407, "CANNOT_INVITE_SELF", "you cannot invite yourself"),
    "CANNOT_KICK_SELF": ErrorSpec(407, "CANNOT_KICK_SELF", "you cannot kick yourself"),
    "NOT_INVITED": ErrorSpec(407, "NOT_INVITED", "no pending invitation"),
    "QUEST_NOT_COMPLETE": ErrorSpec(408, "QUEST_NOT_COMPLETE", "objective not met"),
    "QUEST_NOT_ACTIVE": ErrorSpec(404, "QUEST_NOT_ACTIVE", "you do not hold that quest"),
    "PLAYER_DEAD": ErrorSpec(409, "PLAYER_DEAD", "you are dead"),
    "ALREADY_CONNECTED": ErrorSpec(409, "ALREADY_CONNECTED", "already connected"),
    "CANNOT_SLEEP_HERE": ErrorSpec(410, "CANNOT_SLEEP_HERE", "you cannot sleep here"),
    "LINE_TOO_LONG": ErrorSpec(413, "LINE_TOO_LONG", "line exceeds the maximum length"),
    "NOT_IN_COMBAT": ErrorSpec(415, "NOT_IN_COMBAT", "you are not in combat"),
    "IN_COMBAT": ErrorSpec(416, "IN_COMBAT", "you are in combat, FLEE first"),
}


class ProtocolError(Exception):
    """Base class for protocol failures."""

    def __init__(self, symbol: str) -> None:
        spec = error_spec(symbol)
        super().__init__(f"ERR {spec.code} {spec.symbol}")
        self.symbol = spec.symbol
        self.code = spec.code
        self.message = spec.message


class Unauthorized(ProtocolError):
    """A command was sent before CONNECT."""


class UnknownCommand(ProtocolError):
    """The verb is not in the table."""


def error_spec(symbol: str) -> ErrorSpec:
    """Return the :class:`ErrorSpec` for ``symbol`` (raises when unknown)."""
    try:
        return ERRORS[symbol]
    except KeyError:
        raise KeyError(f"unknown error symbol: {symbol!r}") from None


def error_line(symbol: str) -> str:
    """Render an ``ERR`` reply line for ``symbol``."""
    spec = error_spec(symbol)
    return f"ERR {spec.code} {spec.symbol}"


# ---------------------------------------------------------------------------
# Commands
# ---------------------------------------------------------------------------
@dataclass
class Command:
    """A parsed client command: ``{verb, scope, sub, arg}``.

    ``verb`` is upper-case; ``scope``/``sub`` are upper-case when present; ``arg``
    keeps its original case (it may be a chat message or an item name).
    """

    verb: str
    scope: str = ""
    sub: str = ""
    arg: str = ""
    raw: str = ""

    @property
    def name(self) -> str:
        """``CHAT GLOBAL`` or just ``LOOK`` — the canonical two-word form."""
        if self.scope:
            return f"{self.verb} {self.scope}"
        return self.verb

    def to_dict(self) -> Dict[str, str]:
        return {"verb": self.verb, "scope": self.scope, "sub": self.sub, "arg": self.arg}


#: Verbs the parser accepts. Anything else is UNKNOWN_COMMAND.
KNOWN_VERBS = frozenset(
    {
        "CONNECT", "LOOK", "MOVE", "WHO", "CHAT", "TALK", "TAKE", "DROP",
        "INVENTORY", "EXAMINE", "ATTACK", "STATUS", "QUEST", "QUESTS", "QUIT",
        "GROUP", "DEFEND", "FLEE", "ABANDON_QUEST", "SLEEP", "HELP", "SAY",
    }
)

#: Verbs allowed before CONNECT.
PRE_AUTH_VERBS = frozenset({"CONNECT", "QUIT"})

#: Commands whose scope is the second token.
SCOPED_VERBS = frozenset({"CHAT", "GROUP"})

#: The second token is a sub-command rather than free-form chat text.
SCOPE_IS_SUBCOMMAND = frozenset()


def trim_line(line: str) -> str:
    """Drop the terminator and a single trailing CR (RFC quirk #1)."""
    if line.endswith("\n"):
        line = line[:-1]
    if line.endswith("\r"):
        line = line[:-1]
    return line


def parse_command(line: str) -> Command:
    """Parse one inbound line into a :class:`Command`.

    Raises :class:`UnknownCommand` for a verb outside :data:`KNOWN_VERBS`.
    An empty line yields a ``BAD_REQUEST`` command rather than an exception: the
    caller decides whether blank lines deserve a reply (they do not).
    """
    text = trim_line(line)
    raw = text
    stripped = text.strip()
    if not stripped:
        return Command(verb="", raw=raw)

    if len(stripped.encode("utf-8")) > MAX_LINE_BYTES:
        raise ProtocolError("LINE_TOO_LONG")

    pieces = stripped.split(None, 1)
    verb = pieces[0].upper()
    rest = pieces[1].strip() if len(pieces) == 2 else ""

    if verb not in KNOWN_VERBS:
        raise UnknownCommand("UNKNOWN_COMMAND")

    if verb in SCOPED_VERBS:
        pieces = rest.split(None, 1)
        scope = pieces[0].upper() if pieces else ""
        remainder = pieces[1].strip() if len(pieces) == 2 else ""
        return Command(verb=verb, scope=scope, arg=remainder, raw=raw)

    return Command(verb=verb, arg=rest, raw=raw)


# ---------------------------------------------------------------------------
# Replies
# ---------------------------------------------------------------------------
def ok(payload: str = "") -> str:
    """``OK ...`` — with a trailing space only when there is a payload."""
    payload = payload.strip()
    return f"OK {payload}" if payload else "OK"


def ok_json(payload: Any) -> str:
    """``OK <compact-json>`` — never pretty-printed, never ``null`` for empty."""
    if payload is None:
        payload = []
    return ok(compact_json(payload))


def err(symbol: str) -> str:
    """``ERR <code> <SYMBOL>``."""
    return error_line(symbol)


def compact_json(payload: Any) -> str:
    """Serialise compactly: no newline may ever reach the wire inside a message."""
    return json.dumps(payload, ensure_ascii=False, separators=(",", ":"))


def room_payload(room: Dict[str, Any], players: Sequence[str],
                 items: Sequence[str], npcs: Sequence[str]) -> Dict[str, Any]:
    """The LOOK payload: room fields at the top level (documented choice)."""
    payload: Dict[str, Any] = {
        "id": room["id"],
        "name": room["name"],
        "description": room["description"],
        "exits": dict(room.get("exits") or {}),
        "players": list(players),
        "items": list(items),
        "npcs": list(npcs),
    }
    if room.get("can_sleep"):
        payload["can_sleep"] = True
    return payload


def item_payload(item: Dict[str, Any]) -> Dict[str, Any]:
    """EXAMINE payload: id, name and the optional combat values."""
    return {
        "id": item["id"],
        "name": item["name"],
        "description": item.get("description", ""),
        "damage": item.get("damage"),
        "armor": item.get("armor"),
        "heal": item.get("heal"),
        "value": item.get("value", 0),
    }


def status_payload(hp: int, max_hp: int, status: str, gold: int = 0) -> Dict[str, Any]:
    """STATUS payload."""
    return {"hp": hp, "max_hp": max_hp, "gold": gold, "status": status}


# ---------------------------------------------------------------------------
# Events
# ---------------------------------------------------------------------------
ROOM_SCOPE = "ROOM"
GLOBAL_SCOPE = "GLOBAL"
GROUP_SCOPE = "GROUP"


def event(scope: str, kind: str, *data: Any) -> str:
    """``EVT <SCOPE> <TYPE> [data...]``."""
    parts = ["EVT", scope.upper(), kind.upper()] + [str(part) for part in data if part != ""]
    return " ".join(parts)


def room_presence(kind: str, player: str) -> str:
    """``EVT ROOM PRESENCE ENTER|LEAVE <player>``."""
    return event(ROOM_SCOPE, "PRESENCE", kind, player)


def room_item(kind: str, player: str, item_id: str) -> str:
    """``EVT ROOM ITEM TAKE|DROP <player> <item>``."""
    return event(ROOM_SCOPE, "ITEM", kind, player, item_id)


def room_combat(*data: Any) -> str:
    """``EVT ROOM COMBAT ...``."""
    return event(ROOM_SCOPE, "COMBAT", *data)


def group_event(kind: str, *data: Any) -> str:
    """``EVT GROUP <KIND> ...``."""
    return event(GROUP_SCOPE, kind, *data)


def stats_event(count: int) -> str:
    """``EVT STATS players=<count>``."""
    return f"EVT STATS players={count}"


def chat(scope: str, player: str, message: str) -> str:
    """``(GLOBAL|ROOM|GROUP) <user>: <message>``."""
    return f"({scope.upper()}) {player}: {message}"


# ---------------------------------------------------------------------------
# Parsing server output (used by the clients and the tests)
# ---------------------------------------------------------------------------
@dataclass
class Message:
    """One decoded server line."""

    kind: str  # "ok" | "err" | "evt" | "chat" | "raw"
    payload: str = ""
    code: Optional[int] = None
    symbol: str = ""
    scope: str = ""
    event_type: str = ""
    data: List[str] = field(default_factory=list)
    raw: str = ""

    @property
    def is_error(self) -> bool:
        return self.kind == "err"

    def to_dict(self) -> Dict[str, Any]:
        return {
            "kind": self.kind,
            "payload": self.payload,
            "code": self.code,
            "symbol": self.symbol,
            "scope": self.scope,
            "event_type": self.event_type,
            "data": list(self.data),
            "raw": self.raw,
        }


def parse_message(line: str) -> Message:
    """Decode one server line into a :class:`Message`."""
    text = trim_line(line)
    if text == "OK" or text.startswith("OK "):
        return Message(kind="ok", payload=text[2:].strip(), raw=text)
    if text.startswith("ERR "):
        parts = text.split(" ", 2)
        code: Optional[int] = None
        symbol = ""
        try:
            code = int(parts[1])
        except (IndexError, ValueError):
            code = None
        if len(parts) == 3:
            symbol = parts[2].strip()
        return Message(kind="err", code=code, symbol=symbol, raw=text)
    if text.startswith("EVT "):
        parts = text.split(" ")
        scope = parts[1] if len(parts) > 1 else ""
        event_type = parts[2] if len(parts) > 2 else ""
        return Message(kind="evt", scope=scope, event_type=event_type,
                       data=parts[3:], payload=" ".join(parts[3:]), raw=text)
    if text.startswith("("):
        closing = text.find(")")
        scope = text[1:closing] if closing != -1 else ""
        return Message(kind="chat", scope=scope, payload=text[closing + 1:].strip(), raw=text)
    return Message(kind="raw", payload=text, raw=text)


def split_json(payload: str) -> Any:
    """Parse the JSON payload of an OK reply, returning ``None`` when it is not JSON."""
    payload = payload.strip()
    if not payload:
        return None
    try:
        return json.loads(payload)
    except json.JSONDecodeError:
        return None


def frame(message: str) -> bytes:
    """Encode one message for the socket (UTF-8 + LF)."""
    return (message + LINE_TERMINATOR).encode("utf-8")


def parse_events(line: str) -> Tuple[str, str, List[str]]:
    """Convenience: ``(scope, type, data)`` for an event line."""
    message = parse_message(line)
    if message.kind != "evt":
        raise ProtocolError("BAD_REQUEST")
    return message.scope, message.event_type, message.data
