from __future__ import annotations

import base64
import io
import os
import queue
import socket
import threading
import time

import pytest

import protocol
import client_gui
from client_gui import (MAX_WS_MESSAGE, _close_payload, _read_exact, _valid_close_code,
                         create_gui_server)
from server import TapServer
from test_gui import WebSocketWire


@pytest.fixture
def gui_stack(world):
    tap = TapServer(world, host="127.0.0.1", port=0)
    tap_port = tap.start()
    gui = create_gui_server("127.0.0.1", 0, "127.0.0.1", tap_port)
    thread = threading.Thread(target=gui.serve_forever,
                              kwargs={"poll_interval": 0.05}, daemon=True)
    thread.start()
    try:
        yield tap, gui, int(gui.server_address[1])
    finally:
        gui.shutdown()
        gui.server_close()
        tap.stop()


_ORIGINAL_CREATE_CONNECTION = socket.create_connection


class FakeBackend:
    def __init__(self, incoming=(), *, fail_send=False, fail_shutdown=False,
                 fail_close=False, release_on_shutdown=None):
        self.incoming = queue.Queue()
        for item in incoming:
            self.incoming.put(item)
        self.fail_send = fail_send
        self.fail_shutdown = fail_shutdown
        self.fail_close = fail_close
        self.release_on_shutdown = release_on_shutdown
        self.sent = []
        self.closed = False
        self.timeout = 0.2
        self.recv_count = 0

    def settimeout(self, value):
        self.timeout = value

    def recv(self, _size):
        self.recv_count += 1
        try:
            item = self.incoming.get(timeout=self.timeout)
        except queue.Empty:
            raise socket.timeout()
        if isinstance(item, BaseException):
            raise item
        return item

    def sendall(self, data):
        if self.fail_send:
            raise OSError("backend send failed")
        self.sent.append(data)

    def shutdown(self, _how):
        if self.release_on_shutdown is not None:
            self.release_on_shutdown.set()
        if self.fail_shutdown:
            raise OSError("backend shutdown failed")

    def close(self):
        self.closed = True
        if self.fail_close:
            raise OSError("backend close failed")


def raw_ws(port, key=None, version="13", upgrade="websocket", connection="Upgrade"):
    key = key or base64.b64encode(os.urandom(16)).decode("ascii")
    client = _ORIGINAL_CREATE_CONNECTION(("127.0.0.1", port), timeout=2)
    client.settimeout(2)
    request = (
        "GET /ws HTTP/1.1\r\n"
        f"Host: 127.0.0.1:{port}\r\n"
        f"Upgrade: {upgrade}\r\n"
        f"Connection: {connection}\r\n"
        f"Sec-WebSocket-Version: {version}\r\n"
        f"Sec-WebSocket-Key: {key}\r\n\r\n"
    )
    client.sendall(request.encode("ascii"))
    response = bytearray()
    while b"\r\n\r\n" not in response:
        response.extend(client.recv(4096))
    header, _, rest = response.partition(b"\r\n\r\n")
    return client, header.decode("ascii"), bytearray(rest)


def masked_frame(payload=b"", opcode=1, final=True, mask=b"abcd"):
    if isinstance(payload, str):
        payload = payload.encode("utf-8")
    first = (0x80 if final else 0) | opcode
    length = len(payload)
    if length < 126:
        header = bytes((first, 0x80 | length))
    elif length <= 65535:
        header = bytes((first, 0x80 | 126)) + length.to_bytes(2, "big")
    else:
        header = bytes((first, 0x80 | 127)) + length.to_bytes(8, "big")
    encoded = bytes(value ^ mask[index % 4] for index, value in enumerate(payload))
    return header + mask + encoded


def closed_message(client):
    opcode, data = client.recv_frame()
    if opcode == 8 and isinstance(data, bytes) and len(data) >= 2:
        code = int.from_bytes(data[:2], "big")
        return opcode, (code, data[2:].decode("utf-8", "replace"))
    return opcode, data.decode("utf-8", "replace") if isinstance(data, bytes) else data


def make_fake_gui(monkeypatch, backend):
    host, tap_port = "fake-tap", 46382
    gui = create_gui_server("127.0.0.1", 0, host, tap_port)
    thread = threading.Thread(target=gui.serve_forever,
                              kwargs={"poll_interval": 0.05}, daemon=True)
    thread.start()

    def create_connection(address, timeout=None, source_address=None, all_errors=False):
        if address == (host, tap_port):
            return backend
        return _ORIGINAL_CREATE_CONNECTION(address, timeout, source_address,
                                           all_errors=all_errors)

    monkeypatch.setattr(client_gui.socket, "create_connection", create_connection)
    return gui, thread


def cleanup_gui(gui, thread):
    gui.shutdown()
    gui.server_close()
    thread.join(timeout=1)


def wait_until(predicate, timeout=1):
    deadline = time.monotonic() + timeout
    while time.monotonic() < deadline:
        if predicate():
            return True
        time.sleep(0.005)
    return bool(predicate())


def test_read_exact_returns_all_bytes_or_empty_on_early_eof():
    assert _read_exact(io.BytesIO(b"abcdef"), 4) == b"abcd"
    assert _read_exact(io.BytesIO(b"ab"), 4) == b""
    assert _read_exact(io.BytesIO(b""), 0) == b""


@pytest.mark.parametrize(
    "kwargs",
    [
        {"key": "!!!"},
        {"key": "a"},
        {"version": "12"},
        {"upgrade": "not-websocket"},
        {"connection": "keep-alive"},
        {"connection": "notupgrade"},
    ],
)
def test_websocket_handshake_rejects_each_invalid_header(gui_stack, kwargs):
    _, _, port = gui_stack
    client, headers, _ = raw_ws(port, **kwargs)
    try:
        assert "400" in headers
    finally:
        client.close()


def test_websocket_close_frame_echoes_close_and_pong_is_ignored(gui_stack):
    _, _, port = gui_stack
    client = WebSocketWire(port)
    try:
        assert client.recv_frame()[1] == "OK hello proto=1"
        client.send_frame(b"opaque\xff", opcode=9)
        opcode, payload = client.recv_frame()
        assert opcode == 10 and payload == b"opaque\xff"
        client.send_frame("", opcode=10)
        client.send_frame((1000).to_bytes(2, "big") + b"bye", opcode=8)
        opcode, payload = client.recv_frame()
        assert opcode == 8 and payload == (1000).to_bytes(2, "big") + b"bye"
    finally:
        client.close()


def test_websocket_rejects_rsv_bits_unsupported_opcodes_and_binary(gui_stack):
    _, _, port = gui_stack
    for frame, expected in ((masked_frame("x", opcode=1)[:1], "unsupported WebSocket extension"),
                            (masked_frame("x", opcode=3), "unsupported opcode"),
                            (masked_frame("binary", opcode=2), "text frames only")):
        client = WebSocketWire(port)
        try:
            client.recv_frame()
            if len(frame) == 1:
                full = masked_frame("x", opcode=1)
                client.sock.sendall(bytes((full[0] | 0x40,)) + full[1:])
            else:
                client.sock.sendall(frame)
            opcode, text = closed_message(client)
            assert opcode == 8 and text == (1002 if expected != "text frames only" else 1003, expected)
        finally:
            client.close()


def test_websocket_rejects_fragment_order_and_nonfinal_control_frames(gui_stack):
    _, _, port = gui_stack
    cases = [
        [("x", 0, True)],
        [("first", 1, False), ("second", 1, True)],
        [("ping", 9, False)],
    ]
    expected = ["unexpected continuation", "nested fragmented message", "invalid control frame"]
    for frames, error in zip(cases, expected):
        client = WebSocketWire(port)
        try:
            client.recv_frame()
            for text, opcode, final in frames:
                client.send_frame(text, opcode=opcode, final=final)
            opcode, text = closed_message(client)
            expected_code = 1009 if error == "message too large" else 1002
            assert opcode == 8 and text == (expected_code, error)
        finally:
            client.close()


def test_websocket_rejects_oversize_control_and_data_messages(gui_stack):
    _, _, port = gui_stack
    for frame_opcode, payload, expected in (
        (9, b"P" * 126, "invalid control frame"),
        (1, b"X" * (MAX_WS_MESSAGE + 1), "message too large"),
    ):
        client = WebSocketWire(port)
        try:
            client.recv_frame()
            client.send_frame(payload, opcode=frame_opcode)
            opcode, text = closed_message(client)
            expected_code = 1009 if expected == "message too large" else 1002
            assert opcode == 8 and text == (expected_code, expected)
        finally:
            client.close()


def test_websocket_rejects_64bit_high_bit_length_and_incomplete_frames(gui_stack):
    _, _, port = gui_stack
    client = WebSocketWire(port)
    client.recv_frame()
    client.sock.sendall(b"\x81\xff" + (1 << 63).to_bytes(8, "big"))
    opcode, payload = closed_message(client)
    assert opcode == 8 and payload == (1002, "invalid frame length")
    client.close()

    for raw in (b"\x81\xfe", b"\x81\xff\x00", b"\x81\x81", b"\x81\x82abcdx"):
        client = WebSocketWire(port)
        client.recv_frame()
        client.sock.sendall(raw)
        client.sock.shutdown(socket.SHUT_WR)
        assert client.sock.recv(1) == b""
        client.close()


def test_websocket_handles_pong_blank_extended_126_and_127_line_limits(gui_stack):
    _, _, port = gui_stack
    client = WebSocketWire(port)
    try:
        client.recv_frame()
        client.send_frame(b"P", opcode=10)
        client.send_frame("  ")
        client.send_frame("X" * 126)
        assert client.recv_text_until("ERR 400 UNKNOWN_COMMAND")[0] == "ERR 400 UNKNOWN_COMMAND"
        client.send_frame("X" * 65536)
        assert client.recv_text_until("ERR 413 LINE_TOO_LONG")[0] == "ERR 413 LINE_TOO_LONG"
        client.send_frame("WHO\r")
        assert client.recv_text_until("ERR 400 BAD_REQUEST")[0] == "ERR 400 BAD_REQUEST"
        client.send_frame("CONNECT web")
        assert client.recv_text_until("OK Connected")[0] == "OK Connected"
        client.send_frame(b"\xff")
        opcode, payload = closed_message(client)
        assert opcode == 8 and payload == (1007, "invalid UTF-8 text")
    finally:
        client.close()


def test_websocket_fragmented_text_over_max_is_closed(gui_stack):
    _, _, port = gui_stack
    client = WebSocketWire(port)
    try:
        client.recv_frame()
        client.send_frame(b"a" * MAX_WS_MESSAGE, opcode=1, final=False)
        client.send_frame(b"b", opcode=0, final=True)
        opcode, text = closed_message(client)
        assert opcode == 8 and text == (1009, "message too large")
    finally:
        client.close()


def test_websocket_pending_backend_line_buffer_is_bounded(monkeypatch):
    backend = FakeBackend([b"x" * (protocol.MAX_LINE_BYTES * 4 + 1),
                           b"OK hello proto=1\n", b""])
    gui, thread = make_fake_gui(monkeypatch, backend)
    client = WebSocketWire(int(gui.server_address[1]))
    try:
        assert client.recv_frame()[1] == "OK hello proto=1"
        assert wait_until(lambda: backend.recv_count >= 3)
    finally:
        client.close()
        cleanup_gui(gui, thread)


def test_websocket_relay_truncates_oversized_backend_line_safely(monkeypatch):
    backend = FakeBackend([b"Z" * (MAX_WS_MESSAGE + 1) + b"\n", b""])
    gui, thread = make_fake_gui(monkeypatch, backend)
    client = WebSocketWire(int(gui.server_address[1]))
    try:
        opcode, payload = client.recv_frame()
        assert opcode == 1 and len(payload) == MAX_WS_MESSAGE
    finally:
        client.close()
        cleanup_gui(gui, thread)


def test_websocket_relay_handles_timeout_and_backend_eof(monkeypatch):
    backend = FakeBackend([socket.timeout(), b"OK hello proto=1\n", b""])
    gui, thread = make_fake_gui(monkeypatch, backend)
    client = WebSocketWire(int(gui.server_address[1]))
    try:
        assert client.recv_frame()[1] == "OK hello proto=1"
        assert wait_until(lambda: backend.recv_count >= 3)
    finally:
        client.close()
        cleanup_gui(gui, thread)


def test_websocket_relay_stops_on_invalid_utf8_backend_message(monkeypatch):
    backend = FakeBackend([b"\xff\n"])
    gui, thread = make_fake_gui(monkeypatch, backend)
    client = WebSocketWire(int(gui.server_address[1]))
    try:
        assert wait_until(lambda: backend.recv_count >= 1)
        time.sleep(0.02)
        client.send_frame(b"", opcode=8)
        assert client.sock.recv(1) == b""
    finally:
        client.close()
        cleanup_gui(gui, thread)


def test_websocket_backend_send_and_cleanup_errors_are_reported(monkeypatch):
    backend = FakeBackend([b"OK hello proto=1\n"], fail_send=True,
                          fail_shutdown=True, fail_close=True)
    gui, thread = make_fake_gui(monkeypatch, backend)
    client = WebSocketWire(int(gui.server_address[1]))
    try:
        assert client.recv_frame()[1] == "OK hello proto=1"
        client.send_frame("WHO")
        assert client.recv_text_until("ERR 901 SEND_FAILED")[0] == "ERR 901 SEND_FAILED"
    finally:
        client.close()
        cleanup_gui(gui, thread)
    assert backend.closed and backend.sent == []


def test_websocket_frame_after_client_close_is_suppressed(monkeypatch):
    release = threading.Event()
    backend = FakeBackend([b"OK hello proto=1\n"], release_on_shutdown=release)
    original_recv = backend.recv
    def blocked_recv(size):
        if backend.recv_count >= 2:
            release.wait(1)
            return b"OK too late\n"
        return original_recv(size)
    backend.recv = blocked_recv
    gui, thread = make_fake_gui(monkeypatch, backend)
    client = WebSocketWire(int(gui.server_address[1]))
    try:
        assert client.recv_frame()[1] == "OK hello proto=1"
        client.send_frame(b"", opcode=8)
        assert client.recv_frame() == (8, b"")
        release.set()
    finally:
        release.set()
        client.close()
        cleanup_gui(gui, thread)


def test_backend_unavailable_still_reports_connection_failed():
    gui = create_gui_server("127.0.0.1", 0, "127.0.0.1", 1)
    thread = threading.Thread(target=gui.serve_forever,
                              kwargs={"poll_interval": 0.05}, daemon=True)
    thread.start()
    client = WebSocketWire(int(gui.server_address[1]))
    try:
        assert client.recv_frame() == (1, "ERR 900 CONNECTION_FAILED")
    finally:
        client.close()
        cleanup_gui(gui, thread)


def test_gui_serve_entrypoint_cleans_up_on_keyboard_interrupt(monkeypatch, capsys):
    events = []
    class FakeHTTP:
        server_address = ("0.0.0.0", 8123)
        def serve_forever(self, poll_interval):
            events.append(("serve", poll_interval))
            raise KeyboardInterrupt()
        def shutdown(self):
            events.append("shutdown")
        def server_close(self):
            events.append("close")
    monkeypatch.setattr(client_gui, "create_gui_server", lambda *args: FakeHTTP())
    monkeypatch.setattr(client_gui, "JsonLogger", lambda: object())
    client_gui.serve_gui("0.0.0.0", 8123, logger=None)
    assert events == [("serve", 0.2), "shutdown", "close"]
    assert "8123" in capsys.readouterr().out


def test_websocket_frame_writer_handles_stopped_connections_and_failed_streams():
    class Stop:
        def __init__(self, stopped=False):
            self.stopped = stopped
        def is_set(self):
            return self.stopped
    class Stream:
        def __init__(self, fail=False):
            self.fail = fail
            self.data = bytearray()
            self.flush_count = 0
        def write(self, data):
            if self.fail:
                raise OSError("client disappeared")
            self.data.extend(data)
        def flush(self):
            self.flush_count += 1
    lock = threading.Lock()
    stream = Stream()
    assert client_gui._write_ws_frame(stream, lock, Stop(), b"hello")
    assert stream.data == b"\x81\x05hello" and stream.flush_count == 1
    assert not client_gui._write_ws_frame(stream, lock, Stop(True), b"ignored")
    oversized = Stream()
    assert client_gui._write_ws_frame(oversized, lock, Stop(), b"x" * (MAX_WS_MESSAGE + 1))
    assert len(oversized.data) == MAX_WS_MESSAGE + 10
    assert not client_gui._try_write_ws_frame(Stream(fail=True), lock, Stop(), b"no")


def test_websocket_supports_nonfinal_continuation_before_final_fragment(gui_stack):
    _, _, port = gui_stack
    client = WebSocketWire(port)
    try:
        client.recv_frame()
        client.send_frame("CON", opcode=1, final=False)
        client.send_frame("NEC", opcode=0, final=False)
        client.send_frame("T fragmented", opcode=0, final=True)
        assert client.recv_text_until("OK Connected")[0] == "OK Connected"
    finally:
        client.close()


def test_websocket_outer_socket_error_is_logged_and_cleanup_continues(monkeypatch):
    class Logger:
        def __init__(self):
            self.records = []
        def log(self, level, msg, **fields):
            self.records.append({"level": level, "msg": msg, **fields})
    class BrokenReader:
        def read(self, _size):
            raise OSError("reset by peer")
    class Writer:
        def write(self, _data):
            pass
        def flush(self):
            pass
    from types import SimpleNamespace
    backend = FakeBackend()
    logger = Logger()
    handler = object.__new__(client_gui.GuiRequestHandler)
    handler.server = SimpleNamespace(logger=logger, tap_host="fake", tap_port=1)
    handler.client_address = ("127.0.0.1", 99)
    handler.rfile = BrokenReader()
    handler.wfile = Writer()
    monkeypatch.setattr(client_gui.socket, "create_connection", lambda *_a, **_k: backend)
    handler._websocket_session()
    assert any(record["msg"] == "GUI WebSocket closed" for record in logger.records)
    assert backend.closed


def test_unavailable_backend_write_failure_is_best_effort(monkeypatch):
    class Logger:
        def log(self, *_args, **_kwargs):
            pass
    class BrokenWriter:
        def write(self, _data):
            raise OSError("client reset")
        def flush(self):
            pass
    from types import SimpleNamespace
    handler = object.__new__(client_gui.GuiRequestHandler)
    handler.server = SimpleNamespace(logger=Logger(), tap_host="fake", tap_port=1)
    handler.client_address = ("127.0.0.1", 99)
    handler.rfile = io.BytesIO()
    handler.wfile = BrokenWriter()
    monkeypatch.setattr(client_gui.socket, "create_connection",
                        lambda *_a, **_k: (_ for _ in ()).throw(OSError("refused")))
    handler._websocket_session()


def test_close_payload_truncation_and_close_code_registry():
    assert _close_payload(1000, "normal") == (1000).to_bytes(2, "big") + b"normal"
    truncated = _close_payload(1000, "a" * 122 + "é")
    assert len(truncated) == 2 + 122
    assert truncated[2:] == b"a" * 122
    cases = {
        999: False, 1000: True, 1004: False, 1005: False, 1006: False,
        1015: False, 1016: False, 2999: False, 3000: True, 4999: True, 5000: False,
    }
    assert {code: _valid_close_code(code) for code in cases} == cases


def test_websocket_rejects_invalid_close_status_and_reason(gui_stack):
    _, _, port = gui_stack
    frames = [
        (b"x", (1002, "invalid close payload")),
        ((1005).to_bytes(2, "big"), (1002, "invalid close status")),
        ((1000).to_bytes(2, "big") + b"\xff", (1007, "invalid close reason")),
    ]
    for payload, expected in frames:
        client = WebSocketWire(port)
        try:
            client.recv_frame()
            client.send_frame(payload, opcode=8)
            assert closed_message(client) == (8, expected)
        finally:
            client.close()


def test_websocket_rejects_noncanonical_126_and_127_lengths(gui_stack):
    _, _, port = gui_stack
    frames = [
        b"\x81\xfe\x00\x01abcd" + bytes([ord("x") ^ ord("a")]),
        b"\x81\xff" + (1).to_bytes(8, "big") + b"abcd" + bytes([ord("x") ^ ord("a")]),
    ]
    for frame in frames:
        client = WebSocketWire(port)
        try:
            client.recv_frame()
            client.sock.sendall(frame)
            assert closed_message(client) == (8, (1002, "non-canonical frame length"))
        finally:
            client.close()
