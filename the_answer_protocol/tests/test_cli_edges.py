from __future__ import annotations

import io
import os
import pty
import socket
import sys
import termios

import pytest

import client_cli
from client_cli import CliClient, format_message


class FakeSocket:
    def __init__(self, incoming=(), fail_send=False, fail_shutdown=False, fail_close=False):
        self.incoming = list(incoming)
        self.sent = []
        self.fail_send = fail_send
        self.fail_shutdown = fail_shutdown
        self.fail_close = fail_close
        self.closed = False

    def settimeout(self, _timeout):
        pass

    def setblocking(self, _blocking):
        pass

    def sendall(self, data):
        if self.fail_send:
            raise OSError("send failed")
        self.sent.append(data)

    def recv(self, _size):
        if not self.incoming:
            return b""
        result = self.incoming.pop(0)
        if isinstance(result, BaseException):
            raise result
        return result

    def shutdown(self, _how):
        if self.fail_shutdown:
            raise OSError("already shut down")

    def close(self):
        self.closed = True
        if self.fail_close:
            raise OSError("already closed")


class FakeThread:
    def __init__(self, *args, alive=False, **kwargs):
        self.alive = alive
        self.started = False
        self.joined = False

    def start(self):
        self.started = True

    def is_alive(self):
        return self.alive

    def join(self, timeout=None):
        self.joined = True


class FakeInput:
    def __init__(self, fd):
        self._fd = fd

    def fileno(self):
        return self._fd

    def isatty(self):
        return True


class FakeOutput(io.StringIO):
    def isatty(self):
        return True


def test_connect_accepts_greeting_and_rejects_unexpected_greeting(monkeypatch):
    peers = []

    def create_connection(_address, timeout):
        sock = FakeSocket([b"OK hello proto=1\r\n"])
        peers.append((sock, timeout))
        return sock

    monkeypatch.setattr(client_cli.socket, "create_connection", create_connection)
    client = CliClient("example", 4243)
    assert client.connect(timeout=1) == "OK hello proto=1"
    assert client.connected and peers[0][1] == 1
    client.close()

    bad = FakeSocket([b"WELCOME\n"], fail_shutdown=True, fail_close=True)
    monkeypatch.setattr(client_cli.socket, "create_connection", lambda *_args, **_kwargs: bad)
    client = CliClient()
    with pytest.raises(ConnectionError, match="unexpected greeting"):
        client.connect()
    assert bad.closed and client.sock is None and not client.connected


def test_connect_socket_creation_failure_and_close_without_socket(monkeypatch):
    monkeypatch.setattr(client_cli.socket, "create_connection",
                        lambda *_a, **_k: (_ for _ in ()).throw(OSError("refused")))
    with pytest.raises(OSError, match="refused"):
        CliClient().connect()
    client = CliClient()
    client.close()
    assert client.sock is None and not client.connected
    with pytest.raises(ConnectionError, match="not connected"):
        client.send_line("WHO")


def test_send_read_buffer_timeouts_errors_and_events():
    sock = FakeSocket([socket.timeout(), b"EVT ROOM PRESENCE ENTER Paul\r\n"
                                      b"(GLOBAL) Paul: Hi\nOK done\n"])
    client = CliClient()
    client.sock = sock
    client.connected = True
    client.send_line("connect Paul")
    assert sock.sent == [b"connect Paul\n"]
    assert client.read_one(timeout=0.5) == "EVT ROOM PRESENCE ENTER Paul"
    assert client.read_one(timeout=0.1) == "(GLOBAL) Paul: Hi"
    assert client.read_one(timeout=0.1) == "OK done"
    assert client.events() == ["EVT ROOM PRESENCE ENTER Paul", "(GLOBAL) Paul: Hi"]

    assert CliClient().read_one(0.01) == ""
    timeout = CliClient()
    timeout.sock = FakeSocket([socket.timeout()])
    assert timeout.read_one(0.01) == ""
    broken = CliClient()
    broken.sock = FakeSocket([OSError("reset")])
    broken.connected = True
    assert broken.read_one(1) == "" and not broken.connected
    eof = CliClient()
    eof.sock = FakeSocket([b""])
    eof.connected = True
    assert eof.read_one(1) == "" and not eof.connected


def test_send_and_collect_skips_events_and_returns_empty_on_timeout():
    sock = FakeSocket([b"EVT STATS players=1\nERR 404 NPC_NOT_FOUND\n"])
    client = CliClient()
    client.sock = sock
    assert client.send_and_collect("ATTACK absent") == "ERR 404 NPC_NOT_FOUND"
    assert sock.sent == [b"ATTACK absent\n"]
    no_reply = CliClient()
    no_reply.sock = FakeSocket([])
    assert no_reply.send_and_collect("WHO", timeout=0) == ""


def test_close_and_send_handle_socket_os_errors():
    sock = FakeSocket(fail_send=True, fail_shutdown=True, fail_close=True)
    client = CliClient()
    client.sock = sock
    with pytest.raises(OSError, match="send failed"):
        client.send_line("WHO")
    client.close()
    assert sock.closed and not client.connected


def test_reader_loop_writes_messages_and_stops_after_flood_error():
    messages = ["EVT STATS players=0", "ERR 902 FLOODING"]
    writes = []
    client = CliClient(writer=writes.append)
    client.read_one = lambda timeout=1: messages.pop(0) if messages else ""
    client._reader_loop()
    assert writes == ["EVT STATS players=0", "ERR 902 FLOODING"]


def test_cli_fallback_run_handles_empty_none_eof_interrupt_send_error_and_join(monkeypatch, capsys):
    threads = []

    def fake_thread(*args, **kwargs):
        thread = FakeThread(*args, alive=True, **kwargs)
        threads.append(thread)
        return thread

    monkeypatch.setattr(client_cli.threading, "Thread", fake_thread)
    client = CliClient()
    sock = FakeSocket(fail_shutdown=True, fail_close=True)
    client.sock = sock
    commands = iter(["", "WHO\n", "QUIT"])
    client.run(lambda _prompt: next(commands))
    assert sock.sent == [b"WHO\n", b"QUIT\n"]
    assert threads[-1].started and threads[-1].joined

    client = CliClient()
    client.sock = FakeSocket()
    client.run(lambda _prompt: None)
    assert threads[-1].joined

    client = CliClient()
    client.sock = FakeSocket()
    client.run(lambda _prompt: (_ for _ in ()).throw(EOFError()))
    client = CliClient()
    client.sock = FakeSocket()
    client.run(lambda _prompt: (_ for _ in ()).throw(KeyboardInterrupt()))

    client = CliClient()
    client.sock = FakeSocket(fail_send=True)
    answers = iter(["WHO"])
    client.run(lambda _prompt: next(answers))
    assert "connection lost" in capsys.readouterr().err


def test_cli_run_connects_when_needed_and_uses_builtin_input(monkeypatch):
    monkeypatch.setattr(client_cli, "_is_interactive_terminal", lambda: False)
    monkeypatch.setattr(client_cli.threading, "Thread", lambda *a, **k: FakeThread())
    client = CliClient()
    sock = FakeSocket()
    client.connect = lambda: setattr(client, "sock", sock) or "OK hello proto=1"
    answers = iter(["WHO", "QUIT"])
    monkeypatch.setattr("builtins.input", lambda _prompt: next(answers))
    client.run()
    assert sock.sent == [b"WHO\n", b"QUIT\n"]


def _drive_terminal(monkeypatch, chars, *, echo=True, socket_ready_first=False):
    master, slave = pty.openpty()
    sock, peer = socket.socketpair()
    peer.settimeout(0.5)
    output = FakeOutput()
    writes = []
    client = CliClient(writer=writes.append, echo=echo)
    client.sock = sock
    client.connected = True
    old_stdin, old_stdout = sys.stdin, sys.stdout
    sys.stdin, sys.stdout = FakeInput(slave), output
    old_term = termios.tcgetattr(slave)
    keys = iter(chars)
    select_first = [socket_ready_first]

    def fake_select(readers, _writers, _errors):
        if select_first[0]:
            select_first[0] = False
            return [sock], [], []
        return [slave], [], []

    monkeypatch.setattr(client_cli.select, "select", fake_select)
    monkeypatch.setattr(client_cli.os, "read", lambda _fd, _size: next(keys))
    if socket_ready_first:
        peer.sendall(b"EVT STATS players=2\r\nOK event\n")
    try:
        client._run_terminal()
        after_term = termios.tcgetattr(slave)
        try:
            sent = peer.recv(4096)
        except socket.timeout:
            sent = b""
    finally:
        sys.stdin, sys.stdout = old_stdin, old_stdout
        peer.close()
        os.close(master)
        os.close(slave)
    assert after_term == old_term
    return client, output.getvalue(), writes, sent


def test_terminal_multiplexes_events_edits_backspace_and_sends_raw_lines(monkeypatch):
    client, output, writes, sent = _drive_terminal(
        monkeypatch,
        [b"\x7f", b"A", b"B", b"\x7f", b"C", b"\r", b"\n",
         b"Q", b"U", b"I", b"T", b"\n"],
        socket_ready_first=True,
    )
    assert sent == b"AC\nQUIT\n"
    assert writes == ["EVT STATS players=2", "OK event"]
    assert "AB\b \bC" in output and client.sock is None


def test_terminal_handles_empty_input_ctrl_c_and_ctrl_d(monkeypatch):
    for terminal_key in (b"", b"\x03", b"\x04"):
        client, _, _, sent = _drive_terminal(monkeypatch, [terminal_key], echo=False)
        assert sent == b"" and client.sock is None
    # Ctrl-D is ignored when there is buffered text; Ctrl-C then exits.
    client, _, _, sent = _drive_terminal(
        monkeypatch, [b"X", b"\x04", b"\x03"], echo=False
    )
    assert sent == b"" and client.sock is None


def test_terminal_falls_back_cleanly_if_raw_setup_fails(monkeypatch):
    client = CliClient()
    client.sock = FakeSocket()
    monkeypatch.setattr(client_cli, "_is_interactive_terminal", lambda: True)
    monkeypatch.setattr(client, "_run_terminal", lambda: (_ for _ in ()).throw(ValueError("no tty")))
    monkeypatch.setattr(client_cli.threading, "Thread", lambda *a, **k: FakeThread())
    client.run(lambda _prompt: None)
    assert client.sock is None

    client = CliClient()
    client.sock = FakeSocket()
    monkeypatch.setattr(client, "_run_terminal", lambda: (_ for _ in ()).throw(OSError("raw mode")))
    monkeypatch.setattr(client, "close", lambda: setattr(client, "sock", None))
    client.run(lambda _prompt: None)
    assert client.sock is None


def test_terminal_drain_handles_blocking_error_eof_and_bad_socket():
    client = CliClient()
    assert client._drain_terminal_socket(FakeSocket([BlockingIOError()]), bytearray())
    assert not client._drain_terminal_socket(FakeSocket([OSError("closed")]), bytearray())
    assert not client._drain_terminal_socket(FakeSocket([b""]), bytearray())


def test_interactive_terminal_detection_and_message_fallback(monkeypatch):
    class NotTty:
        def isatty(self):
            return False
    monkeypatch.setattr(client_cli.sys, "stdin", NotTty())
    monkeypatch.setattr(client_cli.sys, "stdout", NotTty())
    assert not client_cli._is_interactive_terminal()
    assert format_message("OK") == ""
    assert format_message("unparsed server output") == "unparsed server output"
