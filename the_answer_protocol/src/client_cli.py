"""Interactive, raw-pass-through CLI client for RFC 42TAP.

A socket reader never waits for the user to press Enter. On a real POSIX terminal
we multiplex stdin and the socket and redraw the current command line whenever an
asynchronous event arrives, so incoming events do not eat the user's typing. For
pipes, IDE consoles, or platforms without ``termios`` the portable fallback is a
reader thread plus blocking ``input()``.
"""

from __future__ import annotations

import os
import select
import socket
import sys
import threading
from typing import Callable, List, Optional

import protocol

DEFAULT_HOST = "127.0.0.1"
DEFAULT_PORT = 4242
PROMPT = "tap> "


class CliClient:
    """A minimal raw-pass-through client with asynchronous event display."""

    def __init__(self, host: str = DEFAULT_HOST, port: int = DEFAULT_PORT,
                 writer: Optional[Callable[[str], None]] = None,
                 echo: bool = True) -> None:
        self.host = host
        self.port = port
        self._writer = writer or (lambda line: print(line, flush=True))
        self.echo = echo
        self.sock: Optional[socket.socket] = None
        self.received: List[str] = []
        self._buffer = b""
        self._reader: Optional[threading.Thread] = None
        self._closed = threading.Event()
        self.connected = False

    # -- connection ------------------------------------------------------
    def connect(self, timeout: float = 5.0) -> str:
        """Open the socket and return the greeting line."""
        sock = socket.create_connection((self.host, self.port), timeout=timeout)
        sock.settimeout(0.2)
        self.sock = sock
        self.connected = True
        greeting = self.read_one(timeout=timeout)
        if not greeting.startswith("OK hello"):
            self.close()
            raise ConnectionError(f"unexpected greeting: {greeting!r}")
        return greeting

    def close(self) -> None:
        self._closed.set()
        sock, self.sock = self.sock, None
        if sock is not None:
            try:
                sock.shutdown(socket.SHUT_RDWR)
            except OSError:
                pass
            try:
                sock.close()
            except OSError:
                pass
        self.connected = False

    # -- sending ---------------------------------------------------------
    def send_line(self, line: str) -> None:
        """Send one raw command line (the protocol is not translated)."""
        sock = self.sock
        if sock is None:
            raise ConnectionError("not connected")
        sock.sendall(protocol.frame(line))

    def send_and_collect(self, line: str, timeout: float = 2.0) -> str:
        """Send a command, then return the first OK/ERR reply (skip events)."""
        self.send_line(line)
        deadline = _now() + timeout
        while _now() < deadline:
            message = self.read_one(timeout=max(0.01, deadline - _now()))
            if message.startswith(("OK", "ERR ")):
                return message
        return ""

    # -- receiving -------------------------------------------------------
    def read_one(self, timeout: float = 1.0) -> str:
        """Read one line, or ``""`` on timeout/EOF."""
        sock = self.sock  # keep a stable local reference if close() races us
        if sock is None:
            return ""
        deadline = _now() + timeout
        while True:
            while b"\n" in self._buffer:
                raw, self._buffer = self._buffer.split(b"\n", 1)
                line = raw.decode("utf-8", "replace").rstrip("\r")
                self.received.append(line)
                return line
            remaining = deadline - _now()
            if remaining <= 0:
                return ""
            try:
                sock.settimeout(min(0.2, remaining))
                chunk = sock.recv(4096)
            except socket.timeout:
                continue
            except OSError:
                self.connected = False
                return ""
            if not chunk:
                self.connected = False
                return ""
            self._buffer += chunk

    def events(self) -> List[str]:
        """Every EVT or chat line received so far."""
        return [line for line in self.received if _is_event(line)]

    # -- interactive loop ------------------------------------------------
    def run(self, readline: Optional[Callable[[str], str]] = None) -> None:
        """Interactive loop, with terminal redraw and a portable fallback."""
        if self.sock is None:
            self.connect()
        if readline is None and _is_interactive_terminal():
            try:
                self._run_terminal()
                return
            except (ImportError, OSError, ValueError):
                # Some pseudo-terminals do not support termios. If raw mode had
                # already started, _run_terminal closed the socket in its
                # finally block; otherwise the line-oriented fallback is safe.
                if self.sock is None:
                    return
        self._reader = threading.Thread(target=self._reader_loop, name="tap-cli-reader",
                                        daemon=True)
        self._reader.start()
        input_reader = readline or input
        try:
            while not self._closed.is_set():
                try:
                    line = input_reader(PROMPT)
                except (EOFError, KeyboardInterrupt):
                    break
                if line is None:
                    break
                line = line.rstrip("\n")
                if not line.strip():
                    continue
                try:
                    self.send_line(line)
                except OSError:
                    print("connection lost", file=sys.stderr)
                    break
                if line.strip().upper() == "QUIT":
                    break
        finally:
            self.close()
            if (self._reader is not None and self._reader.is_alive()
                    and self._reader is not threading.current_thread()):
                self._reader.join(timeout=0.5)

    def _run_terminal(self) -> None:
        """POSIX raw-terminal loop that redraws the line after asynchronous IO."""
        import termios
        import tty

        sock = self.sock
        if sock is None:
            return
        fd = sys.stdin.fileno()
        old_settings = termios.tcgetattr(fd)
        line_buffer = bytearray()
        try:
            tty.setraw(fd)
            sock.setblocking(False)
            sys.stdout.write(PROMPT)
            sys.stdout.flush()
            while not self._closed.is_set():
                ready, _, _ = select.select([sock, fd], [], [])
                if sock in ready:
                    if not self._drain_terminal_socket(sock, line_buffer):
                        break
                if fd in ready:
                    char = os.read(fd, 1)
                    if not char or char == b"\x03":
                        break
                    if char == b"\x04" and not line_buffer:
                        break
                    if char in (b"\r", b"\n"):
                        sys.stdout.write("\r\n")
                        sys.stdout.flush()
                        command = line_buffer.decode("utf-8", "replace")
                        line_buffer.clear()
                        if command.strip():
                            try:
                                self.send_line(command)
                            except OSError:
                                break
                            if command.strip().upper() == "QUIT":
                                break
                        sys.stdout.write(PROMPT)
                        sys.stdout.flush()
                    elif char in (b"\x7f", b"\x08"):
                        if line_buffer:
                            line_buffer.pop()
                            sys.stdout.write("\b \b")
                            sys.stdout.flush()
                    elif char >= b" " and char != b"\x7f":
                        line_buffer.extend(char)
                        if self.echo:
                            sys.stdout.write(char.decode("utf-8", "replace"))
                            sys.stdout.flush()
        finally:
            termios.tcsetattr(fd, termios.TCSADRAIN, old_settings)
            self.close()

    def _drain_terminal_socket(self, sock: socket.socket,
                               line_buffer: bytearray) -> bool:
        """Consume socket data and redraw the prompt; return false on EOF."""
        try:
            chunk = sock.recv(4096)
        except BlockingIOError:
            return True
        except OSError:
            self.connected = False
            return False
        if not chunk:
            self.connected = False
            return False
        self._buffer += chunk
        while b"\n" in self._buffer:
            raw, self._buffer = self._buffer.split(b"\n", 1)
            message = raw.decode("utf-8", "replace").rstrip("\r")
            self.received.append(message)
            sys.stdout.write("\r\x1b[2K")
            self._writer(message)
            sys.stdout.write(PROMPT + line_buffer.decode("utf-8", "replace"))
            sys.stdout.flush()
        return True

    def _reader_loop(self) -> None:
        while not self._closed.is_set():
            message = self.read_one(timeout=0.2)
            if message:
                self._writer(message)
                if message.startswith("ERR 902"):
                    break


def _is_event(line: str) -> bool:
    return line.startswith("EVT ") or (line.startswith("(") and ")" in line)


def _is_interactive_terminal() -> bool:
    return bool(sys.stdin.isatty() and sys.stdout.isatty() and os.name == "posix")


def _now() -> float:
    import time

    return time.monotonic()


def format_message(line: str) -> str:
    """Human-friendly rendering; the original line stays available for scripts."""
    message = protocol.parse_message(line)
    if message.kind == "err":
        return f"[error {message.code}] {message.symbol.lower().replace('_', ' ')}"
    if message.kind == "evt":
        return f"[{message.scope.lower()}] {' '.join(message.data)}"
    if message.kind == "chat":
        return f"[{message.scope.lower()}] {message.payload}"
    return message.payload


def run_client(host: str = DEFAULT_HOST, port: int = DEFAULT_PORT) -> int:
    """Entry point used by ``tap.py client``."""
    client = CliClient(host=host, port=port)
    try:
        greeting = client.connect()
    except OSError as exc:
        print(f"cannot reach {host}:{port}: {exc}", file=sys.stderr)
        return 1
    print(greeting)
    print("Type CONNECT <username>, then HELP for the command list. QUIT to leave.")
    client.run()
    return 0
