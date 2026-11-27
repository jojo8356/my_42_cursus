from __future__ import annotations

import json
import socket
import threading
import time

from client_cli import CliClient, _is_event, format_message, run_client
from server import TapServer


def test_cli_connects_and_passes_typed_command_without_translation(world):
    server = TapServer(world, host="127.0.0.1", port=0)
    port = server.start()
    output = []
    client = CliClient("127.0.0.1", port, writer=output.append)
    try:
        assert client.connect() == "OK hello proto=1"
        assert client.send_and_collect("connect Raw_User") == "OK Connected"
        response = client.send_and_collect("wHo")
        assert response == "OK players=1"
        assert client.send_and_collect("look") .startswith("OK {")
        assert any(line == "EVT STATS players=1" for line in client.received)
        assert client.connected
    finally:
        client.close()
        server.stop()


def test_cli_events_are_received_without_waiting_for_a_command(world):
    server = TapServer(world, host="127.0.0.1", port=0)
    port = server.start()
    alice_lines, bob_lines = [], []
    alice = CliClient("127.0.0.1", port, writer=alice_lines.append)
    bob = CliClient("127.0.0.1", port, writer=bob_lines.append)
    try:
        alice.connect()
        bob.connect()
        assert alice.send_and_collect("CONNECT alice") == "OK Connected"
        assert bob.send_and_collect("CONNECT bob") == "OK Connected"
        alice._reader = threading.Thread(target=alice._reader_loop, daemon=True)
        alice._reader.start()
        assert bob.send_and_collect("CHAT GLOBAL sandstorm") == "OK"
        deadline = time.monotonic() + 1
        while time.monotonic() < deadline and "(GLOBAL) bob: sandstorm" not in alice_lines:
            time.sleep(0.01)
        assert "(GLOBAL) bob: sandstorm" in alice_lines
    finally:
        alice.close()
        bob.close()
        server.stop()


def test_cli_run_uses_prompt_reader_and_quit_disconnects(world):
    server = TapServer(world, host="127.0.0.1", port=0)
    port = server.start()
    client = CliClient("127.0.0.1", port, writer=lambda _: None)
    client.connect()
    commands = iter(["CONNECT runner", "WHO", "QUIT"])
    prompts = []
    def readline(prompt):
        prompts.append(prompt)
        return next(commands)
    client.run(readline=readline)
    deadline = time.monotonic() + 1
    while time.monotonic() < deadline and "runner" in server.state.players:
        time.sleep(0.01)
    try:
        assert prompts == ["tap> ", "tap> ", "tap> "]
        assert "runner" not in server.state.players
    finally:
        server.stop()


def test_cli_formatting_and_event_detection():
    assert _is_event("EVT ROOM PRESENCE ENTER Paul")
    assert _is_event("(GLOBAL) Paul: Hello")
    assert not _is_event("OK Connected")
    assert format_message("ERR 301 NO_EXIT") == "[error 301] no exit"
    assert format_message("EVT ROOM ITEM TAKE Paul item.key") == "[room] TAKE Paul item.key"
    assert format_message("(ROOM) Paul: Hello") == "[room] Paul: Hello"
    assert format_message("OK hi") == "hi"


def test_cli_reports_unreachable_server(capsys):
    code = run_client("127.0.0.1", 1)
    captured = capsys.readouterr()
    assert code == 1
    assert "cannot reach" in captured.err


def test_client_reader_tolerates_eof_and_close(world):
    server = TapServer(world, host="127.0.0.1", port=0)
    port = server.start()
    client = CliClient("127.0.0.1", port)
    client.connect()
    client.close()
    assert client.read_one(0.01) == ""
    assert not client.connected
    server.stop()
