from __future__ import annotations

import io
import os
import pty
import sys
import termios

import client_cli
from client_cli import CliClient
from test_cli_edges import FakeSocket, FakeThread


def test_line_reader_run_skips_prompt_loop_when_already_closed(monkeypatch):
    monkeypatch.setattr(client_cli, "_is_interactive_terminal", lambda: False)
    monkeypatch.setattr(client_cli.threading, "Thread", lambda *a, **k: FakeThread())
    client = CliClient()
    client.sock = FakeSocket()
    client._closed.set()
    client.run(lambda _prompt: (_ for _ in ()).throw(AssertionError("input should be skipped")))
    assert client.sock is None


def test_terminal_loop_skips_select_when_client_is_already_closed(monkeypatch):
    master, slave = pty.openpty()
    old_stdin, old_stdout = sys.stdin, sys.stdout
    class Tty:
        def fileno(self):
            return slave
    sys.stdin, sys.stdout = Tty(), io.StringIO()
    before = termios.tcgetattr(slave)
    client = CliClient()
    client.sock = FakeSocket()
    client._closed.set()
    monkeypatch.setattr(client_cli.select, "select",
                        lambda *_: (_ for _ in ()).throw(AssertionError("select skipped")))
    try:
        client._run_terminal()
        assert client.sock is None
        assert termios.tcgetattr(slave) == before
    finally:
        sys.stdin, sys.stdout = old_stdin, old_stdout
        os.close(master)
        os.close(slave)
