from __future__ import annotations

import copy

import pytest

from world import World, WorldError


def test_world_sections_and_entries_must_be_arrays_of_objects(world_data):
    data = copy.deepcopy(world_data)
    data["items"] = {"id": "key"}
    with pytest.raises(WorldError, match="items must be an array"):
        World.from_dict(data)
    data = copy.deepcopy(world_data)
    data["npcs"] = ["not an object"]
    with pytest.raises(WorldError, match="every npcs entry must be an object"):
        World.from_dict(data)
    data = copy.deepcopy(world_data)
    data["rooms"] = [None]
    with pytest.raises(WorldError, match="every rooms entry must be an object"):
        World.from_dict(data)
    data = copy.deepcopy(world_data)
    data["quests"] = "quest"
    with pytest.raises(WorldError, match="quests must be an array"):
        World.from_dict(data)


def test_room_items_npcs_and_exit_targets_have_valid_shapes(world_data):
    data = copy.deepcopy(world_data)
    data["rooms"][0]["items"] = "key"
    with pytest.raises(WorldError, match="items and npcs must be arrays"):
        World.from_dict(data)
    data = copy.deepcopy(world_data)
    data["rooms"][0]["exits"] = {"north": None}
    with pytest.raises(WorldError, match="needs a room id"):
        World.from_dict(data)


def test_npc_dialogue_and_numeric_values_are_validated(world_data):
    data = copy.deepcopy(world_data)
    data["npcs"][0]["dialogue"] = {"line": "wrong"}
    with pytest.raises(WorldError, match="dialogue must be a string or array"):
        World.from_dict(data)
    data = copy.deepcopy(world_data)
    data["npcs"][0]["max_hp"] = 1
    with pytest.raises(WorldError, match="max_hp cannot be below hp"):
        World.from_dict(data)
    data = copy.deepcopy(world_data)
    data["npcs"][0]["damage"] = -1
    with pytest.raises(WorldError, match="cannot be negative"):
        World.from_dict(data)
    data = copy.deepcopy(world_data)
    data["npcs"][0]["hp"] = "high"
    with pytest.raises(WorldError, match="combat values must be integers"):
        World.from_dict(data)


def test_item_combat_fields_are_nonnegative_integers(world_data):
    data = copy.deepcopy(world_data)
    data["items"][0]["damage"] = -1
    with pytest.raises(WorldError, match="values cannot be negative"):
        World.from_dict(data)
    data = copy.deepcopy(world_data)
    data["items"][0]["armor"] = "heavy"
    with pytest.raises(WorldError, match="must be integers"):
        World.from_dict(data)


def test_quest_counts_and_reward_count_are_checked(world_data):
    data = copy.deepcopy(world_data)
    data["quests"][0]["reward_count"] = 0
    with pytest.raises(WorldError, match="reward_count must be positive"):
        World.from_dict(data)
    data = copy.deepcopy(world_data)
    data["quests"][0]["target_count"] = "many"
    with pytest.raises(WorldError, match="counts must be integers"):
        World.from_dict(data)


def test_non_string_entity_ids_are_rejected(world_data):
    data = copy.deepcopy(world_data)
    data["items"][0]["id"] = 4
    with pytest.raises(WorldError, match="item entry has no id"):
        World.from_dict(data)
    data = copy.deepcopy(world_data)
    data["rooms"][0]["id"] = 4
    with pytest.raises(WorldError, match="room entry has no id"):
        World.from_dict(data)
