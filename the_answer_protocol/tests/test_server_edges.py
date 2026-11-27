from __future__ import annotations

import queue
import signal
import socket
import threading
import time

import pytest

import protocol
import server as server_module
from logging_json import NullLogger
from server import ClientConnection, QueuedSession, TapServer, _SENTINEL


class FakeSocket:
    def __init__(self, recv_items=(), fail_send=False, fail_shutdown=False, fail_close=False):
        self.recv_items = list(recv_items)
        self.sent = []
        self.fail_send = fail_send
        self.fail_shutdown = fail_shutdown
        self.fail_close = fail_close
        self.closed = False
        self.timeout = None

    def settimeout(self, value):
        self.timeout = value

    def setsockopt(self, *_args):
        pass

    def bind(self, _address):
        pass

    def listen(self, _count):
        pass

    def getsockname(self):
        return "127.0.0.1", 43121

    def accept(self):
        item = self.recv_items.pop(0)
        if isinstance(item, BaseException):
            raise item
        return item

    def recv(self, _size):
        item = self.recv_items.pop(0)
        if isinstance(item, BaseException):
            raise item
        return item

    def sendall(self, data):
        if self.fail_send:
            raise OSError("broken pipe")
        self.sent.append(data)

    def shutdown(self, _how):
        if self.fail_shutdown:
            raise OSError("shutdown failed")

    def close(self):
        self.closed = True
        if self.fail_close:
            raise OSError("close failed")


class CaptureLogger(NullLogger):
    def __init__(self):
        self.records = []

    def log(self, level, msg, **fields):
        record = super().log(level, msg, **fields)
        self.records.append(record)
        return record


class FakeThread:
    def __init__(self, alive=False):
        self.alive = alive
        self.joined = []

    def is_alive(self):
        return self.alive

    def join(self, timeout=None):
        self.joined.append(timeout)


class FakeEvent:
    def __init__(self, waits=()):
        self.waits = list(waits)
        self.is_set_value = False
        self.set_count = 0

    def wait(self, _timeout=None):
        result = self.waits.pop(0) if self.waits else True
        if isinstance(result, BaseException):
            raise result
        return result

    def set(self):
        self.set_count += 1
        self.is_set_value = True

    def clear(self):
        self.is_set_value = False

    def is_set(self):
        return self.is_set_value


def test_queued_session_handles_closed_full_and_overflow_callback(world):
    full = queue.Queue(maxsize=1)
    full.put("occupied")
    callback = []
    session = QueuedSession(full, on_overflow=lambda: callback.append("overflow"))
    session.send("lost")
    assert session.closed and session.kicked
    assert callback == ["overflow"]
    session.send("ignored")
    assert full.get_nowait() == "occupied"
    assert full.empty()

    empty = queue.Queue(maxsize=1)
    session = QueuedSession(empty)
    session.send("queued")
    assert empty.get_nowait() == "queued"
    session.closed = True
    session.send("not queued")
    assert empty.empty()


def test_bind_is_idempotent_and_start_returns_existing_port(world):
    server = TapServer(world, host="127.0.0.1", port=0)
    port = server.bind()
    assert server.bind() == port
    server._running.set()
    assert server.start() == port
    server.stop()


def test_serve_forever_requires_bind_and_handles_timeout_refusal_and_listener_error(world, monkeypatch):
    server = TapServer(world, logger=CaptureLogger())
    with pytest.raises(RuntimeError, match="call bind"):
        server.serve_forever()

    listener = FakeSocket([socket.timeout(), (FakeSocket(), ("192.0.2.2", 42)), OSError("closed")])
    server._listener = listener
    server._running.set()
    refused = []
    monkeypatch.setattr(server, "_admit", lambda _address: False)
    monkeypatch.setattr(server, "_refuse", lambda sock, address: refused.append((sock, address)))
    server.serve_forever()
    assert len(refused) == 1 and refused[0][1] == ("192.0.2.2", 42)
    assert any(record["msg"] == "accept loop ended" for record in server.logger.records)


def test_refuse_sends_error_and_closes_even_after_socket_failures(world):
    server = TapServer(world)
    sock = FakeSocket(fail_send=True, fail_close=True)
    server._refuse(sock, ("127.0.0.1", 1))
    assert sock.closed
    sock = FakeSocket()
    server._refuse(sock, ("127.0.0.1", 1))
    assert sock.sent == [b"ERR 900 CONNECTION_FAILED\n"] and sock.closed


def test_tick_loop_logs_exceptions_and_stops_after_signal(world, monkeypatch):
    logger = CaptureLogger()
    server = TapServer(world, logger=logger)
    stop = FakeEvent([False, True])
    server._ticker_stop = stop
    server.state.tick = lambda: (_ for _ in ()).throw(RuntimeError("timer bug"))
    monkeypatch.setattr(server_module, "TICK_INTERVAL_S", 0)
    server._tick_loop()
    assert any(record["type"] == "timer" and "timer bug" in record["error"]
               for record in logger.records)


def test_writer_loop_sends_queue_and_stops_on_sentinel(world):
    server = TapServer(world)
    sock = FakeSocket()
    connection = ClientConnection(sock, ("127.0.0.1", 9))
    connection.outbound.put("hello")
    connection.outbound.put(_SENTINEL)
    server._writer_loop(connection)
    assert sock.sent == [b"hello\n"] and sock.closed


def test_writer_send_error_runs_connection_cleanup(world):
    logger = CaptureLogger()
    server = TapServer(world, logger=logger)
    sock = FakeSocket(fail_send=True, fail_shutdown=True, fail_close=True)
    connection = ClientConnection(sock, ("127.0.0.1", 9))
    connection.outbound.put("lost")
    server._writer_loop(connection)
    assert connection.cleaned
    assert any(record.get("code") == 901 for record in logger.records)


def test_reader_handles_timeout_oserror_eof_and_always_cleans_up(world):
    server = TapServer(world)
    sock = FakeSocket([socket.timeout(), OSError("reset")])
    connection = ClientConnection(sock, ("127.0.0.1", 10))
    server._reader_loop(connection)
    assert connection.cleaned
    assert connection.outbound.get_nowait() == protocol.GREETING
    server._writer_loop(connection)
    assert sock.closed

    sock = FakeSocket([b""])
    connection = ClientConnection(sock, ("127.0.0.1", 11))
    server._reader_loop(connection)
    assert connection.cleaned


def test_raw_handler_ignores_blank_rejects_invalid_utf8_and_cleans_quit(world):
    server = TapServer(world)
    connection = ClientConnection(FakeSocket(), ("127.0.0.1", 1))
    connection.session = QueuedSession(connection.outbound)
    server._handle_raw(connection, b" \t\r")
    assert connection.outbound.empty()
    server._handle_raw(connection, b"\xff")
    assert connection.outbound.get_nowait() == "ERR 400 BAD_REQUEST"
    server._handle_raw(connection, b"CONNECT raw")
    assert connection.username == "raw"
    server._handle_raw(connection, b"QUIT")
    assert connection.cleaned and connection.close_reason == "quit"


def test_overflow_cleanup_and_finish_full_queue_idempotence(world):
    server = TapServer(world)
    sock = FakeSocket(fail_shutdown=True, fail_close=True)
    connection = ClientConnection(sock, ("127.0.0.1", 7),
                                  outbound=queue.Queue(maxsize=1))
    connection.outbound.put("stale event")
    server._overflow(connection)
    deadline = time.monotonic() + 1
    while time.monotonic() < deadline and not connection.cleaned:
        time.sleep(0.005)
    assert connection.cleaned and sock.closed
    assert connection.outbound.get_nowait() is _SENTINEL
    server._finish(connection, "again")
    assert connection.close_reason == "slow_consumer"


def test_finish_uses_saved_username_when_session_is_gone(world):
    server = TapServer(world)
    session = QueuedSession(queue.Queue())
    server.state.connect(session, "saved")
    connection = ClientConnection(FakeSocket(), ("127.0.0.1", 1))
    connection.session = session
    connection.username = "saved"
    server.state.sessions.pop("saved")
    server._finish(connection, "saved_name")
    assert "saved" not in server.state.players
    assert connection.cleaned


def test_finish_without_user_and_socket_close_failure(world):
    server = TapServer(world)
    sock = FakeSocket(fail_shutdown=True, fail_close=True)
    connection = ClientConnection(sock, ("127.0.0.1", 1))
    server._finish(connection, "anonymous", drain=False)
    assert connection.cleaned and sock.closed
    assert connection.outbound.get_nowait() is _SENTINEL


def test_finish_defensive_queue_race_does_not_escape(world):
    class AlwaysFullQueue:
        def __init__(self):
            self.first = True
        def put_nowait(self, _item):
            raise queue.Full
        def get_nowait(self):
            if self.first:
                self.first = False
                return "old"
            raise queue.Empty
    connection = ClientConnection(FakeSocket(), ("127.0.0.1", 1),
                                  outbound=AlwaysFullQueue())
    TapServer(world)._finish(connection)
    assert connection.cleaned


def test_serve_entrypoint_handles_signal_registration_thread_exit_and_interrupt(world, monkeypatch, capsys):
    servers = []
    class FakeServer:
        def __init__(self, world, host, port, logger):
            self.port_actual = port
            self._thread = FakeThread(alive=False)
            self.stops = 0
            servers.append(self)
        def start(self):
            self.port_actual = 4444
            return self.port_actual
        def stop(self):
            self.stops += 1

    monkeypatch.setattr(server_module, "TapServer", FakeServer)
    monkeypatch.setattr(server_module.signal, "signal",
                        lambda *_: (_ for _ in ()).throw(ValueError("not main thread")))
    event = FakeEvent([False])
    monkeypatch.setattr(server_module.threading, "Event", lambda: event)
    server_module.serve(world, host="127.0.0.1", port=0, logger=NullLogger())
    assert servers[-1].stops == 1
    assert "listening" in capsys.readouterr().out

    # An embedding application can receive KeyboardInterrupt while waiting.
    event = FakeEvent([KeyboardInterrupt()])
    monkeypatch.setattr(server_module.threading, "Event", lambda: event)
    server_module.serve(world, logger=NullLogger())
    assert servers[-1].stops == 1


def test_serve_signal_callback_and_missing_listener_thread_paths(world, monkeypatch):
    servers = []
    class FakeServer:
        def __init__(self, *args, **kwargs):
            self.port_actual = 0
            self._thread = None
            self.stops = 0
            servers.append(self)
        def start(self):
            return 4242
        def stop(self):
            self.stops += 1

    callbacks = []
    def register(signum, handler):
        callbacks.append((signum, handler))
        if signum == signal.SIGINT:
            handler(signum, None)
    monkeypatch.setattr(server_module, "TapServer", FakeServer)
    monkeypatch.setattr(server_module.signal, "signal", register)
    event = FakeEvent([True])
    monkeypatch.setattr(server_module.threading, "Event", lambda: event)
    server_module.serve(world, logger=NullLogger())
    assert callbacks and servers[-1].stops == 2
    assert event.set_count == 1


def test_serve_handles_absent_signal_constants_and_shutdown(world, monkeypatch):
    class Signals:
        SIGINT = signal.SIGINT
        def signal(self, *_args):
            return None
    class FakeServer:
        def __init__(self, *args, **kwargs):
            self.port_actual = 0
            self._thread = None
            self.stops = 0
        def start(self):
            return 0
        def stop(self):
            self.stops += 1
    monkeypatch.setattr(server_module, "TapServer", FakeServer)
    monkeypatch.setattr(server_module, "signal", Signals())
    # First wait continues with a missing server thread, second interrupts.
    event = FakeEvent([False, KeyboardInterrupt()])
    monkeypatch.setattr(server_module.threading, "Event", lambda: event)
    server_module.serve(world, logger=NullLogger())


def test_tick_loop_is_joinable_stop_and_start_are_idempotent(world):
    server = TapServer(world, host="127.0.0.1", port=0)
    port = server.start()
    assert server.start() == port
    server.stop()
    server.stop()


def test_nonblocking_enqueue_and_response_overflow_cleanup(world, monkeypatch):
    server = TapServer(world)
    sock = FakeSocket()
    outbound = queue.Queue(maxsize=1)
    outbound.put("stale")
    connection = ClientConnection(sock, ("127.0.0.1", 88), outbound=outbound)
    connection.session = QueuedSession(outbound)
    overflowed = []
    monkeypatch.setattr(server, "_overflow", lambda current: overflowed.append(current))
    assert not server._enqueue(connection, "reply")
    assert overflowed == [connection]
    assert connection.close_reason == "slow_consumer"
    assert connection.session.closed and connection.session.kicked

    connection.cleaned = False
    connection.closed = False
    connection.close_reason = ""
    connection.session = QueuedSession(outbound)
    server._handle_raw(connection, b"WHO")
    assert connection.cleaned
    assert connection.close_reason == "slow_consumer"
    assert outbound.get_nowait() is _SENTINEL


def test_enqueue_overflow_without_session_is_safe(world, monkeypatch):
    server = TapServer(world)
    outbound = queue.Queue(maxsize=1)
    outbound.put("full")
    connection = ClientConnection(FakeSocket(), ("127.0.0.1", 89), outbound=outbound)
    overflowed = []
    monkeypatch.setattr(server, "_overflow", lambda current: overflowed.append(current))
    assert not server._enqueue(connection, "new")
    assert overflowed == [connection]
    assert connection.close_reason == "slow_consumer"
    assert connection.session is None


def test_overflow_preserves_existing_close_reason(world, monkeypatch):
    import server as server_module

    created = []
    class CapturedThread:
        def __init__(self, **kwargs):
            self.kwargs = kwargs
            created.append(self)
        def start(self):
            self.started = True

    monkeypatch.setattr(server_module.threading, "Thread", CapturedThread)
    connection = ClientConnection(FakeSocket(), ("127.0.0.1", 90))
    connection.close_reason = "prior_reason"
    TapServer(world)._overflow(connection)
    assert connection.close_reason == "prior_reason"
    assert created and created[0].started
    assert created[0].kwargs["args"] == (connection, "slow_consumer", False)
