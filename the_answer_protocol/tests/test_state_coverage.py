from __future__ import annotations

import pytest

import protocol
from protocol import Command, ProtocolError
from state import GameState, Group, Player, Session, _match, _match_quest
from world import World, default_world_path


def login(state, name):
    session = Session()
    assert state.dispatch(session, f"CONNECT {name}") == "OK Connected"
    return session


def test_broadcast_exclusion_stale_group_members_and_direct_connect_errors(world):
    state = GameState(world)
    alice = login(state, "alice")
    bob = login(state, "bob")
    state.broadcast_all("only bob", exclude=["alice"])
    assert "only bob" not in alice.sent and "only bob" in bob.sent
    state.groups["stale"] = Group("stale", "alice", ["alice", "ghost"])
    state.broadcast_group("stale", "for alice", exclude=["ghost"])
    assert "for alice" in alice.sent
    with pytest.raises(ProtocolError, match="BAD_REQUEST"):
        state.connect(Session(), "")
    assert state.dispatch(alice, "GROUP WAT") == "ERR 400 UNKNOWN_COMMAND"
    assert state.dispatch(alice, "EXAMINE") == "ERR 400 MISSING_ARGUMENT"


def test_quest_unavailable_delivery_wrong_giver_and_abandon_validation(world):
    state = GameState(world)
    alice = login(state, "alice")
    state.dispatch(alice, "QUEST Mira")
    assert state.dispatch(alice, "QUEST Mira") == "ERR 408 QUEST_NOT_COMPLETE"
    # Active fetch objectives cannot be turned in at a different quest giver.
    state.dispatch(alice, "MOVE east")
    assert state.dispatch(alice, "QUEST Sage") == "ERR 406 NO_QUEST_AVAILABLE"
    assert state.dispatch(alice, "ABANDON_QUEST") == "ERR 400 MISSING_ARGUMENT"


def test_quest_giver_reports_none_after_all_quests_are_complete(world):
    state = GameState(world)
    alice = login(state, "alice")
    # A quest giver with no available quests returns the normative 406 symbol.
    world.quests.clear()
    assert state.dispatch(alice, "QUEST Mira") == "ERR 406 NO_QUEST_AVAILABLE"
    assert world.quest_for_giver("npc.giver") is None
    assert world.quests_for_giver("npc.giver") == []


def test_quest_delivery_giver_and_completion_without_declared_reward(world):
    quest = world.quests["quest.deliver_token"]
    quest.reward = ""
    state = GameState(world)
    alice = login(state, "alice")
    state.players["alice"].quests["quest.fetch_key"] = {"status": "completed", "progress": 1}
    state.players["alice"].quests["quest.kill_guard"] = {"status": "completed", "progress": 1}
    state.players["alice"].quests[quest.id] = {"status": "active", "progress": 0}
    assert state.dispatch(alice, "QUEST Mira") == "ERR 408 QUEST_NOT_COMPLETE"
    state.players["alice"].inventory.append("item.token")
    state.dispatch(alice, "MOVE east")
    assert state.dispatch(alice, "QUEST Sage") == (
        'OK {"quest_id":"quest.deliver_token","status":"completed",'
        '"reward":"","reward_count":1}'
    )
    assert "item.token" not in state.players["alice"].inventory


def test_quest_ineligible_prerequisite_is_skipped_and_no_available_returns_none(world):
    world.quests["quest.fetch_key"].requires = "quest.kill_guard"
    world.quests["quest.deliver_token"].requires = "quest.fetch_key"
    state = GameState(world)
    alice = login(state, "alice")
    # All three quests are locked because their prerequisite has not completed.
    assert state._quest_for_giver(state.players["alice"], "npc.giver") is None
    assert state.dispatch(alice, "QUEST Mira") == "ERR 406 NO_QUEST_AVAILABLE"
    assert state.dispatch(alice, "ABANDON_QUEST nobody") == "ERR 404 QUEST_NOT_ACTIVE"


def test_completed_quest_cannot_be_abandoned_and_attack_sleep_dead_errors(world):
    state = GameState(world)
    alice = login(state, "alice")
    state.dispatch(alice, "QUEST Mira")
    state.players["alice"].quests["quest.fetch_key"]["status"] = "completed"
    assert state.dispatch(alice, "ABANDON_QUEST fetch_key") == "ERR 404 QUEST_NOT_ACTIVE"
    assert state.dispatch(alice, "ATTACK guard") == "ERR 404 NPC_NOT_FOUND"
    state.players["alice"].dead = True
    assert state.dispatch(alice, "ATTACK guard") == "ERR 409 PLAYER_DEAD"
    assert state.dispatch(alice, "SLEEP") == "ERR 409 PLAYER_DEAD"


def test_group_invitation_with_missing_session_join_by_inviter_and_all_role_errors(world):
    state = GameState(world)
    alice = login(state, "alice")
    bob = login(state, "bob")
    assert state.dispatch(alice, "GROUP CREATE squad") == "OK group=squad"
    assert state.dispatch(bob, "GROUP JOIN") == "ERR 400 MISSING_ARGUMENT"
    assert state.dispatch(bob, "GROUP JOIN absent") == "ERR 404 GROUP_NOT_FOUND"
    state.dispatch(alice, "GROUP INVITE bob")
    # The protocol also accepts the inviter's name rather than the group id.
    assert state.dispatch(bob, "GROUP JOIN alice") == "OK group=squad"
    assert state.dispatch(bob, "GROUP KICK alice") == "ERR 403 NOT_GROUP_LEADER"
    assert state.dispatch(bob, "GROUP DISBAND") == "ERR 403 NOT_GROUP_LEADER"
    assert state.dispatch(bob, "GROUP INFO").startswith("OK {")
    assert state.dispatch(alice, "GROUP KICK bob") == "OK"
    assert state.dispatch(alice, "GROUP LEAVE") == "OK"
    assert "squad" not in state.groups
    assert state.dispatch(alice, "GROUP KICK bob") == "ERR 401 NOT_IN_GROUP"
    assert state.dispatch(alice, "GROUP DISBAND") == "ERR 401 NOT_IN_GROUP"
    assert state.dispatch(alice, "GROUP INFO") == "ERR 401 NOT_IN_GROUP"


def test_group_disband_notifies_all_and_handles_stale_members(world):
    state = GameState(world)
    alice = login(state, "alice")
    bob = login(state, "bob")
    assert state.dispatch(alice, "GROUP CREATE squad") == "OK group=squad"
    state.dispatch(alice, "GROUP INVITE bob")
    state.dispatch(bob, "GROUP JOIN squad")
    assert state.dispatch(alice, "GROUP DISBAND") == "OK"
    assert "EVT GROUP DISBAND" in alice.sent
    assert "EVT GROUP DISBAND" in bob.sent
    assert not state.groups and not state.players["bob"].group

    state.groups["stale"] = Group("stale", "ghost", ["ghost"])
    ghost = Player("ghost", "loc.atrium", group="stale")
    state._leave_group(ghost)
    assert "stale" not in state.groups and ghost.group == ""


def test_group_cleanup_of_stale_kick_and_disband_records(world):
    state = GameState(world)
    alice = login(state, "alice")
    bob = login(state, "bob")
    state.dispatch(alice, "GROUP CREATE squad")
    state.dispatch(alice, "GROUP INVITE bob")
    state.dispatch(bob, "GROUP JOIN squad")
    state.sessions.pop("bob")
    assert state.dispatch(alice, "GROUP KICK bob") == "OK"
    assert "bob" not in state.groups["squad"].players
    state.groups["ghosts"] = Group("ghosts", "alice", ["alice", "missing"])
    state.players["alice"].group = "ghosts"
    assert state.dispatch(alice, "GROUP DISBAND") == "OK"
    assert "ghosts" not in state.groups
