from __future__ import annotations

import os
import pty
import socket
import sys

import client_cli
from client_cli import CliClient, run_client

from test_cli_edges import FakeSocket, FakeThread


def test_read_one_zero_timeout_hits_deadline_without_recv():
    client = CliClient()
    client.sock = FakeSocket([b"never read"])
    assert client.read_one(timeout=0) == ""
    assert client.sock.incoming == [b"never read"]


def test_run_selects_terminal_when_available_and_returns(monkeypatch):
    client = CliClient()
    client.sock = FakeSocket()
    calls = []
    monkeypatch.setattr(client_cli, "_is_interactive_terminal", lambda: True)
    monkeypatch.setattr(client, "_run_terminal", lambda: calls.append("terminal"))
    client.run()
    assert calls == ["terminal"]
    assert client.sock is not None


def test_run_terminal_setup_failure_falls_back_only_before_socket_closed(monkeypatch):
    monkeypatch.setattr(client_cli, "_is_interactive_terminal", lambda: True)
    monkeypatch.setattr(client_cli.threading, "Thread", lambda *a, **k: FakeThread())
    monkeypatch.setattr("builtins.input", lambda _prompt: None)

    client = CliClient()
    client.sock = FakeSocket()
    monkeypatch.setattr(client, "_run_terminal",
                        lambda: (_ for _ in ()).throw(ValueError("tcgetattr")))
    client.run()
    assert client.sock is None

    client = CliClient()
    client.sock = FakeSocket()
    def close_then_fail():
        client.sock = None
        raise OSError("raw mode failed")
    monkeypatch.setattr(client, "_run_terminal", close_then_fail)
    client.run()
    assert client.sock is None


def test_terminal_returns_without_socket_and_breaks_if_socket_eof(monkeypatch):
    client = CliClient()
    client._run_terminal()
    assert client.sock is None

    master, slave = pty.openpty()
    sock, peer = socket.socketpair()
    old_stdin = sys.stdin
    class Tty:
        def fileno(self):
            return slave
    sys.stdin = Tty()
    monkeypatch.setattr(client_cli.select, "select", lambda *_: ([sock], [], []))
    client.sock = sock
    peer.close()
    try:
        client._run_terminal()
        assert client.sock is None and not client.connected
    finally:
        sys.stdin = old_stdin
        os.close(master)
        os.close(slave)


def test_terminal_send_error_closes_without_prompting_again(monkeypatch):
    master, slave = pty.openpty()
    old_stdin = sys.stdin
    class Tty:
        def fileno(self):
            return slave
    sys.stdin = Tty()
    chars = iter([b"W", b"\r"])
    monkeypatch.setattr(client_cli.select, "select", lambda *_: ([slave], [], []))
    monkeypatch.setattr(client_cli.os, "read", lambda *_: next(chars))
    client = CliClient()
    client.sock = FakeSocket()
    client.send_line = lambda _line: (_ for _ in ()).throw(OSError("peer closed"))
    try:
        client._run_terminal()
        assert client.sock is None
    finally:
        sys.stdin = old_stdin
        os.close(master)
        os.close(slave)


def test_run_client_success_uses_selected_cli_and_returns_zero(monkeypatch, capsys):
    calls = []
    class FakeClient:
        def __init__(self, host, port):
            calls.append((host, port))
        def connect(self):
            return "OK hello proto=1"
        def run(self):
            calls.append("run")
    monkeypatch.setattr(client_cli, "CliClient", FakeClient)
    assert run_client("tap.example", 4321) == 0
    assert calls == [("tap.example", 4321), "run"]
    assert "OK hello proto=1" in capsys.readouterr().out
