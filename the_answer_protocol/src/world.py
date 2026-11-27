"""The world: rooms, items, NPCs and quests, plus startup validation.

The world is loaded from a JSON data file (the subject allows any format; JSON is
the one the wire already speaks). Ids are canonicalised ONCE at load — ``loc.``,
``item.``, ``npc.``, ``quest.`` — so the data file stays readable while the wire
stays unambiguous.

Validation is strict and selective: an exit to an unknown room, a room holding an
unknown item or NPC, or a quest referencing an unknown giver/target/reward makes
the server refuse to start. A broken world is a bug, not a warning.
"""

from __future__ import annotations

import json
import os
from dataclasses import dataclass, field
from typing import Any, Dict, Iterable, List, Optional, Sequence

ROOM_PREFIX = "loc."
ITEM_PREFIX = "item."
NPC_PREFIX = "npc."
QUEST_PREFIX = "quest."

DIRECTIONS = ("north", "south", "east", "west", "up", "down")
NPC_ROLES = ("dialogue", "quest_giver", "enemy")
QUEST_TYPES = ("fetch", "kill", "deliver")


class WorldError(ValueError):
    """The world data is invalid; the server must not start."""


def canonical(identifier: str, prefix: str) -> str:
    """Add a type prefix and normalise the readable ``room.`` alias to ``loc.``."""
    if not identifier:
        return identifier
    if "." in identifier:
        head, tail = identifier.split(".", 1)
        if head in ("loc", "room", "item", "npc", "quest"):
            return ("loc." if head == "room" else head + ".") + tail
    return prefix + identifier


@dataclass
class Room:
    """One place in the world."""

    id: str
    name: str
    description: str
    exits: Dict[str, str] = field(default_factory=dict)
    items: List[str] = field(default_factory=list)
    npcs: List[str] = field(default_factory=list)
    can_sleep: bool = False

    def to_dict(self) -> Dict[str, Any]:
        payload: Dict[str, Any] = {
            "id": self.id,
            "name": self.name,
            "description": self.description,
            "exits": dict(self.exits),
            "items": list(self.items),
            "npcs": list(self.npcs),
        }
        if self.can_sleep:
            payload["can_sleep"] = True
        return payload


@dataclass
class Item:
    """One item definition."""

    id: str
    name: str
    description: str = ""
    damage: Optional[int] = None
    armor: Optional[int] = None
    heal: Optional[int] = None
    value: int = 0
    obtainable: bool = True

    def to_dict(self) -> Dict[str, Any]:
        return {
            "id": self.id,
            "name": self.name,
            "description": self.description,
            "damage": self.damage,
            "armor": self.armor,
            "heal": self.heal,
            "value": self.value,
            "obtainable": self.obtainable,
        }


@dataclass
class Npc:
    """One non-player character."""

    id: str
    name: str
    role: str = "dialogue"
    description: str = ""
    dialogue: List[str] = field(default_factory=list)
    hp: int = 20
    max_hp: int = 20
    damage: int = 5
    gold_drop: int = 0

    @property
    def hostile(self) -> bool:
        return self.role == "enemy"

    def to_dict(self) -> Dict[str, Any]:
        return {
            "id": self.id,
            "name": self.name,
            "role": self.role,
            "hostile": self.hostile,
            "hp": self.hp,
            "max_hp": self.max_hp,
            "damage": self.damage,
        }


@dataclass
class Quest:
    """One quest definition.

    ``type`` is ``fetch`` (bring ``target_count`` of ``target``), ``kill``
    (defeat ``target_count`` of ``target``) or ``deliver`` (carry ``target`` to
    ``recipient``).
    """

    id: str
    giver: str
    description: str = ""
    type: str = "fetch"
    target: str = ""
    target_count: int = 1
    reward: str = ""
    reward_count: int = 1
    requires: str = ""
    recipient: str = ""
    name: str = ""

    def to_dict(self) -> Dict[str, Any]:
        payload: Dict[str, Any] = {
            "id": self.id,
            "description": self.description,
            "giver": self.giver,
            "type": self.type,
            "target": self.target,
            "target_count": self.target_count,
            "reward": self.reward,
            "reward_count": self.reward_count,
        }
        if self.requires:
            payload["requires"] = self.requires
        if self.recipient:
            payload["recipient"] = self.recipient
        return payload


class World:
    """The immutable-at-startup world definition."""

    def __init__(self) -> None:
        self.rooms: Dict[str, Room] = {}
        self.items: Dict[str, Item] = {}
        self.npcs: Dict[str, Npc] = {}
        self.quests: Dict[str, Quest] = {}
        self.start_room: str = ""
        self.sleep_room: str = ""
        self.name: str = "The Answer Protocol"

    # -- loading ---------------------------------------------------------
    @classmethod
    def from_dict(cls, data: Dict[str, Any]) -> "World":
        if not isinstance(data, dict):
            raise WorldError("the world file must contain a JSON object")
        world = cls()
        world.name = data.get("name", world.name)

        def entries(section: str) -> List[Dict[str, Any]]:
            value = data.get(section, [])
            if value is None:
                return []
            if not isinstance(value, list):
                raise WorldError(f"{section} must be an array")
            if any(not isinstance(entry, dict) for entry in value):
                raise WorldError(f"every {section} entry must be an object")
            return value

        for raw in entries("items"):
            item_id = raw.get("id")
            if not isinstance(item_id, str) or not item_id:
                raise WorldError("an item entry has no id")
            canonical_id = canonical(item_id, ITEM_PREFIX)
            if canonical_id in world.items:
                raise WorldError(f"duplicate item id: {canonical_id}")
            try:
                damage = int(raw["damage"]) if raw.get("damage") is not None else None
                armor = int(raw["armor"]) if raw.get("armor") is not None else None
                heal = int(raw["heal"]) if raw.get("heal") is not None else None
                value = int(raw.get("value", 0))
            except (TypeError, ValueError):
                raise WorldError(f"item {canonical_id}: combat values and value must be integers") from None
            if any(number is not None and number < 0 for number in (damage, armor, heal)) or value < 0:
                raise WorldError(f"item {canonical_id}: values cannot be negative")
            world.items[canonical_id] = Item(
                id=canonical_id,
                name=raw.get("name", canonical_id),
                description=raw.get("description", ""),
                damage=damage,
                armor=armor,
                heal=heal,
                value=value,
                obtainable=bool(raw.get("obtainable", True)),
            )

        for raw in entries("npcs"):
            npc_id = raw.get("id")
            if not isinstance(npc_id, str) or not npc_id:
                raise WorldError("an npc entry has no id")
            canonical_id = canonical(npc_id, NPC_PREFIX)
            if canonical_id in world.npcs:
                raise WorldError(f"duplicate npc id: {canonical_id}")
            role = raw.get("role", "dialogue")
            if role not in NPC_ROLES:
                raise WorldError(f"npc {canonical_id}: unknown role {role!r}")
            dialogue = raw.get("dialogue", [])
            if dialogue is None:
                dialogue = []
            if isinstance(dialogue, str):
                dialogue = [dialogue]
            if not isinstance(dialogue, list):
                raise WorldError(f"npc {canonical_id}: dialogue must be a string or array")
            try:
                hp = int(raw.get("hp", 20))
                max_hp = int(raw.get("max_hp", hp))
                damage = int(raw.get("damage", 5))
                gold_drop = int(raw.get("gold_drop", 0))
            except (TypeError, ValueError):
                raise WorldError(f"npc {canonical_id}: combat values must be integers") from None
            if hp <= 0 or max_hp <= 0:
                raise WorldError(f"npc {canonical_id}: hp must be positive")
            if max_hp < hp:
                raise WorldError(f"npc {canonical_id}: max_hp cannot be below hp")
            if damage < 0 or gold_drop < 0:
                raise WorldError(f"npc {canonical_id}: damage and gold_drop cannot be negative")
            world.npcs[canonical_id] = Npc(
                id=canonical_id,
                name=raw.get("name", canonical_id),
                role=role,
                description=raw.get("description", ""),
                dialogue=[str(line) for line in dialogue],
                hp=hp,
                max_hp=max_hp,
                damage=damage,
                gold_drop=gold_drop,
            )

        for raw in entries("rooms"):
            room_id = raw.get("id")
            if not isinstance(room_id, str) or not room_id:
                raise WorldError("a room entry has no id")
            canonical_id = canonical(room_id, ROOM_PREFIX)
            if canonical_id in world.rooms:
                raise WorldError(f"duplicate room id: {canonical_id}")
            exits_raw = raw.get("exits") or {}
            if not isinstance(exits_raw, dict):
                raise WorldError(f"room {canonical_id}: exits must be an object")
            exits: Dict[str, str] = {}
            for direction, target in exits_raw.items():
                key = str(direction).lower()
                if key not in DIRECTIONS:
                    raise WorldError(f"room {canonical_id}: unknown direction {direction!r}")
                if not isinstance(target, str) or not target:
                    raise WorldError(f"room {canonical_id}: exit {direction} needs a room id")
                exits[key] = canonical(target, ROOM_PREFIX)
            room_items = raw.get("items", [])
            room_npcs = raw.get("npcs", [])
            if not isinstance(room_items, list) or not isinstance(room_npcs, list):
                raise WorldError(f"room {canonical_id}: items and npcs must be arrays")
            world.rooms[canonical_id] = Room(
                id=canonical_id,
                name=raw.get("name", canonical_id),
                description=raw.get("description", ""),
                exits=exits,
                items=[canonical(str(i), ITEM_PREFIX) for i in room_items],
                npcs=[canonical(str(n), NPC_PREFIX) for n in room_npcs],
                can_sleep=bool(raw.get("can_sleep", False)),
            )

        for raw in entries("quests"):
            quest_id = raw.get("id")
            if not isinstance(quest_id, str) or not quest_id:
                raise WorldError("a quest entry has no id")
            canonical_id = canonical(quest_id, QUEST_PREFIX)
            if canonical_id in world.quests:
                raise WorldError(f"duplicate quest id: {canonical_id}")
            quest_type = raw.get("type", "fetch")
            if quest_type not in QUEST_TYPES:
                raise WorldError(f"quest {canonical_id}: unknown type {quest_type!r}")
            try:
                target_count = int(raw.get("target_count", 1))
                reward_count = int(raw.get("reward_count", 1))
            except (TypeError, ValueError):
                raise WorldError(f"quest {canonical_id}: counts must be integers") from None
            world.quests[canonical_id] = Quest(
                id=canonical_id,
                giver=canonical(str(raw.get("giver", "")), NPC_PREFIX),
                description=raw.get("description", ""),
                type=quest_type,
                target=canonical(str(raw.get("target", "")), NPC_PREFIX
                                 if quest_type == "kill" else ITEM_PREFIX),
                target_count=target_count,
                reward=canonical(str(raw.get("reward", "")), ITEM_PREFIX),
                reward_count=reward_count,
                requires=canonical(str(raw["requires"]), QUEST_PREFIX) if raw.get("requires") else "",
                recipient=canonical(str(raw["recipient"]), NPC_PREFIX) if raw.get("recipient") else "",
                name=raw.get("name", ""),
            )

        world.start_room = canonical(str(data.get("start_room", "")), ROOM_PREFIX)
        world.sleep_room = canonical(str(data.get("sleep_room", "")), ROOM_PREFIX)
        world.validate()
        return world

    @classmethod
    def load(cls, path: str) -> "World":
        try:
            with open(path, encoding="utf-8") as handle:
                data = json.load(handle)
        except FileNotFoundError:
            raise WorldError(f"world file not found: {path}") from None
        except json.JSONDecodeError as exc:
            raise WorldError(f"world file is not valid JSON ({path}): {exc}") from None
        return cls.from_dict(data)

    # -- validation ------------------------------------------------------
    def validate(self) -> List[str]:
        """Raise :class:`WorldError` on the first integrity problem."""
        problems = self.integrity_problems()
        if problems:
            raise WorldError("; ".join(problems))
        return []

    def integrity_problems(self) -> List[str]:
        """Every referential problem, in a stable order (empty when the world is sound)."""
        problems: List[str] = []
        if not self.rooms:
            problems.append("the world has no room")
        for room in self.rooms.values():
            for direction, target in room.exits.items():
                if target not in self.rooms:
                    problems.append(
                        f"room {room.id}: exit {direction} targets unknown room {target}"
                    )
            for item_id in room.items:
                if item_id not in self.items:
                    problems.append(f"room {room.id}: unknown item {item_id}")
            for npc_id in room.npcs:
                if npc_id not in self.npcs:
                    problems.append(f"room {room.id}: unknown npc {npc_id}")
        for quest in self.quests.values():
            if quest.giver not in self.npcs:
                problems.append(f"quest {quest.id}: unknown giver {quest.giver}")
            elif self.npcs[quest.giver].role != "quest_giver":
                problems.append(
                    f"quest {quest.id}: giver {quest.giver} is not a quest_giver"
                )
            expected = self.npcs if quest.type == "kill" else self.items
            if quest.target and quest.target not in expected:
                problems.append(f"quest {quest.id}: unknown target {quest.target}")
            elif not quest.target:
                problems.append(f"quest {quest.id}: no target")
            if quest.reward and quest.reward not in self.items:
                problems.append(f"quest {quest.id}: unknown reward {quest.reward}")
            if quest.requires:
                if quest.requires not in self.quests:
                    problems.append(f"quest {quest.id}: unknown prerequisite {quest.requires}")
                elif quest.requires == quest.id:
                    problems.append(f"quest {quest.id}: requires itself")
            if quest.type == "deliver":
                if not quest.recipient:
                    problems.append(f"quest {quest.id}: deliver quest without recipient")
                elif quest.recipient not in self.npcs:
                    problems.append(f"quest {quest.id}: unknown recipient {quest.recipient}")
            if quest.target_count <= 0:
                problems.append(f"quest {quest.id}: target_count must be positive")
            if quest.reward and quest.reward_count <= 0:
                problems.append(f"quest {quest.id}: reward_count must be positive")
        if self.start_room:
            if self.start_room not in self.rooms:
                problems.append(f"start_room targets unknown room {self.start_room}")
        elif self.rooms:
            problems.append("no start_room is defined")
        if self.sleep_room and self.sleep_room not in self.rooms:
            problems.append(f"sleep_room targets unknown room {self.sleep_room}")
        return problems

    # -- lookups ---------------------------------------------------------
    def room(self, room_id: str) -> Room:
        return self.rooms[room_id]

    def quest_for_giver(self, npc_id: str) -> Optional[Quest]:
        """The first quest this NPC gives, in declaration order."""
        for quest in self.quests.values():
            if quest.giver == npc_id:
                return quest
        return None

    def quests_for_giver(self, npc_id: str) -> List[Quest]:
        return [quest for quest in self.quests.values() if quest.giver == npc_id]

    def stats(self) -> Dict[str, int]:
        return {
            "rooms": len(self.rooms),
            "items": len(self.items),
            "npcs": len(self.npcs),
            "quests": len(self.quests),
        }

    def to_dict(self) -> Dict[str, Any]:
        return {
            "name": self.name,
            "start_room": self.start_room,
            "sleep_room": self.sleep_room,
            "rooms": [room.to_dict() for room in self.rooms.values()],
            "items": [item.to_dict() for item in self.items.values()],
            "npcs": [npc.to_dict() for npc in self.npcs.values()],
            "quests": [quest.to_dict() for quest in self.quests.values()],
        }


def default_world_path() -> str:
    """``data/world.json`` next to the project root."""
    root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    return os.path.join(root, "data", "world.json")
