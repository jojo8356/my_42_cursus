from __future__ import annotations

from state import GameState, Group, Player, Session


def login(state, name="alice"):
    session = Session()
    assert state.dispatch(session, f"CONNECT {name}") == "OK Connected"
    return session


def test_room_and_group_broadcasts_skip_excluded_and_missing_recipients(world):
    state = GameState(world)
    alice, bob = login(state), login(state, "bob")
    state.broadcast_room("loc.atrium", "for alice", exclude=["bob"])
    assert "for alice" in alice.sent and "for alice" not in bob.sent
    state.groups["squad"] = Group("squad", "alice", ["alice", "ghost", "bob"])
    state.broadcast_group("squad", "group only", exclude=["bob"])
    assert "group only" in alice.sent and "group only" not in bob.sent
    state.sessions.pop("ghost", None)
    state.broadcast_group("squad", "no ghost session")


def test_quest_rejects_completed_entry_and_unmet_prerequisite(world):
    state = GameState(world)
    alice = login(state)
    state._quest_for_giver = lambda *_args: world.quests["quest.fetch_key"]
    state.players["alice"].quests["quest.fetch_key"] = {
        "status": "completed", "progress": 1
    }
    assert state.dispatch(alice, "QUEST Mira") == "ERR 406 NO_QUEST_AVAILABLE"

    state.players["alice"].quests.clear()
    state._quest_for_giver = lambda *_args: world.quests["quest.kill_guard"]
    assert state.dispatch(alice, "QUEST Mira") == "ERR 406 NO_QUEST_AVAILABLE"


def test_quest_with_missing_reward_does_not_grant_an_unknown_item(world):
    state = GameState(world)
    login(state)
    quest = world.quests["quest.fetch_key"]
    quest.reward = "item.not_in_world"
    player = state.players["alice"]
    player.inventory.append("item.key")
    result = state._complete_quest(player, quest)
    assert '"status":"completed"' in result
    assert "item.key" not in player.inventory
    assert "item.not_in_world" not in player.inventory


def test_victory_with_zero_gold_drop_and_flee_without_previous_room(world):
    world.npcs["npc.guard"].gold_drop = 0
    state = GameState(world)
    alice = login(state)
    state.dispatch(alice, "MOVE north")
    state.dispatch(alice, "ATTACK guard")
    state.players["alice"].previous_room = ""
    victory = state.dispatch(alice, "ATTACK guard")
    assert '"status":"victory"' in victory
    assert 'gold_earned' not in victory

    # A fresh game and a strong guard lets us test FLEE without a previous room.
    world.npcs["npc.guard"].hp = world.npcs["npc.guard"].max_hp = 200
    state = GameState(world)
    alice = login(state)
    state.dispatch(alice, "MOVE north")
    state.dispatch(alice, "ATTACK guard")
    state.players["alice"].previous_room = ""
    fled = state.dispatch(alice, "FLEE")
    assert '"room":"loc.hall"' in fled


def test_group_invitation_without_live_session_and_leave_without_member_entry(world):
    state = GameState(world)
    alice, bob = login(state), login(state, "bob")
    state.sessions.pop("bob")
    state.dispatch(alice, "GROUP CREATE squad")
    assert state.dispatch(alice, "GROUP INVITE bob") == "OK"
    assert "bob" in state.groups["squad"].invited

    outsider = Player("outsider", "loc.atrium", group="squad")
    state.groups["squad"].players.append("alice")
    events = state._leave_group(outsider)
    assert outsider.group == "" and events


def test_abandon_quest_matches_canonical_id_exactly(world):
    state = GameState(world)
    alice = login(state)
    state.dispatch(alice, "QUEST Mira")
    result = state.dispatch(alice, "ABANDON_QUEST quest.fetch_key")
    assert result == 'OK {"quest_id":"quest.fetch_key","status":"abandoned"}'
