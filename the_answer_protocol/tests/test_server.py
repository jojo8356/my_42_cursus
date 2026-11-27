from __future__ import annotations

import io
import json
import socket
import time

import pytest

from logging_json import JsonLogger
from server import MAX_CONNECTIONS, RAPID_CONNECTIONS, RAPID_WINDOW_S, TapServer


class Wire:
    def __init__(self, port, timeout=2.0):
        self.sock = socket.create_connection(("127.0.0.1", port), timeout=timeout)
        self.sock.settimeout(0.1)
        self.buffer = bytearray()
        self.events = []
        self.greeting = self.read_line(timeout)

    def read_line(self, timeout=2.0):
        deadline = time.monotonic() + timeout
        while time.monotonic() < deadline:
            newline = self.buffer.find(b"\n")
            if newline >= 0:
                raw = bytes(self.buffer[:newline])
                del self.buffer[:newline + 1]
                return raw.decode("utf-8", "replace").rstrip("\r")
            self.sock.settimeout(max(0.01, min(0.1, deadline - time.monotonic())))
            try:
                chunk = self.sock.recv(4096)
            except socket.timeout:
                continue
            except OSError:
                return ""
            if not chunk:
                return ""
            self.buffer.extend(chunk)
        return ""

    def send(self, data):
        self.sock.sendall(data if isinstance(data, bytes) else (data + "\n").encode())

    def exchange(self, command, timeout=2.0):
        self.send(command)
        deadline = time.monotonic() + timeout
        while time.monotonic() < deadline:
            line = self.read_line(max(0.01, deadline - time.monotonic()))
            if not line:
                continue
            if line.startswith(("OK", "ERR ")):
                return line
            self.events.append(line)
        raise TimeoutError(f"no response for {command!r}")

    def replies(self, count, timeout=2.0):
        result = []
        deadline = time.monotonic() + timeout
        while len(result) < count and time.monotonic() < deadline:
            line = self.read_line(max(0.01, deadline - time.monotonic()))
            if not line:
                continue
            if line.startswith(("OK", "ERR ")):
                result.append(line)
            else:
                self.events.append(line)
        return result

    def wait_event(self, prefix, timeout=1.0):
        deadline = time.monotonic() + timeout
        while time.monotonic() < deadline:
            for line in self.events:
                if line.startswith(prefix):
                    return line
            line = self.read_line(max(0.01, deadline - time.monotonic()))
            if line and not line.startswith(("OK", "ERR ")):
                self.events.append(line)
                if line.startswith(prefix):
                    return line
        return ""

    def close(self):
        try:
            self.sock.close()
        except OSError:
            pass


@pytest.fixture
def running_server(world):
    server = TapServer(world, host="127.0.0.1", port=0)
    port = server.start()
    try:
        yield server, port
    finally:
        server.stop()


def wait_until(predicate, timeout=2.0):
    deadline = time.monotonic() + timeout
    while time.monotonic() < deadline:
        if predicate():
            return True
        time.sleep(0.01)
    return bool(predicate())


def test_greeting_authentication_and_quit_over_real_socket(running_server):
    server, port = running_server
    client = Wire(port)
    try:
        assert client.greeting == "OK hello proto=1"
        assert client.exchange("LOOK") == "ERR 400 NOT_AUTHENTICATED"
        assert client.exchange("CONNECT alice") == "OK Connected"
        assert "EVT STATS players=1" in client.events
        assert client.exchange("WHO") == "OK players=1"
        assert client.exchange("QUIT") == "OK bye"
        assert wait_until(lambda: server.client_count == 0)
        assert "alice" not in server.state.players
    finally:
        client.close()


def test_fragmented_and_coalesced_tcp_lines(running_server):
    _, port = running_server
    client = Wire(port)
    try:
        client.send(b"CON")
        assert client.read_line(0.05) == ""
        client.send(b"NECT split\nWHO\nSTATUS\n")
        responses = client.replies(3)
        assert responses[0] == "OK Connected"
        assert responses[1] == "OK players=1"
        status = json.loads(responses[2][3:])
        assert status["status"] == "alive"
    finally:
        client.close()


def test_line_too_long_is_reported_and_stream_recovers(running_server):
    _, port = running_server
    client = Wire(port)
    try:
        assert client.exchange("CONNECT longline") == "OK Connected"
        client.send(b"x" * 1025)
        assert client.read_line() == "ERR 413 LINE_TOO_LONG"
        client.send(b"\nWHO\n")
        assert client.read_line() == "OK players=1"
    finally:
        client.close()


def test_invalid_utf8_is_one_bad_request_reply(running_server):
    _, port = running_server
    client = Wire(port)
    try:
        client.send(b"\xff\n")
        assert client.read_line() == "ERR 400 BAD_REQUEST"
        assert client.exchange("CONNECT utf8") == "OK Connected"
    finally:
        client.close()


def test_two_clients_share_world_and_receive_correct_broadcasts(running_server):
    _, port = running_server
    alice, bob = Wire(port), Wire(port)
    try:
        charlie = Wire(port)
        try:
            assert alice.exchange("CONNECT alice") == "OK Connected"
            assert bob.exchange("CONNECT bob") == "OK Connected"
            assert charlie.exchange("CONNECT charlie") == "OK Connected"
            assert charlie.exchange("MOVE east") == "OK room=loc.bunk"
            alice.events.clear()
            bob.events.clear()
            charlie.events.clear()
            assert alice.exchange("MOVE east") == "OK room=loc.bunk"
            assert bob.wait_event("EVT ROOM PRESENCE LEAVE alice")
            assert charlie.wait_event("EVT ROOM PRESENCE ENTER alice")
            alice.events.clear()
            bob.events.clear()
            charlie.events.clear()
            assert alice.exchange("CHAT GLOBAL hello from the sand") == "OK"
            assert "(GLOBAL) alice: hello from the sand" in alice.events
            assert bob.wait_event("(GLOBAL) alice: hello from the sand")
            assert charlie.wait_event("(GLOBAL) alice: hello from the sand")
            assert not any("PRESENCE" in line for line in alice.events)
        finally:
            charlie.close()
    finally:
        alice.close()
        bob.close()


def test_multiple_commands_each_receive_one_reply(running_server):
    _, port = running_server
    client = Wire(port)
    try:
        assert client.exchange("CONNECT pipelined") == "OK Connected"
        client.send("WHO\nSTATUS\n")
        responses = client.replies(2)
        assert len(responses) == 2
        assert responses[0] == "OK players=1"
        assert json.loads(responses[1][3:])["max_hp"] == 100
    finally:
        client.close()


def test_eof_cleanup_does_not_affect_other_clients(running_server):
    server, port = running_server
    abandoned, survivor = Wire(port), Wire(port)
    try:
        assert abandoned.exchange("CONNECT gone") == "OK Connected"
        assert survivor.exchange("CONNECT alive") == "OK Connected"
        abandoned.close()
        assert wait_until(lambda: "gone" not in server.state.players)
        assert survivor.exchange("WHO") == "OK players=1"
    finally:
        abandoned.close()
        survivor.close()


def test_flood_limit_returns_902_then_closes_client(running_server):
    server, port = running_server
    client = Wire(port)
    try:
        assert client.exchange("CONNECT flood") == "OK Connected"
        # CONNECT counts as one command in the same sliding-window budget.
        client.send(("WHO\n" * 20).encode())
        results = client.replies(20, timeout=3)
        assert len(results) == 20
        assert results[:19] == ["OK players=1"] * 19
        assert results[-1] == "ERR 902 FLOODING"
        assert wait_until(lambda: "flood" not in server.state.players)
    finally:
        client.close()


def test_server_stop_cleans_clients_and_players(running_server):
    server, port = running_server
    client = Wire(port)
    assert client.exchange("CONNECT stopme") == "OK Connected"
    server.stop()
    assert server.client_count == 0
    assert server.state.players == {}
    client.close()


def test_admission_control_caps_clients_and_rapid_connections(world, monkeypatch):
    import server as server_module
    server = TapServer(world, port=0)
    monkeypatch.setattr(server_module, "MAX_CONNECTIONS", 1)
    with server._lock:
        server._clients[1] = object()
    assert not server._admit(("127.0.0.1", 10))
    with server._lock:
        server._clients.clear()
    for index in range(RAPID_CONNECTIONS):
        assert server._admit(("127.0.0.1", index))
    assert not server._admit(("127.0.0.1", 99))
    # An expired history is pruned using monotonic timestamps.
    server._connect_history["127.0.0.1"] = [time.monotonic() - RAPID_WINDOW_S - 1]
    assert server._admit(("127.0.0.1", 100))


def test_structured_server_logs_are_valid_json(world):
    stream = io.StringIO()
    logger = JsonLogger(stream=stream)
    server = TapServer(world, host="127.0.0.1", port=0, logger=logger)
    port = server.start()
    client = Wire(port)
    try:
        assert client.exchange("CONNECT logger") == "OK Connected"
        assert client.exchange("WHO") == "OK players=1"
    finally:
        client.close()
        time.sleep(0.05)
        server.stop()
    records = [json.loads(line) for line in stream.getvalue().splitlines()]
    assert all({"time", "level", "msg"} <= row.keys() for row in records)
    assert any(row.get("command") == "WHO" for row in records)
    assert any(row.get("type") == "shutdown" for row in records)


def test_server_factory_creates_unbound_server(world):
    from server import create_server
    server = create_server(world, host="127.0.0.1", port=0)
    assert server.client_count == 0
    assert server._listener is None
