"""Shared test fixtures for the TAP project."""

from __future__ import annotations

import copy
import sys
from pathlib import Path
from typing import Any, Dict

import pytest

ROOT = Path(__file__).resolve().parents[1]
SRC = ROOT / "src"
if str(SRC) not in sys.path:
    sys.path.insert(0, str(SRC))

from state import GameState, Session  # noqa: E402
from world import World  # noqa: E402


class FakeClock:
    def __init__(self, value: float = 100.0) -> None:
        self.value = value

    def __call__(self) -> float:
        return self.value

    def advance(self, seconds: float) -> None:
        self.value += seconds


@pytest.fixture
def world_data() -> Dict[str, Any]:
    return {
        "name": "Test Frontier",
        "start_room": "atrium",
        "sleep_room": "bunk",
        "items": [
            {"id": "key", "name": "Silver Key", "description": "A little key.", "value": 4},
            {"id": "sword", "name": "Sword", "damage": 7, "value": 20},
            {"id": "coat", "name": "Shield Coat", "armor": 6},
            {"id": "gift", "name": "Gift", "heal": 10},
            {"id": "token", "name": "Token"},
        ],
        "npcs": [
            {"id": "giver", "name": "Mira", "role": "quest_giver",
             "dialogue": ["First line.", "Second line."], "hp": 20},
            {"id": "recipient", "name": "Sage", "role": "dialogue",
             "dialogue": ["Welcome."], "hp": 20},
            {"id": "guard", "name": "Guard", "role": "enemy", "hp": 20,
             "damage": 5, "gold_drop": 3},
            {"id": "guide", "name": "Guide", "role": "dialogue"},
        ],
        "rooms": [
            {"id": "atrium", "name": "Atrium", "description": "A test room.",
             "exits": {"east": "bunk", "north": "hall"},
             "items": ["key", "sword", "coat"], "npcs": ["giver", "guide"]},
            {"id": "bunk", "name": "Bunk Room", "description": "A safe bed.",
             "exits": {"west": "atrium"}, "items": ["gift"], "npcs": ["recipient"],
             "can_sleep": True},
            {"id": "hall", "name": "Hall", "description": "A long hall.",
             "exits": {"south": "atrium"}, "items": ["token"], "npcs": ["guard"]},
        ],
        "quests": [
            {"id": "fetch_key", "giver": "giver", "description": "Bring the key.",
             "type": "fetch", "target": "key", "target_count": 1, "reward": "gift"},
            {"id": "kill_guard", "giver": "giver", "description": "Defeat the guard.",
             "type": "kill", "target": "guard", "target_count": 1, "reward": "sword",
             "requires": "fetch_key"},
            {"id": "deliver_token", "giver": "giver", "description": "Deliver the token.",
             "type": "deliver", "target": "token", "target_count": 1,
             "reward": "gift", "recipient": "recipient"},
        ],
    }


@pytest.fixture
def world(world_data):
    return World.from_dict(copy.deepcopy(world_data))


@pytest.fixture
def fake_clock():
    return FakeClock()


@pytest.fixture
def state(world, fake_clock):
    return GameState(world, clock=fake_clock)


@pytest.fixture
def session():
    return Session()


def connect(state: GameState, name: str = "alice") -> Session:
    session = Session()
    result = state.dispatch(session, f"CONNECT {name}")
    assert result == "OK Connected"
    return session
