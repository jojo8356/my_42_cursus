from __future__ import annotations

import protocol


def test_message_dictionary_preserves_all_fields():
    message = protocol.parse_message("EVT ROOM PRESENCE ENTER Paul")
    assert message.to_dict() == {
        "kind": "evt", "payload": "ENTER Paul", "code": None, "symbol": "",
        "scope": "ROOM", "event_type": "PRESENCE",
        "data": ["ENTER", "Paul"], "raw": "EVT ROOM PRESENCE ENTER Paul",
    }
    message.data.append("copy")
    assert message.to_dict()["data"] == ["ENTER", "Paul", "copy"]


def test_message_parser_distinguishes_exact_ok_prefix():
    assert protocol.parse_message("OK").payload == ""
    assert protocol.parse_message("OK Connected").payload == "Connected"
    assert protocol.parse_message("OKAY").kind == "raw"


def test_message_parser_handles_empty_event_and_chat_scope():
    event = protocol.parse_message("EVT")
    assert event.kind == "raw"
    malformed_chat = protocol.parse_message("(ROOM message without close")
    assert malformed_chat.kind == "chat"


def test_empty_room_payload_exits_falls_back_to_empty_object():
    payload = protocol.room_payload({"id": "loc.none", "name": "Void",
                                     "description": "", "exits": None}, [], [], [])
    assert payload["exits"] == {}
