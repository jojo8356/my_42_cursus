from __future__ import annotations

import copy

import pytest

from world import World, WorldError


def test_optional_sections_none_and_optional_npc_dialogue_shapes(world_data):
    data = copy.deepcopy(world_data)
    data["items"] = data["npcs"] = data["quests"] = None
    data["rooms"] = [{"id": "lone", "exits": None, "items": [], "npcs": []}]
    data["start_room"] = "lone"
    data["sleep_room"] = ""
    world = World.from_dict(data)
    assert world.stats() == {"rooms": 1, "items": 0, "npcs": 0, "quests": 0}
    assert world.rooms["loc.lone"].exits == {}

    data = copy.deepcopy(world_data)
    data["npcs"][0]["dialogue"] = None
    world = World.from_dict(data)
    assert world.npcs["npc.giver"].dialogue == []
    data["npcs"][0]["dialogue"] = "One quoted line"
    world = World.from_dict(data)
    assert world.npcs["npc.giver"].dialogue == ["One quoted line"]


def test_npc_and_quest_ids_are_required_and_quest_target_cannot_be_empty(world_data):
    data = copy.deepcopy(world_data)
    data["npcs"][0].pop("id")
    with pytest.raises(WorldError, match="npc entry has no id"):
        World.from_dict(data)
    data = copy.deepcopy(world_data)
    data["quests"][0].pop("id")
    with pytest.raises(WorldError, match="quest entry has no id"):
        World.from_dict(data)
    data = copy.deepcopy(world_data)
    data["quests"][0]["target"] = ""
    with pytest.raises(WorldError, match="no target"):
        World.from_dict(data)


def test_world_quest_lookup_returns_none_for_unknown_giver(world):
    assert world.quest_for_giver("npc.unknown") is None
    assert world.quests_for_giver("npc.unknown") == []
