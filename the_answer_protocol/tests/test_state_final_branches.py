from __future__ import annotations

from state import GameState, Session


def login(state, name="alice"):
    session = Session()
    assert state.dispatch(session, f"CONNECT {name}") == "OK Connected"
    return session


def test_room_broadcast_skips_player_without_a_session(world):
    state = GameState(world)
    login(state)
    state.players["orphan"] = state.players["alice"].__class__("orphan", "loc.atrium")
    state.sessions.pop("alice")
    state.broadcast_room("loc.atrium", "room event")
    assert state.players["orphan"].name == "orphan"


def test_quest_matcher_scans_past_first_id_for_a_substring(world):
    state = GameState(world)
    alice = login(state)
    player = state.players["alice"]
    player.quests["quest.kill_guard"] = {"status": "active", "progress": 0}
    player.quests["quest.fetch_key"] = {"status": "active", "progress": 0}
    assert state.dispatch(alice, "ABANDON_QUEST fetch_key") == (
        'OK {"quest_id":"quest.fetch_key","status":"abandoned"}'
    )
    assert "quest.kill_guard" in player.quests
