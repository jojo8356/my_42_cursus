from __future__ import annotations

import json
import socket
import threading
import time

from server import TapServer


class Peer:
    def __init__(self, port):
        self.sock = socket.create_connection(("127.0.0.1", port), timeout=2)
        self.sock.settimeout(0.05)
        self.buf = bytearray()
        self.events = []
        self.greeting = self.read_line(2)

    def read_line(self, timeout=2):
        deadline = time.monotonic() + timeout
        while time.monotonic() < deadline:
            pos = self.buf.find(b"\n")
            if pos >= 0:
                line = bytes(self.buf[:pos]).decode("utf-8", "replace").rstrip("\r")
                del self.buf[:pos + 1]
                return line
            try:
                chunk = self.sock.recv(4096)
            except socket.timeout:
                continue
            except OSError:
                return ""
            if not chunk:
                return ""
            self.buf.extend(chunk)
        return ""

    def send(self, text):
        if isinstance(text, bytes):
            self.sock.sendall(text)
        else:
            self.sock.sendall((text + "\n").encode("utf-8"))

    def command(self, text, timeout=2):
        self.send(text)
        deadline = time.monotonic() + timeout
        while time.monotonic() < deadline:
            line = self.read_line(max(0.01, deadline - time.monotonic()))
            if not line:
                continue
            if line.startswith(("OK", "ERR ")):
                return line
            self.events.append(line)
        raise TimeoutError(text)

    def collect_event(self, prefix, timeout=1):
        deadline = time.monotonic() + timeout
        while time.monotonic() < deadline:
            for line in self.events:
                if line.startswith(prefix):
                    return line
            line = self.read_line(max(0.01, deadline - time.monotonic()))
            if line:
                self.events.append(line)
        return ""

    def close(self):
        try:
            self.sock.close()
        except OSError:
            pass


def test_full_two_player_adventure_over_tcp(world):
    server = TapServer(world, "127.0.0.1", 0)
    port = server.start()
    alice, bob = Peer(port), Peer(port)
    try:
        assert alice.greeting == bob.greeting == "OK hello proto=1"
        assert alice.command("CONNECT Alice") == "OK Connected"
        assert bob.command("CONNECT Bob") == "OK Connected"
        assert alice.command("QUEST Mira").startswith("OK {\"id\":\"quest.fetch_key\"")
        assert alice.command("TAKE key") == "OK taken=item.key"
        assert alice.command("QUEST Mira").startswith("OK {\"quest_id\":\"quest.fetch_key\"")
        assert alice.command("MOVE east") == "OK room=loc.bunk"
        assert alice.command("MOVE west") == "OK room=loc.atrium"
        assert alice.command("INVENTORY") == 'OK ["item.gift"]'
        assert bob.command("GROUP CREATE explorers") == "OK group=explorers"
        assert bob.command("GROUP INVITE Alice") == "OK"
        assert alice.collect_event("EVT GROUP INVITE Bob")
        assert alice.command("GROUP JOIN explorers") == "OK group=explorers"
        assert bob.collect_event("EVT GROUP JOIN Alice")
        assert alice.command("CHAT GROUP together") == "OK"
        assert alice.collect_event("(GROUP) Alice: together")
        assert bob.collect_event("(GROUP) Alice: together")
        assert alice.command("WHO") == "OK players=2"
        assert alice.command("QUIT") == "OK bye"
        assert bob.command("WHO") == "OK players=1"
    finally:
        alice.close()
        bob.close()
        server.stop()


def test_concurrent_tcp_pickup_has_one_authoritative_winner(world):
    server = TapServer(world, "127.0.0.1", 0)
    port = server.start()
    peers = [Peer(port), Peer(port)]
    try:
        for peer, name in zip(peers, ("one", "two")):
            assert peer.command("CONNECT " + name) == "OK Connected"
        gate = threading.Barrier(3)
        results = []
        def race(peer):
            gate.wait()
            peer.send("TAKE key")
            deadline = time.monotonic() + 2
            while time.monotonic() < deadline:
                line = peer.read_line(max(0.01, deadline - time.monotonic()))
                if line.startswith(("OK", "ERR ")):
                    results.append(line)
                    return
                if line:
                    peer.events.append(line)
        workers = [threading.Thread(target=race, args=(peer,)) for peer in peers]
        for worker in workers:
            worker.start()
        gate.wait()
        for worker in workers:
            worker.join(timeout=3)
        assert sorted(results) == ["ERR 404 ITEM_NOT_FOUND", "OK taken=item.key"]
        assert sum("item.key" in player.inventory for player in server.state.players.values()) == 1
    finally:
        for peer in peers:
            peer.close()
        server.stop()


def test_tcp_server_keeps_serving_after_malformed_and_unknown_commands(world):
    server = TapServer(world, "127.0.0.1", 0)
    port = server.start()
    peer = Peer(port)
    try:
        assert peer.command("CONNECT resilient") == "OK Connected"
        assert peer.command("NOT_A_VERB") == "ERR 400 UNKNOWN_COMMAND"
        peer.send(b"\xff\n")
        assert peer.read_line() == "ERR 400 BAD_REQUEST"
        assert peer.command("WHO") == "OK players=1"
    finally:
        peer.close()
        server.stop()


def test_server_shutdown_is_idempotent_and_removes_live_session(world):
    server = TapServer(world, "127.0.0.1", 0)
    port = server.start()
    peer = Peer(port)
    assert peer.command("CONNECT shutdown") == "OK Connected"
    server.stop()
    server.stop()
    assert server.state.players == {}
    assert server.client_count == 0
    peer.close()
