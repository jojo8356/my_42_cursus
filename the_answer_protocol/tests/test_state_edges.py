from __future__ import annotations

import json

import protocol
from state import GameState, Group, Player, Session
from world import World, default_world_path


def login(state, name):
    session = Session()
    assert state.dispatch(session, f"CONNECT {name}") == "OK Connected"
    return session


def test_empty_connect_and_missing_scopes_are_precise(world):
    state = GameState(world)
    fresh = Session()
    assert state.dispatch(fresh, "CONNECT") == "ERR 400 MISSING_ARGUMENT"
    assert state.dispatch(fresh, "GROUP") == "ERR 400 NOT_AUTHENTICATED"
    # Authentication gates all commands except CONNECT and QUIT.
    assert state.dispatch(fresh, "CHAT") == "ERR 400 NOT_AUTHENTICATED"
    logged = login(state, "alice")
    assert state.dispatch(logged, "GROUP") == "ERR 400 MISSING_ARGUMENT"
    assert state.dispatch(logged, "CHAT") == "ERR 400 MISSING_ARGUMENT"
    assert "SAY" in state.dispatch(logged, "HELP")


def test_session_send_callback_failure_and_closed_session_are_safe():
    def broken(_message):
        raise OSError("gone")
    session = Session(send=broken)
    session.send("one")
    assert session.sent == ["one"] and session.closed
    session.send("two")
    assert session.sent == ["one"]


def test_player_inventory_helpers_and_weapon_armor_defaults(world):
    player = Player("p", "loc.atrium")
    assert not player.has("item.sword")
    assert player.count("item.sword") == 0
    assert player.weapon_bonus(world.items) == 0
    assert player.armor_bonus(world.items) == 0
    player.inventory.extend(["item.sword", "item.sword", "unknown"])
    assert player.has("item.sword") and player.count("item.sword") == 2
    assert player.weapon_bonus(world.items) == 7


def test_player_and_room_lookup_error_and_empty_room_collections(world):
    state = GameState(world)
    try:
        state.player("missing")
    except protocol.ProtocolError as exc:
        assert exc.symbol == "PLAYER_NOT_FOUND"
    else:
        raise AssertionError("missing player must fail")
    assert state.npcs_in("loc.missing") == []
    assert state.items_in("loc.missing") == []
    assert state.disconnect("missing") == []


def test_broadcast_skip_and_stale_session_paths(world):
    state = GameState(world)
    alice, bob = login(state, "alice"), login(state, "bob")
    bob.closed = True
    state.broadcast_room("loc.atrium", "room event", exclude=["alice"])
    assert "room event" not in alice.sent
    assert "room event" not in bob.sent
    state.sessions.pop("bob")
    state.broadcast_all("global event")
    assert "global event" not in bob.sent
    state.groups["stale"] = Group("stale", "ghost", ["ghost"])
    state.broadcast_group("stale", "group event")
    state.broadcast_group("absent", "ignored")
    assert state.disconnect("bob") == [
        ("room:loc.atrium", "EVT ROOM PRESENCE LEAVE bob")
    ]
    assert "alice" in state.players


def test_empty_dialogue_npc_has_safe_fallback(world):
    world.npcs["npc.guide"].dialogue.clear()
    state = GameState(world)
    alice = login(state, "alice")
    payload = json.loads(state.dispatch(alice, "TALK Guide")[3:])
    assert payload == {"npc": "Guide", "dialogue": "Guide says nothing."}


def test_default_world_quest_chain_unlocks_fremen_trust():
    world = World.load(default_world_path())
    state = GameState(world)
    alice = login(state, "Paul")
    assert state.dispatch(alice, "MOVE east") == "OK room=loc.market"
    assert "quest.spice_harvest" in state.dispatch(alice, "QUEST Chani")
    state.dispatch(alice, "TAKE melange")
    state.dispatch(alice, "TAKE melange")
    state.dispatch(alice, "QUEST Chani")
    state.dispatch(alice, "MOVE south")
    state.dispatch(alice, "MOVE south")
    state.dispatch(alice, "MOVE east")
    assert state.dispatch(alice, "QUEST Elder").startswith("OK {")
    state.dispatch(alice, "TAKE token")
    completed = json.loads(state.dispatch(alice, "QUEST Elder")[3:])
    assert completed["status"] == "completed"
    assert completed["reward"] == "item.maker_hooks"


def test_quest_progress_and_abandonment_without_inventory_loss(world):
    state = GameState(world)
    alice = login(state, "alice")
    state.dispatch(alice, "QUEST Mira")
    state.dispatch(alice, "TAKE key")
    assert state.dispatch(alice, "QUESTS").find("1/1") >= 0
    state.dispatch(alice, "DROP key")
    assert state.dispatch(alice, "QUESTS").find("0/1") >= 0
    state.dispatch(alice, "ABANDON_QUEST fetch_key")
    assert "item.key" not in state.players["alice"].inventory


def test_group_default_name_duplicate_name_and_cross_group_invite(world):
    state = GameState(world)
    alice, bob, carol = login(state, "alice"), login(state, "bob"), login(state, "carol")
    assert state.dispatch(alice, "GROUP CREATE") == "OK group=alice"
    assert state.dispatch(bob, "GROUP CREATE team") == "OK group=team"
    assert state.dispatch(carol, "GROUP CREATE team") == "ERR 211 GROUP_NAME_IN_USE"
    assert state.dispatch(alice, "GROUP INVITE bob") == "ERR 402 ALREADY_IN_GROUP"
    assert state.dispatch(alice, "GROUP INVITE carol") == "OK"
    state.disconnect("carol")
    assert "carol" not in state.groups["alice"].invited


def test_kicking_stale_member_and_leaving_unknown_group(world):
    state = GameState(world)
    alice, bob = login(state, "alice"), login(state, "bob")
    state.dispatch(alice, "GROUP CREATE squad")
    state.dispatch(alice, "GROUP INVITE bob")
    state.dispatch(bob, "GROUP JOIN squad")
    state.sessions.pop("bob")
    state.players.pop("bob")
    assert state.dispatch(alice, "GROUP KICK bob") == "OK"
    assert state.groups["squad"].players == ["alice"]
    stale = Player("ghost", "loc.atrium", group="not-a-group")
    assert state._leave_group(stale) == []
    assert stale.group == ""


def test_due_respawn_is_discarded_if_player_disconnected(world, fake_clock):
    state = GameState(world, clock=fake_clock)
    state.player_respawn_delay_s = 1
    alice = login(state, "alice")
    state.dispatch(alice, "MOVE north")
    state.players["alice"].hp = 5
    state.dispatch(alice, "ATTACK guard")
    assert state.players["alice"].dead
    state.disconnect("alice")
    fake_clock.advance(2)
    assert state.tick() == []


def test_explicit_respawn_is_idempotently_stateful(world):
    state = GameState(world)
    alice = login(state, "alice")
    player = state.players["alice"]
    player.dead = True
    player.hp = 0
    player.room_id = "loc.hall"
    lines = state.respawn(player)
    assert lines == ["EVT ROOM PRESENCE ENTER alice"]
    assert player.room_id == "loc.atrium" and player.hp == 50 and not player.dead
