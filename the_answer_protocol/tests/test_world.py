from __future__ import annotations

import copy
import json

import pytest

from world import World, WorldError, canonical, default_world_path


def test_packaged_arrakis_world_is_valid():
    world = World.load(default_world_path())
    assert world.name == "Arrakis"
    assert world.stats() == {"rooms": 9, "items": 8, "npcs": 7, "quests": 3}
    assert world.start_room == "loc.arrakeen_gate"
    assert world.sleep_room == "loc.guest_room"
    assert world.integrity_problems() == []


def test_world_round_trip_and_lookup(world):
    restored = World.from_dict(world.to_dict())
    assert restored.stats() == world.stats()
    assert restored.room("loc.atrium").name == "Atrium"
    assert restored.quests_for_giver("npc.giver")[0].id == "quest.fetch_key"
    assert restored.quest_for_giver("npc.giver").id == "quest.fetch_key"


def test_canonical_ids_preserve_known_prefix_and_map_room_alias():
    assert canonical("hall", "loc.") == "loc.hall"
    assert canonical("loc.hall", "loc.") == "loc.hall"
    assert canonical("room.hall", "loc.") == "loc.hall"
    assert canonical("npc.guard", "item.") == "npc.guard"
    assert canonical("some.deep.id", "item.") == "item.some.deep.id"
    assert canonical("", "loc.") == ""


def test_world_data_canonicalizes_all_references(world_data):
    source = copy.deepcopy(world_data)
    source["rooms"][0]["id"] = "room.atrium"
    source["rooms"][0]["exits"]["east"] = "room.bunk"
    source["start_room"] = "room.atrium"
    world = World.from_dict(source)
    assert world.start_room == "loc.atrium"
    assert "loc.bunk" in world.rooms["loc.atrium"].exits.values()


def test_entities_render_wire_friendly_dicts(world):
    item = world.items["item.sword"].to_dict()
    assert item["id"] == "item.sword" and item["damage"] == 7
    npc = world.npcs["npc.guard"].to_dict()
    assert npc["hostile"] is True and npc["max_hp"] == 20
    quest = world.quests["quest.fetch_key"].to_dict()
    assert quest["giver"] == "npc.giver" and quest["target"] == "item.key"
    room = world.rooms["loc.bunk"].to_dict()
    assert room["can_sleep"] is True


def test_world_load_missing_file_raises_world_error(tmp_path):
    with pytest.raises(WorldError, match="world file not found"):
        World.load(str(tmp_path / "missing.json"))


def test_world_load_malformed_json_raises_world_error(tmp_path):
    path = tmp_path / "broken.json"
    path.write_text("{ nope", encoding="utf-8")
    with pytest.raises(WorldError, match="not valid JSON"):
        World.load(str(path))


def test_world_load_valid_json_and_non_object(world_data, tmp_path):
    path = tmp_path / "world.json"
    path.write_text(json.dumps(world_data), encoding="utf-8")
    assert World.load(str(path)).name == "Test Frontier"
    with pytest.raises(WorldError, match="JSON object"):
        World.from_dict([])


def test_empty_world_is_rejected():
    with pytest.raises(WorldError, match="world has no room"):
        World.from_dict({"rooms": []})


def test_missing_start_room_is_reported():
    data = {"rooms": [{"id": "only"}]}
    with pytest.raises(WorldError, match="no start_room"):
        World.from_dict(data)


def test_unknown_start_and_sleep_rooms_are_reported(world_data):
    data = copy.deepcopy(world_data)
    data["start_room"] = "nowhere"
    data["sleep_room"] = "elsewhere"
    with pytest.raises(WorldError) as caught:
        World.from_dict(data)
    assert "start_room targets unknown room loc.nowhere" in str(caught.value)
    assert "sleep_room targets unknown room loc.elsewhere" in str(caught.value)


def test_unknown_exit_item_and_npc_are_reported(world_data):
    data = copy.deepcopy(world_data)
    data["rooms"][0]["exits"]["west"] = "missing_room"
    data["rooms"][0]["items"].append("missing_item")
    data["rooms"][0]["npcs"].append("missing_npc")
    with pytest.raises(WorldError) as caught:
        World.from_dict(data)
    text = str(caught.value)
    assert "exit west targets unknown room loc.missing_room" in text
    assert "unknown item item.missing_item" in text
    assert "unknown npc npc.missing_npc" in text


def test_duplicate_canonical_item_ids_rejected(world_data):
    data = copy.deepcopy(world_data)
    data["items"].append({"id": "item.key", "name": "Duplicate"})
    with pytest.raises(WorldError, match="duplicate item id: item.key"):
        World.from_dict(data)


def test_item_without_id_rejected(world_data):
    data = copy.deepcopy(world_data)
    data["items"][0].pop("id")
    with pytest.raises(WorldError, match="item entry has no id"):
        World.from_dict(data)


def test_duplicate_npc_room_and_quest_ids_rejected(world_data):
    data = copy.deepcopy(world_data)
    data["npcs"].append({"id": "giver", "name": "Copy"})
    with pytest.raises(WorldError, match="duplicate npc id"):
        World.from_dict(data)
    data = copy.deepcopy(world_data)
    data["rooms"].append({"id": "atrium"})
    with pytest.raises(WorldError, match="duplicate room id"):
        World.from_dict(data)
    data = copy.deepcopy(world_data)
    data["quests"].append(copy.deepcopy(data["quests"][0]))
    with pytest.raises(WorldError, match="duplicate quest id"):
        World.from_dict(data)


def test_npc_role_and_positive_hp_validation(world_data):
    bad_role = copy.deepcopy(world_data)
    bad_role["npcs"][0]["role"] = "wizard"
    with pytest.raises(WorldError, match="unknown role"):
        World.from_dict(bad_role)
    bad_hp = copy.deepcopy(world_data)
    bad_hp["npcs"][0]["hp"] = 0
    with pytest.raises(WorldError, match="hp must be positive"):
        World.from_dict(bad_hp)


def test_room_exit_shape_and_direction_validation(world_data):
    bad_shape = copy.deepcopy(world_data)
    bad_shape["rooms"][0]["exits"] = ["north"]
    with pytest.raises(WorldError, match="exits must be an object"):
        World.from_dict(bad_shape)
    bad_direction = copy.deepcopy(world_data)
    bad_direction["rooms"][0]["exits"]["diagonal"] = "bunk"
    with pytest.raises(WorldError, match="unknown direction"):
        World.from_dict(bad_direction)


def test_quest_type_target_giver_reward_and_prerequisite_validation(world_data):
    cases = [
        (lambda d: d["quests"][0].update(type="teleport"), "unknown type"),
        (lambda d: d["quests"][0].update(giver="missing"), "unknown giver"),
        (lambda d: d["quests"][0].update(giver="guide"), "not a quest_giver"),
        (lambda d: d["quests"][0].update(target="missing"), "unknown target"),
        (lambda d: d["quests"][0].update(reward="missing"), "unknown reward"),
        (lambda d: d["quests"][0].update(requires="missing"), "unknown prerequisite"),
        (lambda d: d["quests"][0].update(requires="fetch_key"), "requires itself"),
        (lambda d: d["quests"][0].update(target_count=0), "target_count must be positive"),
    ]
    for mutate, text in cases:
        data = copy.deepcopy(world_data)
        mutate(data)
        with pytest.raises(WorldError, match=text):
            World.from_dict(data)


def test_deliver_quest_requires_a_valid_recipient(world_data):
    missing = copy.deepcopy(world_data)
    missing["quests"][2].pop("recipient")
    with pytest.raises(WorldError, match="deliver quest without recipient"):
        World.from_dict(missing)
    unknown = copy.deepcopy(world_data)
    unknown["quests"][2]["recipient"] = "not_here"
    with pytest.raises(WorldError, match="unknown recipient"):
        World.from_dict(unknown)


def test_giver_must_be_a_quest_giver(world_data):
    data = copy.deepcopy(world_data)
    data["quests"][0]["giver"] = "guide"
    with pytest.raises(WorldError, match="is not a quest_giver"):
        World.from_dict(data)


def test_integrity_problems_are_stable_and_validate_raises(world_data):
    data = copy.deepcopy(world_data)
    data["rooms"][0]["exits"]["west"] = "void"
    with pytest.raises(WorldError) as caught:
        World.from_dict(data)
    assert "exit west targets unknown room loc.void" in str(caught.value)


def test_world_to_dict_contains_start_sleep_and_all_entities(world):
    payload = world.to_dict()
    assert payload["start_room"] == "loc.atrium"
    assert payload["sleep_room"] == "loc.bunk"
    assert len(payload["rooms"]) == 3
    assert len(payload["items"]) == 5
    assert len(payload["npcs"]) == 4
    assert len(payload["quests"]) == 3
