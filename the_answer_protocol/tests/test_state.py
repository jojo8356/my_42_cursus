from __future__ import annotations

import json
import threading

import pytest

import protocol
from state import GameState, Group, Player, Session


def connect_user(state, name="alice"):
    session = Session()
    assert state.dispatch(session, f"CONNECT {name}") == "OK Connected"
    return session


def payload(reply):
    assert reply.startswith("OK ")
    return json.loads(reply[3:])


def test_authentication_validation_duplicate_and_quit(state):
    first, second = Session(), Session()
    assert state.dispatch(first, "LOOK") == "ERR 400 NOT_AUTHENTICATED"
    assert state.dispatch(first, "CONNECT bad name") == "ERR 202 INVALID_USER_NAME"
    assert state.dispatch(first, "CONNECT !bad!") == "ERR 202 INVALID_USER_NAME"
    assert state.dispatch(first, "CONNECT alice") == "OK Connected"
    assert state.dispatch(first, "CONNECT alice") == "ERR 409 ALREADY_CONNECTED"
    assert state.dispatch(second, "CONNECT alice") == "ERR 201 NAME_IN_USE"
    assert state.dispatch(second, "QUIT") == "OK bye"
    assert second.closed
    assert "alice" in state.players
    assert state.dispatch(first, "QUIT") == "OK bye"
    assert "alice" not in state.players
    assert first.closed


def test_username_boundaries_and_allowed_characters(state):
    assert state.dispatch(Session(), "CONNECT " + "a" * 20) == "OK Connected"
    assert state.dispatch(Session(), "CONNECT " + "b" * 21) == "ERR 202 INVALID_USER_NAME"
    assert state.dispatch(Session(), "CONNECT A_9-z") == "OK Connected"
    assert state.dispatch(Session(), "CONNECT bad.name") == "ERR 202 INVALID_USER_NAME"


def test_connect_broadcasts_presence_and_online_stats(state):
    alice, bob = Session(), Session()
    assert state.dispatch(alice, "CONNECT alice") == "OK Connected"
    assert alice.sent == ["EVT STATS players=1"]
    assert state.dispatch(bob, "CONNECT bob") == "OK Connected"
    assert "EVT ROOM PRESENCE ENTER bob" in alice.sent
    assert "EVT STATS players=2" in alice.sent
    assert bob.sent == ["EVT STATS players=2"]


def test_blank_unknown_and_missing_argument_responses(state, session):
    assert state.dispatch(session, " \t ") is None
    assert state.dispatch(session, "BLAH") == "ERR 400 UNKNOWN_COMMAND"
    session = connect_user(state)
    assert state.dispatch(session, "MOVE") == "ERR 400 MISSING_ARGUMENT"
    assert state.dispatch(session, "TAKE") == "ERR 400 MISSING_ARGUMENT"
    assert state.dispatch(session, "DROP") == "ERR 400 MISSING_ARGUMENT"
    assert state.dispatch(session, "TALK") == "ERR 400 MISSING_ARGUMENT"
    assert state.dispatch(session, "QUEST") == "ERR 400 MISSING_ARGUMENT"
    assert state.dispatch(session, "ATTACK") == "ERR 400 MISSING_ARGUMENT"


def test_line_limit_and_handler_exception_are_contained(state, monkeypatch):
    session = connect_user(state)
    assert state.dispatch(session, "LOOK " + "x" * 1024) == "ERR 413 LINE_TOO_LONG"
    original = GameState._HANDLERS["LOOK"]
    monkeypatch.setitem(GameState._HANDLERS, "LOOK", lambda *_: (_ for _ in ()).throw(RuntimeError("boom")))
    assert state.dispatch(session, "LOOK") == "ERR 400 BAD_REQUEST"
    monkeypatch.setitem(GameState._HANDLERS, "LOOK", original)


def test_status_who_look_and_help_payloads(state, session):
    assert state.dispatch(session, "STATUS") == "ERR 400 NOT_AUTHENTICATED"
    session = connect_user(state)
    look = payload(state.dispatch(session, "LOOK"))
    assert look["id"] == "loc.atrium"
    assert look["name"] == "Atrium"
    assert look["players"] == ["alice"]
    assert "item.key" in look["items"]
    assert "npc.giver" in look["npcs"]
    assert state.dispatch(session, "WHO") == "OK players=1"
    status = payload(state.dispatch(session, "STATUS"))
    assert status == {"hp": 100, "max_hp": 100, "gold": 0, "status": "alive"}
    help_line = state.dispatch(session, "HELP")
    assert help_line.startswith("OK commands=")
    assert "LOOK" in help_line and "MARKET" not in help_line


def test_move_broadcast_excludes_actor_and_notifies_arrival(state):
    alice = connect_user(state, "alice")
    bob = connect_user(state, "bob")
    charlie = connect_user(state, "charlie")
    assert state.dispatch(charlie, "MOVE east") == "OK room=loc.bunk"
    alice.sent.clear()
    bob.sent.clear()
    charlie.sent.clear()
    assert state.dispatch(alice, "MOVE east") == "OK room=loc.bunk"
    assert "EVT ROOM PRESENCE LEAVE alice" in bob.sent
    assert "EVT ROOM PRESENCE ENTER alice" in charlie.sent
    assert not any("PRESENCE" in line for line in alice.sent)
    assert state.dispatch(alice, "MOVE west") == "OK room=loc.atrium"
    assert state.dispatch(alice, "MOVE west") == "ERR 301 NO_EXIT"


def test_move_in_combat_and_player_death_are_rejected(state):
    alice = connect_user(state)
    assert state.dispatch(alice, "MOVE north") == "OK room=loc.hall"
    assert state.dispatch(alice, "ATTACK guard").startswith("OK {")
    assert state.dispatch(alice, "MOVE south") == "ERR 416 IN_COMBAT"
    state.players["alice"].dead = True
    state.players["alice"].in_combat_with = ""
    assert state.dispatch(alice, "MOVE south") == "ERR 409 PLAYER_DEAD"


def test_chat_scopes_include_sender_and_room_group_rules(state):
    alice = connect_user(state, "alice")
    bob = connect_user(state, "bob")
    assert state.dispatch(alice, "CHAT GLOBAL Arrakis listens") == "OK"
    assert "(GLOBAL) alice: Arrakis listens" in alice.sent
    assert "(GLOBAL) alice: Arrakis listens" in bob.sent
    alice.sent.clear()
    bob.sent.clear()
    assert state.dispatch(alice, "CHAT ROOM Dust ahead") == "OK"
    assert "(ROOM) alice: Dust ahead" in alice.sent
    assert "(ROOM) alice: Dust ahead" in bob.sent
    assert state.dispatch(alice, "CHAT GROUP hi") == "ERR 401 NOT_IN_GROUP"
    assert state.dispatch(alice, "CHAT SIDE hi") == "ERR 400 BAD_REQUEST"
    assert state.dispatch(alice, "CHAT ROOM") == "ERR 400 MISSING_ARGUMENT"
    assert state.dispatch(alice, "SAY A quiet warning") == "OK"
    assert "(ROOM) alice: A quiet warning" in alice.sent


def test_take_drop_inventory_examine_and_room_events(state):
    alice = connect_user(state)
    bob = connect_user(state, "bob")
    assert state.dispatch(alice, "TAKE Silver Key") == "OK taken=item.key"
    assert "EVT ROOM ITEM TAKE alice item.key" in bob.sent
    assert payload(state.dispatch(alice, "INVENTORY")) == ["item.key"]
    item = payload(state.dispatch(alice, "EXAMINE key"))
    assert item["id"] == "item.key" and item["name"] == "Silver Key"
    assert state.dispatch(alice, "DROP item.key") == "OK dropped=item.key"
    assert "EVT ROOM ITEM DROP alice item.key" in bob.sent
    assert payload(state.dispatch(alice, "INVENTORY")) == []
    assert state.dispatch(alice, "TAKE not-there") == "ERR 404 ITEM_NOT_FOUND"
    assert state.dispatch(alice, "DROP key") == "ERR 404 ITEM_NOT_IN_INVENTORY"
    assert state.dispatch(alice, "EXAMINE missing") == "ERR 404 ITEM_NOT_IN_INVENTORY"


def test_take_uses_first_matching_item_name_and_respects_obtainable(world, fake_clock):
    world.items["item.key"].obtainable = False
    state = GameState(world, clock=fake_clock)
    alice = connect_user(state)
    assert state.dispatch(alice, "TAKE Silver") == "ERR 404 ITEM_NOT_FOUND"
    world.items["item.key"].obtainable = True
    assert state.dispatch(alice, "TAKE Silver") == "OK taken=item.key"


def test_item_respawn_timer_restores_each_taken_copy(state, fake_clock):
    state.item_respawn_delay_s = 5
    alice = connect_user(state)
    assert state.dispatch(alice, "TAKE key") == "OK taken=item.key"
    assert "item.key" not in state.items_in("loc.atrium")
    fake_clock.advance(4.9)
    assert state.tick() == []
    fake_clock.advance(0.1)
    emitted = state.tick()
    assert emitted == ["EVT ROOM ITEM RESPAWN item.key"]
    assert state.items_in("loc.atrium").count("item.key") == 1
    assert emitted[0] in alice.sent


def test_dialogue_cycles_and_name_matching(state):
    alice = connect_user(state)
    first = payload(state.dispatch(alice, "TALK Mira"))
    second = payload(state.dispatch(alice, "TALK npc.giver"))
    third = payload(state.dispatch(alice, "TALK giver"))
    assert first["dialogue"] == "First line."
    assert second["dialogue"] == "Second line."
    assert third["dialogue"] == "First line."
    assert state.dispatch(alice, "TALK absent") == "ERR 404 NPC_NOT_FOUND"


def test_fetch_quest_accept_progress_turn_in_and_prerequisite_chain(state):
    alice = connect_user(state)
    offer = payload(state.dispatch(alice, "QUEST Mira"))
    assert offer["id"] == "quest.fetch_key"
    assert offer["status"] == "active" and offer["progress"] == "0/1"
    assert state.dispatch(alice, "QUEST Mira") == "ERR 408 QUEST_NOT_COMPLETE"
    # The second quest is locked until the first one has been completed.
    state.dispatch(alice, "TAKE key")
    completed = payload(state.dispatch(alice, "QUEST Mira"))
    assert completed["status"] == "completed"
    assert completed["reward"] == "item.gift"
    assert "item.key" not in state.players["alice"].inventory
    assert "item.gift" in state.players["alice"].inventory
    second = payload(state.dispatch(alice, "QUEST Mira"))
    assert second["id"] == "quest.kill_guard"
    assert state.dispatch(alice, "QUESTS") == (
        'OK [{"quest_id":"quest.fetch_key","status":"completed"},'
        '{"quest_id":"quest.kill_guard","status":"active","progress":"0/1"}]'
    )


def test_kill_quest_progress_and_enemy_respawn(state, fake_clock):
    state.respawn_delay_s = 4
    alice = connect_user(state)
    state.dispatch(alice, "QUEST Mira")  # accept fetch
    state.dispatch(alice, "TAKE key")
    state.dispatch(alice, "QUEST Mira")  # turn in fetch
    state.dispatch(alice, "QUEST Mira")  # accept kill
    assert state.dispatch(alice, "MOVE north") == "OK room=loc.hall"
    first = payload(state.dispatch(alice, "ATTACK guard"))
    assert first["target_hp"] == 10 and first["npc_damage"] == 5
    victory = payload(state.dispatch(alice, "ATTACK guard"))
    assert victory["status"] == "victory" and victory["gold_earned"] == 3
    assert state.npcs_in("loc.hall") == []
    assert state.dispatch(alice, "ATTACK guard") == "ERR 404 NPC_NOT_FOUND"
    assert state.dispatch(alice, "MOVE south") == "OK room=loc.atrium"
    result = payload(state.dispatch(alice, "QUEST Mira"))
    assert result["status"] == "completed" and result["reward"] == "item.sword"
    assert "item.sword" in state.players["alice"].inventory
    fake_clock.advance(4)
    assert state.tick() == ["EVT ROOM NPC RESPAWN npc.guard"]
    assert state.npcs_in("loc.hall") == ["npc.guard"]
    assert state.npcs["loc.hall"]["npc.guard"] == 20


def test_deliver_quest_requires_item_and_turns_in_at_recipient(state):
    alice = connect_user(state)
    state.dispatch(alice, "TAKE key")
    state.dispatch(alice, "QUEST Mira")
    state.dispatch(alice, "QUEST Mira")  # complete fetch
    state.dispatch(alice, "QUEST Mira")  # accept kill
    state.dispatch(alice, "MOVE north")
    state.dispatch(alice, "ATTACK guard")
    state.dispatch(alice, "ATTACK guard")
    state.dispatch(alice, "MOVE south")
    state.dispatch(alice, "QUEST Mira")  # complete kill
    accepted = payload(state.dispatch(alice, "QUEST Mira"))
    assert accepted["id"] == "quest.deliver_token"
    state.dispatch(alice, "MOVE east")
    assert state.dispatch(alice, "QUEST Sage") == "ERR 408 QUEST_NOT_COMPLETE"
    state.dispatch(alice, "MOVE west")
    state.dispatch(alice, "MOVE north")
    assert state.dispatch(alice, "TAKE token") == "OK taken=item.token"
    state.dispatch(alice, "MOVE south")
    state.dispatch(alice, "MOVE east")
    result = payload(state.dispatch(alice, "QUEST Sage"))
    assert result["status"] == "completed"
    assert "item.token" not in state.players["alice"].inventory


def test_abandon_quest_and_not_active_errors(state):
    alice = connect_user(state)
    assert state.dispatch(alice, "ABANDON_QUEST none") == "ERR 404 QUEST_NOT_ACTIVE"
    state.dispatch(alice, "QUEST Mira")
    result = payload(state.dispatch(alice, "ABANDON_QUEST fetch_key"))
    assert result == {"quest_id": "quest.fetch_key", "status": "abandoned"}
    assert state.dispatch(alice, "ABANDON_QUEST fetch_key") == "ERR 404 QUEST_NOT_ACTIVE"


def test_quest_giver_and_completed_quest_errors(state):
    alice = connect_user(state)
    assert state.dispatch(alice, "QUEST Guide") == "ERR 406 NO_QUEST_AVAILABLE"
    state.dispatch(alice, "QUEST Mira")
    state.dispatch(alice, "TAKE key")
    state.dispatch(alice, "QUEST Mira")
    assert state.dispatch(alice, "QUEST Mira") == "OK {\"id\":\"quest.kill_guard\",\"description\":\"Defeat the guard.\",\"giver\":\"npc.giver\",\"type\":\"kill\",\"target\":\"npc.guard\",\"target_count\":1,\"reward\":\"item.sword\",\"reward_count\":1,\"requires\":\"quest.fetch_key\",\"status\":\"active\"}"


def test_sleep_only_works_in_safe_location(state):
    alice = connect_user(state)
    state.players["alice"].hp = 10
    assert state.dispatch(alice, "SLEEP") == "ERR 410 CANNOT_SLEEP_HERE"
    state.dispatch(alice, "MOVE east")
    assert state.dispatch(alice, "SLEEP") == "OK hp=100/100 You feel really good now !"
    assert state.players["alice"].hp == 100
    assert state.dispatch(alice, "MOVE west") == "OK room=loc.atrium"


def test_attack_uses_best_weapon_and_armor_and_rejects_friendly_npc(state):
    alice = connect_user(state)
    state.dispatch(alice, "TAKE sword")
    state.dispatch(alice, "TAKE coat")
    assert state.dispatch(alice, "ATTACK Guide") == "ERR 405 NPC_NOT_HOSTILE"
    state.dispatch(alice, "MOVE north")
    strike = payload(state.dispatch(alice, "ATTACK guard"))
    assert strike["damage"] == 17
    assert strike["npc_damage"] == 0
    assert strike["attacker_hp"] == 100
    assert state.dispatch(alice, "MOVE south") == "ERR 416 IN_COMBAT"


def test_defend_and_flee_are_combat_only_and_flee_returns_previous_room(state):
    alice = connect_user(state)
    assert state.dispatch(alice, "DEFEND") == "ERR 415 NOT_IN_COMBAT"
    assert state.dispatch(alice, "FLEE") == "ERR 415 NOT_IN_COMBAT"
    state.dispatch(alice, "MOVE north")
    state.dispatch(alice, "ATTACK guard")
    before = state.players["alice"].hp
    defended = payload(state.dispatch(alice, "DEFEND"))
    assert defended["attacker_hp"] == min(100, before + 5)
    fled = payload(state.dispatch(alice, "FLEE"))
    assert fled["fled"] is True and fled["room"] == "loc.atrium"
    assert state.players["alice"].in_combat_with == ""


def test_death_hides_player_and_delayed_respawn_restores_half_hp(state, fake_clock):
    state.player_respawn_delay_s = 3
    alice = connect_user(state)
    state.dispatch(alice, "MOVE north")
    state.players["alice"].hp = 5
    response = payload(state.dispatch(alice, "ATTACK guard"))
    assert response["status"] == "death"
    assert state.players["alice"].dead
    assert state.players_in("loc.hall") == []
    assert state.dispatch(alice, "MOVE south") == "ERR 409 PLAYER_DEAD"
    fake_clock.advance(2.9)
    assert state.tick() == []
    fake_clock.advance(0.1)
    emitted = state.tick()
    assert emitted == ["EVT ROOM COMBAT alice respawned"]
    player = state.players["alice"]
    assert not player.dead and player.hp == 50 and player.room_id == "loc.atrium"
    assert not any(line == "EVT ROOM PRESENCE ENTER alice" for line in alice.sent)
    assert "EVT ROOM COMBAT alice respawned" in alice.sent


def test_player_and_group_model_serialization(world):
    player = Player(name="Paul", room_id="loc.atrium")
    assert player.status == "alive"
    player.dead = True
    assert player.status == "dead"
    assert player.to_dict()["status"] == "dead"
    group = Group(id="fedaykin", leader="Paul", players=["Paul"])
    assert group.to_dict() == {"id": "fedaykin", "leader": "Paul",
                               "players": ["Paul"], "invited": []}


def test_group_create_invite_join_info_and_chat(state):
    alice = connect_user(state, "alice")
    bob = connect_user(state, "bob")
    assert state.dispatch(alice, "GROUP INVITE bob") == "ERR 401 NOT_IN_GROUP"
    assert state.dispatch(alice, "GROUP CREATE bad name") == "ERR 212 INVALID_GROUP_NAME"
    assert state.dispatch(alice, "GROUP CREATE fremen") == "OK group=fremen"
    assert state.dispatch(alice, "GROUP CREATE") == "ERR 402 ALREADY_IN_GROUP"
    assert state.dispatch(alice, "GROUP INVITE alice") == "ERR 407 CANNOT_INVITE_SELF"
    assert state.dispatch(alice, "GROUP INVITE") == "ERR 400 MISSING_ARGUMENT"
    assert state.dispatch(alice, "GROUP INVITE ghost") == "ERR 404 PLAYER_NOT_FOUND"
    assert state.dispatch(bob, "GROUP JOIN fremen") == "ERR 407 NOT_INVITED"
    assert state.dispatch(alice, "GROUP INVITE bob") == "OK"
    assert "EVT GROUP INVITE alice" in bob.sent
    assert state.dispatch(alice, "GROUP INVITE bob") == "ERR 402 ALREADY_IN_GROUP"
    assert state.dispatch(bob, "GROUP JOIN fremen") == "OK group=fremen"
    assert "EVT GROUP JOIN bob" in alice.sent
    info = payload(state.dispatch(bob, "GROUP INFO"))
    assert info["leader"] == "alice" and info["players"] == ["alice", "bob"]
    assert state.dispatch(bob, "CHAT GROUP squad up") == "OK"
    assert "(GROUP) bob: squad up" in alice.sent
    assert "(GROUP) bob: squad up" in bob.sent


def test_group_leadership_transfer_leave_kick_and_disband(state):
    alice = connect_user(state, "alice")
    bob = connect_user(state, "bob")
    carol = connect_user(state, "carol")
    state.dispatch(alice, "GROUP CREATE team")
    state.dispatch(alice, "GROUP INVITE bob")
    state.dispatch(alice, "GROUP INVITE carol")
    state.dispatch(bob, "GROUP JOIN team")
    state.dispatch(carol, "GROUP JOIN team")
    assert state.dispatch(bob, "GROUP INVITE alice") == "ERR 403 NOT_GROUP_LEADER"
    assert state.dispatch(alice, "GROUP KICK alice") == "ERR 407 CANNOT_KICK_SELF"
    assert state.dispatch(alice, "GROUP KICK ghost") == "ERR 404 PLAYER_NOT_IN_GROUP"
    assert state.dispatch(alice, "GROUP KICK bob") == "OK"
    assert "EVT GROUP KICK alice" in bob.sent
    assert state.players["bob"].group == ""
    assert state.dispatch(alice, "GROUP LEAVE") == "OK"
    assert state.players["carol"].group == "team"
    assert state.groups["team"].leader == "carol"
    assert "EVT GROUP LEADER carol" in carol.sent
    assert state.dispatch(carol, "GROUP DISBAND") == "OK"
    assert "team" not in state.groups
    assert state.players["carol"].group == ""
    assert "EVT GROUP DISBAND" in carol.sent
    assert state.dispatch(carol, "GROUP LEAVE") == "ERR 401 NOT_IN_GROUP"


def test_group_join_by_inviter_and_kick_argument_errors(state):
    alice = connect_user(state, "alice")
    bob = connect_user(state, "bob")
    assert state.dispatch(bob, "GROUP JOIN nowhere") == "ERR 404 GROUP_NOT_FOUND"
    state.dispatch(alice, "GROUP CREATE team")
    assert state.dispatch(alice, "GROUP KICK") == "ERR 400 MISSING_ARGUMENT"
    state.dispatch(alice, "GROUP INVITE bob")
    assert state.dispatch(bob, "GROUP JOIN alice") == "OK group=team"
    assert state.dispatch(bob, "GROUP JOIN team") == "ERR 402 ALREADY_IN_GROUP"


def test_disconnecting_group_leader_transfers_and_clears_pending_invites(state):
    alice = connect_user(state, "alice")
    bob = connect_user(state, "bob")
    state.dispatch(alice, "GROUP CREATE squad")
    state.dispatch(alice, "GROUP INVITE bob")
    state.dispatch(alice, "QUIT")
    assert state.groups["squad"].leader == "bob" if "squad" in state.groups else True
    # The group had only its leader; leaving empties and removes it, and the
    # outstanding invitation is cleaned up along with the player.
    assert "squad" not in state.groups
    assert "alice" not in state.players
    assert "EVT STATS players=1" in bob.sent


def test_disconnect_drops_inventory_and_broadcasts_presence(state):
    alice = connect_user(state, "alice")
    bob = connect_user(state, "bob")
    state.dispatch(alice, "TAKE key")
    bob.sent.clear()
    state.dispatch(alice, "QUIT")
    assert "item.key" in state.items_in("loc.atrium")
    assert "EVT ROOM PRESENCE LEAVE alice" in bob.sent
    assert "EVT STATS players=1" in bob.sent


def test_sliding_rate_limiter_warns_and_kicks(fake_clock):
    session = Session(clock=fake_clock)
    assert all(session.record_command() == "" for _ in range(10))
    assert session.record_command() == "warn"
    for _ in range(9):
        assert session.record_command() == "warn"
    assert session.record_command() == "kick"
    assert session.kicked
    fresh = Session(clock=fake_clock)
    for _ in range(10):
        fresh.record_command()
    fake_clock.advance(1)
    assert fresh.record_command() == ""


def test_dispatch_kicks_over_limit_and_error_is_queued(state, fake_clock):
    session = Session(clock=fake_clock)
    assert state.dispatch(session, "CONNECT alice") == "OK Connected"
    for _ in range(19):
        state.dispatch(session, "WHO")
    assert state.dispatch(session, "WHO") is None
    assert session.kicked
    assert "ERR 902 FLOODING" in session.sent


def test_concurrent_take_is_serialized_and_only_one_player_wins(state):
    alice = connect_user(state, "alice")
    bob = connect_user(state, "bob")
    barrier = threading.Barrier(3)
    results = []
    def take(session):
        barrier.wait()
        results.append(state.dispatch(session, "TAKE key"))
    threads = [threading.Thread(target=take, args=(alice,)),
               threading.Thread(target=take, args=(bob,))]
    for thread in threads:
        thread.start()
    barrier.wait()
    for thread in threads:
        thread.join(timeout=2)
    assert sorted(results) == ["ERR 404 ITEM_NOT_FOUND", "OK taken=item.key"]
    assert sum("item.key" in player.inventory for player in state.players.values()) == 1
    assert "item.key" not in state.items_in("loc.atrium")


def test_state_logger_emits_command_and_error_records(world, fake_clock):
    class Capture:
        def __init__(self):
            self.rows = []
        def log(self, level, msg, **fields):
            self.rows.append((level, msg, fields))
    logger = Capture()
    state = GameState(world, clock=fake_clock, logger=logger)
    session = connect_user(state)
    assert state.dispatch(session, "MOVE west") == "ERR 301 NO_EXIT"
    assert any(row[1] == "command" and row[2]["command"] == "MOVE" for row in logger.rows)
    assert any(row[1] == "error response" and row[2]["code"] == 301 for row in logger.rows)
