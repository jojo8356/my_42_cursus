from __future__ import annotations

import json

import pytest

import protocol


def test_error_table_has_unique_symbols_and_valid_codes():
    assert protocol.ERRORS
    assert len(protocol.ERRORS) == len(set(protocol.ERRORS))
    for symbol, spec in protocol.ERRORS.items():
        assert spec.symbol == symbol
        assert 100 <= spec.code <= 999
        assert spec.message
        assert protocol.err(symbol) == f"ERR {spec.code} {symbol}"


def test_normative_and_extension_error_codes():
    expected = {
        "NAME_IN_USE": 201, "NO_EXIT": 301, "NOT_IN_GROUP": 401,
        "ALREADY_IN_GROUP": 402, "ITEM_NOT_FOUND": 404, "NPC_NOT_FOUND": 404,
        "PLAYER_NOT_FOUND": 404, "GROUP_NOT_FOUND": 404,
        "NPC_NOT_HOSTILE": 405, "NO_QUEST_AVAILABLE": 406,
        "CONNECTION_FAILED": 900, "SEND_FAILED": 901, "FLOODING": 902,
        "INVALID_USER_NAME": 202, "NOT_GROUP_LEADER": 403,
        "QUEST_NOT_COMPLETE": 408, "PLAYER_DEAD": 409,
        "CANNOT_SLEEP_HERE": 410, "LINE_TOO_LONG": 413,
        "NOT_IN_COMBAT": 415, "IN_COMBAT": 416,
    }
    assert {name: protocol.ERRORS[name].code for name in expected} == expected


def test_fatal_split_is_code_based():
    assert protocol.ERRORS["CONNECTION_FAILED"].fatal
    assert protocol.ERRORS["SEND_FAILED"].fatal
    assert protocol.ERRORS["FLOODING"].fatal
    assert not protocol.ERRORS["NAME_IN_USE"].fatal
    assert not protocol.ERRORS["LINE_TOO_LONG"].fatal


def test_error_lookup_and_protocol_error_attributes():
    spec = protocol.error_spec("NO_EXIT")
    assert spec.code == 301
    assert protocol.error_line("NO_EXIT") == "ERR 301 NO_EXIT"
    error = protocol.ProtocolError("NO_EXIT")
    assert str(error) == "ERR 301 NO_EXIT"
    assert (error.code, error.symbol, error.message) == (301, "NO_EXIT", "no exit in that direction")
    with pytest.raises(KeyError, match="unknown error symbol"):
        protocol.error_spec("NOT_A_REAL_ERROR")


def test_specialized_protocol_errors_inherit_protocol_error():
    assert issubclass(protocol.Unauthorized, protocol.ProtocolError)
    assert issubclass(protocol.UnknownCommand, protocol.ProtocolError)


@pytest.mark.parametrize("line,expected", [
    ("LOOK", ("LOOK", "", "")),
    ("  lOoK   ", ("LOOK", "", "")),
    ("MOVE north", ("MOVE", "", "north")),
    ("chat global Hello, Arrakis", ("CHAT", "GLOBAL", "Hello, Arrakis")),
    ("CHAT ROOM multiple words keep case", ("CHAT", "ROOM", "multiple words keep case")),
    ("CHAT GROUP Welcome home", ("CHAT", "GROUP", "Welcome home")),
    ("GROUP CREATE fedaykin", ("GROUP", "CREATE", "fedaykin")),
    ("GROUP INVITE Paul Atreides", ("GROUP", "INVITE", "Paul Atreides")),
    ("GROUP JOIN fedaykin", ("GROUP", "JOIN", "fedaykin")),
    ("GROUP LEAVE", ("GROUP", "LEAVE", "")),
    ("GROUP KICK Chani", ("GROUP", "KICK", "Chani")),
    ("GROUP DISBAND", ("GROUP", "DISBAND", "")),
    ("GROUP INFO", ("GROUP", "INFO", "")),
    ("ABANDON_QUEST quest.one", ("ABANDON_QUEST", "", "quest.one")),
    ("SAY Sardaukar on the ridge", ("SAY", "", "Sardaukar on the ridge")),
    ("CONNECT user_42", ("CONNECT", "", "user_42")),
])
def test_parse_command_fields(line, expected):
    command = protocol.parse_command(line)
    assert (command.verb, command.scope, command.arg) == expected
    assert command.raw == line


def test_parse_command_name_and_dictionary():
    command = protocol.parse_command("group invite paul")
    assert command.name == "GROUP INVITE"
    assert command.to_dict() == {"verb": "GROUP", "scope": "INVITE", "sub": "", "arg": "paul"}
    assert protocol.parse_command("look").name == "LOOK"


def test_parser_accepts_tabs_between_tokens():
    command = protocol.parse_command("CHAT\tGLOBAL\thello there")
    assert (command.verb, command.scope, command.arg) == ("CHAT", "GLOBAL", "hello there")


def test_parser_knows_each_supported_verb():
    for verb in protocol.KNOWN_VERBS:
        assert protocol.parse_command(verb).verb == verb
    assert "MARKET" not in protocol.KNOWN_VERBS
    assert "SHOP" not in protocol.KNOWN_VERBS


def test_parse_unknown_command_uses_unknown_symbol():
    with pytest.raises(protocol.UnknownCommand) as caught:
        protocol.parse_command("FLY north")
    assert caught.value.symbol == "UNKNOWN_COMMAND"
    assert caught.value.code == 400


def test_parse_command_over_limit_is_line_too_long():
    with pytest.raises(protocol.ProtocolError) as caught:
        protocol.parse_command("X" * (protocol.MAX_LINE_BYTES + 1))
    assert caught.value.symbol == "LINE_TOO_LONG"
    assert protocol.parse_command("LOOK " + "x" * 1018).verb == "LOOK"


def test_blank_command_is_returned_for_dispatcher_to_silence():
    for line in ("", " \t ", "\r\n"):
        command = protocol.parse_command(line)
        assert command.verb == ""


def test_trim_line_removes_lf_and_one_cr():
    assert protocol.trim_line("LOOK\r\n") == "LOOK"
    assert protocol.trim_line("LOOK\r") == "LOOK"
    assert protocol.trim_line("LOOK\r\r") == "LOOK\r"
    assert protocol.trim_line("LOOK") == "LOOK"


def test_reply_builders():
    assert protocol.ok() == "OK"
    assert protocol.ok("  bye  ") == "OK bye"
    assert protocol.ok_json(None) == "OK []"
    assert protocol.ok_json([]) == "OK []"
    assert protocol.ok_json({"name": "Chani", "message": "spice flows"}) == (
        'OK {"name":"Chani","message":"spice flows"}'
    )
    assert "\n" not in protocol.ok_json({"line": "a\nb"})


def test_compact_json_preserves_unicode():
    encoded = protocol.compact_json({"dune": "Arrakis — désert"})
    assert "—" in encoded
    assert ": " not in encoded and ", " not in encoded
    assert json.loads(encoded) == {"dune": "Arrakis — désert"}


def test_room_payload_flattening_and_optional_sleep_flag():
    room = {"id": "loc.a", "name": "A", "description": "Dust", "exits": {"east": "loc.b"}}
    payload = protocol.room_payload(room, ["Paul"], ["item.key"], ["npc.guard"])
    assert payload == {"id": "loc.a", "name": "A", "description": "Dust",
                       "exits": {"east": "loc.b"}, "players": ["Paul"],
                       "items": ["item.key"], "npcs": ["npc.guard"]}
    assert "can_sleep" not in payload
    room["can_sleep"] = True
    assert protocol.room_payload(room, [], [], [])["can_sleep"] is True


def test_item_and_status_payload_shapes():
    assert protocol.item_payload({"id": "item.sword", "name": "Sword", "damage": 4}) == {
        "id": "item.sword", "name": "Sword", "description": "", "damage": 4,
        "armor": None, "heal": None, "value": 0
    }
    assert protocol.status_payload(4, 10, "alive", 8) == {
        "hp": 4, "max_hp": 10, "gold": 8, "status": "alive"
    }


def test_event_builders_have_exact_wire_shapes():
    assert protocol.event("room", "presence", "ENTER", "Paul") == "EVT ROOM PRESENCE ENTER Paul"
    assert protocol.room_presence("LEAVE", "Paul") == "EVT ROOM PRESENCE LEAVE Paul"
    assert protocol.room_item("TAKE", "Paul", "item.key") == "EVT ROOM ITEM TAKE Paul item.key"
    assert protocol.room_combat("Paul attacks Guard") == "EVT ROOM COMBAT Paul attacks Guard"
    assert protocol.group_event("INVITE", "Paul") == "EVT GROUP INVITE Paul"
    assert protocol.stats_event(12) == "EVT STATS players=12"
    assert protocol.chat("global", "Paul", "Hello all") == "(GLOBAL) Paul: Hello all"


@pytest.mark.parametrize("line,kind", [
    ("OK Connected", "ok"),
    ("ERR 301 NO_EXIT", "err"),
    ("EVT ROOM PRESENCE ENTER Paul", "evt"),
    ("(ROOM) Paul: Hello", "chat"),
    ("raw server line", "raw"),
])
def test_parse_message_kinds(line, kind):
    assert protocol.parse_message(line).kind == kind


def test_parse_message_fields_for_each_shape():
    ok = protocol.parse_message("OK  {\"hp\":10}")
    assert ok.payload == '{"hp":10}'
    error = protocol.parse_message("ERR 404 NPC_NOT_FOUND")
    assert (error.code, error.symbol, error.is_error) == (404, "NPC_NOT_FOUND", True)
    event = protocol.parse_message("EVT GROUP INVITE Paul Atreides")
    assert (event.scope, event.event_type, event.data) == ("GROUP", "INVITE", ["Paul", "Atreides"])
    chat = protocol.parse_message("(GLOBAL) Chani: Spice must flow.")
    assert chat.scope == "GLOBAL"
    assert chat.payload == "Chani: Spice must flow."
    raw = protocol.parse_message("not a TAP message")
    assert raw.payload == raw.raw == "not a TAP message"


def test_parse_message_handles_malformed_error_and_unclosed_chat():
    assert protocol.parse_message("ERR nope missing").code is None
    assert protocol.parse_message("ERR 404").symbol == ""
    assert protocol.parse_message("(").scope == ""


def test_split_json_returns_data_or_none():
    assert protocol.split_json('{"a":1}') == {"a": 1}
    assert protocol.split_json("[]") == []
    assert protocol.split_json("") is None
    assert protocol.split_json("not json") is None


def test_frame_is_utf8_and_lf_terminated():
    assert protocol.frame("LOOK") == b"LOOK\n"
    assert protocol.frame("CHAT GLOBAL épice") == "CHAT GLOBAL épice\n".encode("utf-8")


def test_parse_events_validates_message_kind():
    assert protocol.parse_events("EVT ROOM NPC RESPAWN npc.guard") == (
        "ROOM", "NPC", ["RESPAWN", "npc.guard"]
    )
    with pytest.raises(protocol.ProtocolError) as caught:
        protocol.parse_events("OK bye")
    assert caught.value.symbol == "BAD_REQUEST"
