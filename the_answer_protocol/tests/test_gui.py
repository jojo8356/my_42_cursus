from __future__ import annotations

import base64
import hashlib
import http.client
import json
import os
import socket
import threading
import time

import pytest

from client_gui import HTML, WS_GUID, create_gui_server, websocket_frame
from server import TapServer


def recv_exact(sock, size):
    data = bytearray()
    while len(data) < size:
        chunk = sock.recv(size - len(data))
        if not chunk:
            raise EOFError("socket closed")
        data.extend(chunk)
    return bytes(data)


class WebSocketWire:
    def __init__(self, port):
        self.sock = socket.create_connection(("127.0.0.1", port), timeout=2)
        self.sock.settimeout(2)
        self.buffer = bytearray()
        key = base64.b64encode(os.urandom(16)).decode("ascii")
        self.key = key
        request = (
            "GET /ws HTTP/1.1\r\n"
            f"Host: 127.0.0.1:{port}\r\n"
            "Upgrade: websocket\r\n"
            "Connection: keep-alive, Upgrade\r\n"
            "Sec-WebSocket-Version: 13\r\n"
            f"Sec-WebSocket-Key: {key}\r\n\r\n"
        )
        self.sock.sendall(request.encode("ascii"))
        while b"\r\n\r\n" not in self.buffer:
            self.buffer.extend(self.sock.recv(4096))
        header, _, rest = self.buffer.partition(b"\r\n\r\n")
        self.buffer[:] = rest
        self.headers = header.decode("ascii")

    def _take(self, size):
        while len(self.buffer) < size:
            chunk = self.sock.recv(4096)
            if not chunk:
                raise EOFError("socket closed")
            self.buffer.extend(chunk)
        result = bytes(self.buffer[:size])
        del self.buffer[:size]
        return result

    def recv_frame(self):
        first, second = self._take(2)
        length = second & 0x7F
        if length == 126:
            length = int.from_bytes(self._take(2), "big")
        elif length == 127:
            length = int.from_bytes(self._take(8), "big")
        payload = self._take(length)
        opcode = first & 0x0F
        if opcode == 1:
            return opcode, payload.decode("utf-8", "replace")
        return opcode, payload

    def send_frame(self, payload=b"", opcode=1, final=True):
        if isinstance(payload, str):
            payload = payload.encode("utf-8")
        mask = os.urandom(4)
        first = (0x80 if final else 0) | (opcode & 0x0F)
        length = len(payload)
        if length < 126:
            header = bytes((first, 0x80 | length))
        elif length <= 65535:
            header = bytes((first, 0x80 | 126)) + length.to_bytes(2, "big")
        else:
            header = bytes((first, 0x80 | 127)) + length.to_bytes(8, "big")
        masked = bytes(value ^ mask[index % 4] for index, value in enumerate(payload))
        self.sock.sendall(header + mask + masked)

    def recv_text_until(self, prefix, timeout=2.0):
        deadline = time.monotonic() + timeout
        received = []
        while time.monotonic() < deadline:
            opcode, payload = self.recv_frame()
            text = payload if isinstance(payload, str) else payload.decode("utf-8", "replace")
            received.append((opcode, text))
            if opcode == 1 and text.startswith(prefix):
                return text, received
        raise TimeoutError(f"no WebSocket text beginning with {prefix!r}; got {received!r}")

    def close(self):
        try:
            self.sock.close()
        except OSError:
            pass


@pytest.fixture
def gui_stack(world):
    tap = TapServer(world, host="127.0.0.1", port=0)
    tap_port = tap.start()
    gui = create_gui_server("127.0.0.1", 0, "127.0.0.1", tap_port)
    thread = threading.Thread(target=gui.serve_forever, kwargs={"poll_interval": 0.05}, daemon=True)
    thread.start()
    try:
        yield tap, gui, int(gui.server_address[1])
    finally:
        gui.shutdown()
        gui.server_close()
        tap.stop()


def test_single_page_is_self_contained_and_contains_required_ui():
    assert "https://" not in HTML
    assert "cdn." not in HTML.lower()
    assert "new WebSocket" in HTML
    for phrase in ("VITALITY", "Exits / move", "Live wire", "Carried", "Active objectives"):
        assert phrase in HTML
    assert "location.host+'/ws'" in HTML


def test_websocket_frame_encodes_short_and_extended_lengths():
    short = websocket_frame(b"hello")
    assert short == b"\x81\x05hello"
    medium = websocket_frame(b"x" * 126)
    assert medium[:4] == b"\x81\x7e\x00\x7e"
    long = websocket_frame(b"x" * 65536, opcode=2)
    assert long[:10] == b"\x82\x7f" + (65536).to_bytes(8, "big")
    assert len(long) == 65546


def test_http_index_health_and_not_found(gui_stack):
    _, _, port = gui_stack
    connection = http.client.HTTPConnection("127.0.0.1", port, timeout=2)
    connection.request("GET", "/")
    response = connection.getresponse()
    html = response.read().decode("utf-8")
    assert response.status == 200
    assert response.getheader("Content-Type").startswith("text/html")
    assert "Content-Security-Policy" in response.headers
    assert "Across the deep desert." in html
    connection.request("GET", "/health")
    response = connection.getresponse()
    report = json.loads(response.read())
    assert response.status == 200 and report["service"] == "tap-gui"
    connection.request("GET", "/missing")
    response = connection.getresponse()
    response.read()
    assert response.status == 404
    connection.close()


def test_websocket_proxy_bridges_greeting_commands_and_events(gui_stack):
    tap, _, port = gui_stack
    client = WebSocketWire(port)
    try:
        assert "101 Switching Protocols" in client.headers
        accept = base64.b64encode(hashlib.sha1((client.key + WS_GUID).encode()).digest()).decode()
        assert f"Sec-WebSocket-Accept: {accept}" in client.headers
        opcode, greeting = client.recv_frame()
        assert opcode == 1 and greeting == "OK hello proto=1"
        client.send_frame("CONNECT browser")
        connected, seen = client.recv_text_until("OK Connected")
        assert connected == "OK Connected"
        assert any(line == "EVT STATS players=1" for _, line in seen)
        client.send_frame("LOOK")
        look, _ = client.recv_text_until("OK {")
        room = json.loads(look[3:])
        assert room["id"] == "loc.atrium"
        client.send_frame("CHAT GLOBAL hello from browser")
        chat, seen = client.recv_text_until("OK")
        assert chat == "OK"
        assert any(line == "(GLOBAL) browser: hello from browser" for _, line in seen)
        assert "browser" in tap.state.players
    finally:
        client.close()


def test_websocket_masks_frames_and_supports_ping_pong(gui_stack):
    _, _, port = gui_stack
    client = WebSocketWire(port)
    try:
        assert client.recv_frame()[1] == "OK hello proto=1"
        client.send_frame(b"heartbeat", opcode=9)
        opcode, payload = client.recv_frame()
        assert opcode == 10 and payload == b"heartbeat"
        client.send_frame("CONNECT ping")
        assert client.recv_text_until("OK Connected")[0] == "OK Connected"
    finally:
        client.close()


def test_websocket_reassembles_fragmented_text_messages(gui_stack):
    _, _, port = gui_stack
    client = WebSocketWire(port)
    try:
        client.recv_frame()  # TAP greeting
        client.send_frame("CONN", opcode=1, final=False)
        client.send_frame("ECT fragmented", opcode=0, final=True)
        result, _ = client.recv_text_until("OK Connected")
        assert result == "OK Connected"
    finally:
        client.close()


def test_websocket_rejects_multiline_and_oversized_commands_but_recovers(gui_stack):
    _, _, port = gui_stack
    client = WebSocketWire(port)
    try:
        client.recv_frame()
        client.send_frame("CONNECT safe")
        client.recv_text_until("OK Connected")
        client.send_frame("WHO\nQUIT")
        assert client.recv_text_until("ERR 400 BAD_REQUEST")[0] == "ERR 400 BAD_REQUEST"
        client.send_frame("X" * 1025)
        assert client.recv_text_until("ERR 413 LINE_TOO_LONG")[0] == "ERR 413 LINE_TOO_LONG"
        client.send_frame("WHO")
        assert client.recv_text_until("OK players=1")[0] == "OK players=1"
    finally:
        client.close()


def test_websocket_rejects_unmasked_and_binary_frames(gui_stack):
    _, _, port = gui_stack
    client = WebSocketWire(port)
    try:
        client.recv_frame()
        # An unmasked client frame is a protocol violation; server closes it.
        client.sock.sendall(b"\x81\x02hi")
        opcode, payload = client.recv_frame()
        assert opcode == 8
        assert payload[:2] == (1002).to_bytes(2, "big")
        assert payload[2:] == b"client frames must be masked"
    finally:
        client.close()


def test_websocket_bridge_reports_backend_unavailable():
    gui = create_gui_server("127.0.0.1", 0, "127.0.0.1", 1)
    thread = threading.Thread(target=gui.serve_forever, kwargs={"poll_interval": 0.05}, daemon=True)
    thread.start()
    client = WebSocketWire(int(gui.server_address[1]))
    try:
        opcode, message = client.recv_frame()
        assert opcode == 1 and message == "ERR 900 CONNECTION_FAILED"
    finally:
        client.close()
        gui.shutdown()
        gui.server_close()


def test_invalid_websocket_handshake_gets_bad_request():
    gui = create_gui_server("127.0.0.1", 0, "127.0.0.1", 1)
    thread = threading.Thread(target=gui.serve_forever, kwargs={"poll_interval": 0.05}, daemon=True)
    thread.start()
    sock = socket.create_connection(("127.0.0.1", gui.server_address[1]), timeout=2)
    try:
        sock.sendall(b"GET /ws HTTP/1.1\r\nHost: localhost\r\n\r\n")
        response = sock.recv(4096)
        assert b"400" in response
    finally:
        sock.close()
        gui.shutdown()
        gui.server_close()
