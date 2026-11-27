"""Threaded TCP server for RFC 42TAP.

The state machine is socket-free; each accepted client receives a ``Session``
adapter that puts outgoing lines on its own bounded queue. A dedicated writer
thread drains that queue. Consequently, a slow peer never blocks a game-state
broadcast or another client's socket.
"""

from __future__ import annotations

import queue
import signal
import socket
import threading
import time
from dataclasses import dataclass, field
from typing import Any, Dict, List, Optional

import protocol
from logging_json import JsonLogger, NullLogger
from state import GameState, Session
from world import World

DEFAULT_HOST = "127.0.0.1"
DEFAULT_PORT = 4242
MAX_CONNECTIONS = 128
RAPID_CONNECTIONS = 10
RAPID_WINDOW_S = 10.0
OUTBOUND_QUEUE_SIZE = 512
TICK_INTERVAL_S = 0.1
_SENTINEL = object()


class QueuedSession(Session):
    """A production Session adapter with a bounded, non-blocking output queue."""

    def __init__(self, outbound: "queue.Queue", clock=None,
                 on_overflow=None) -> None:
        super().__init__(clock=clock or time.monotonic)
        self.outbound = outbound
        self.on_overflow = on_overflow
        self.ip = ""

    def send(self, message: str) -> None:
        if self.closed:
            return
        try:
            self.outbound.put_nowait(message)
        except queue.Full:
            self.kicked = True
            self.closed = True
            if self.on_overflow is not None:
                self.on_overflow()


@dataclass
class ClientConnection:
    """One accepted client and its reader/writer workers."""

    sock: socket.socket
    address: tuple
    outbound: "queue.Queue" = field(
        default_factory=lambda: queue.Queue(maxsize=OUTBOUND_QUEUE_SIZE)
    )
    session: Optional[QueuedSession] = None
    username: str = ""
    closed: bool = False
    cleaned: bool = False
    close_reason: str = ""
    writer: Optional[threading.Thread] = None
    reader: Optional[threading.Thread] = None


class TapServer:
    """Concurrent TCP server with an authoritative shared :class:`GameState`."""

    def __init__(self, world: World, host: str = DEFAULT_HOST, port: int = DEFAULT_PORT,
                 logger: Optional[Any] = None) -> None:
        self.world = world
        self.host = host
        self.port = port
        self.logger = logger if logger is not None else NullLogger()
        self.state = GameState(world, logger=self.logger)
        self._listener: Optional[socket.socket] = None
        self._thread: Optional[threading.Thread] = None
        self._ticker: Optional[threading.Thread] = None
        self._running = threading.Event()
        self._ticker_stop = threading.Event()
        self._clients: Dict[int, ClientConnection] = {}
        self._lock = threading.RLock()
        self._connect_history: Dict[str, List[float]] = {}
        self._next_id = 1
        self.port_actual = port
        self._stopping = False

    # -- lifecycle -------------------------------------------------------
    def bind(self) -> int:
        """Bind the listening socket and return the actual port."""
        if self._listener is not None:
            return self.port_actual
        listener = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        listener.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        listener.bind((self.host, self.port))
        listener.listen(64)
        listener.settimeout(0.2)
        self._listener = listener
        self.port_actual = int(listener.getsockname()[1])
        return self.port_actual

    def start(self) -> int:
        """Start accepting in the background; return the bound TCP port."""
        if self._running.is_set():
            return self.port_actual
        self.bind()
        self._stopping = False
        self._ticker_stop.clear()
        self._running.set()
        self._thread = threading.Thread(target=self.serve_forever, name="tap-listener",
                                        daemon=True)
        self._ticker = threading.Thread(target=self._tick_loop, name="tap-timers",
                                        daemon=True)
        self._thread.start()
        self._ticker.start()
        self.logger.log("INFO", "listening", type="startup", host=self.host,
                        port=self.port_actual, world=self.world.stats())
        return self.port_actual

    def serve_forever(self) -> None:
        """Run the accept loop. Normally called by :meth:`start`."""
        listener = self._listener
        if listener is None:
            raise RuntimeError("call bind() before serve_forever()")
        while self._running.is_set():
            try:
                sock, address = listener.accept()
            except socket.timeout:
                continue
            except OSError:
                break
            if self._stopping or not self._admit(address):
                self._refuse(sock, address)
                continue
            self._spawn(sock, address)
        self.logger.log("INFO", "accept loop ended", type="shutdown")

    def stop(self, timeout: float = 2.0) -> None:
        """Stop accepting and cleanly remove all connected players."""
        if self._stopping:
            return
        self._stopping = True
        self._running.clear()
        self._ticker_stop.set()
        listener, self._listener = self._listener, None
        if listener is not None:
            try:
                listener.close()
            except OSError:
                pass
        with self._lock:
            connections = list(self._clients.values())
        for connection in connections:
            self._finish(connection, "server_stop", drain=False)
        current = threading.current_thread()
        for worker in (self._thread, self._ticker):
            if worker is not None and worker is not current and worker.is_alive():
                worker.join(timeout)
        self.logger.log("INFO", "server stopped", type="shutdown")

    def _tick_loop(self) -> None:
        while not self._ticker_stop.wait(TICK_INTERVAL_S):
            try:
                self.state.tick()
            except Exception as exc:  # one bad timer must not kill the listener
                self.logger.log("ERROR", "timer tick failed", type="timer",
                                error=f"{type(exc).__name__}: {exc}")

    # -- admission control -----------------------------------------------
    def _admit(self, address: tuple) -> bool:
        ip = str(address[0])
        now = time.monotonic()
        with self._lock:
            if len(self._clients) >= MAX_CONNECTIONS:
                self.logger.log("WARN", "server full", type="abuse", ip=ip,
                                reason="server_full")
                return False
            history = [stamp for stamp in self._connect_history.get(ip, [])
                       if now - stamp < RAPID_WINDOW_S]
            if len(history) >= RAPID_CONNECTIONS:
                self._connect_history[ip] = history
                self.logger.log("WARN", "rapid connections", type="abuse", ip=ip,
                                reason="rapid_connections")
                return False
            history.append(now)
            self._connect_history[ip] = history
        return True

    def _refuse(self, sock: socket.socket, address: tuple) -> None:
        try:
            sock.sendall(protocol.frame(protocol.err("CONNECTION_FAILED")))
        except OSError:
            pass
        finally:
            try:
                sock.close()
            except OSError:
                pass

    # -- client workers --------------------------------------------------
    def _spawn(self, sock: socket.socket, address: tuple) -> None:
        sock.settimeout(0.2)
        connection = ClientConnection(sock=sock, address=address)
        connection.session = QueuedSession(
            connection.outbound,
            on_overflow=lambda: self._overflow(connection),
        )
        connection.session.ip = str(address[0])
        with self._lock:
            client_id = self._next_id
            self._next_id += 1
            self._clients[client_id] = connection
        self.logger.log("INFO", "client connected", type="connection", ip=address[0],
                        remote=f"{address[0]}:{address[1]}")
        connection.writer = threading.Thread(
            target=self._writer_loop, args=(connection,),
            name=f"tap-writer-{client_id}", daemon=True,
        )
        connection.reader = threading.Thread(
            target=self._reader_loop, args=(connection,),
            name=f"tap-reader-{client_id}", daemon=True,
        )
        connection.writer.start()
        connection.reader.start()

    def _writer_loop(self, connection: ClientConnection) -> None:
        """Serialize this client's queue as UTF-8/LF frames."""
        while True:
            item = connection.outbound.get()
            if item is _SENTINEL:
                self._close_socket(connection)
                return
            try:
                connection.sock.sendall(protocol.frame(str(item)))
            except OSError:
                self.logger.log("ERROR", "send failed", type="network",
                                player=connection.username or "-",
                                ip=connection.address[0], code=901)
                self._finish(connection, "send_failed", drain=False)
                return

    def _reader_loop(self, connection: ClientConnection) -> None:
        """Read one bounded line at a time; handle fragmentation/coalescing."""
        self._enqueue(connection, protocol.GREETING)
        buffer = bytearray()
        discarding = False
        try:
            while not connection.closed:
                try:
                    chunk = connection.sock.recv(4096)
                except socket.timeout:
                    continue
                except OSError:
                    break
                if not chunk:
                    break
                remaining = chunk
                while remaining and not connection.closed:
                    if discarding:
                        newline = remaining.find(b"\n")
                        if newline < 0:
                            remaining = b""
                            continue
                        remaining = remaining[newline + 1:]
                        discarding = False
                        continue

                    newline = remaining.find(b"\n")
                    if newline >= 0:
                        piece = remaining[:newline]
                        if len(buffer) + len(piece) > protocol.MAX_LINE_BYTES:
                            self._enqueue(connection, protocol.err("LINE_TOO_LONG"))
                        else:
                            buffer.extend(piece)
                            self._handle_raw(connection, bytes(buffer))
                        buffer.clear()
                        remaining = remaining[newline + 1:]
                        continue

                    if len(buffer) + len(remaining) > protocol.MAX_LINE_BYTES:
                        self._enqueue(connection, protocol.err("LINE_TOO_LONG"))
                        buffer.clear()
                        discarding = True
                    else:
                        buffer.extend(remaining)
                    remaining = b""
        finally:
            reason = connection.close_reason or "eof"
            self._finish(connection, reason, drain=True)

    def _handle_raw(self, connection: ClientConnection, raw: bytes) -> None:
        try:
            line = raw.decode("utf-8")
        except UnicodeDecodeError:
            self._enqueue(connection, protocol.err("BAD_REQUEST"))
            return
        line = protocol.trim_line(line)
        if not line.strip():
            return

        session = connection.session
        assert session is not None
        reply = self.state.dispatch(session, line)
        username = self.state.name_of(session)
        if username:
            connection.username = username
        if reply is not None:
            self._enqueue(connection, reply)

        if session.kicked or session.closed:
            connection.close_reason = (
                connection.close_reason or ("flooding" if session.kicked else "quit")
            )
            self._finish(connection, connection.close_reason, drain=True)

    def _enqueue(self, connection: ClientConnection, message: str) -> bool:
        """Queue one server-owned line without ever blocking a reader/broadcaster."""
        try:
            connection.outbound.put_nowait(message)
            return True
        except queue.Full:
            connection.close_reason = "slow_consumer"
            session = connection.session
            if session is not None:
                session.kicked = True
                session.closed = True
            self._overflow(connection)
            return False

    def _overflow(self, connection: ClientConnection) -> None:
        # Do not call back into GameState while its dispatch lock is held.
        if not connection.close_reason:
            connection.close_reason = "slow_consumer"
        threading.Thread(target=self._finish,
                         args=(connection, "slow_consumer", False),
                         name="tap-overflow-cleanup", daemon=True).start()

    # -- cleanup ---------------------------------------------------------
    def _finish(self, connection: ClientConnection, reason: str = "eof",
                drain: bool = True) -> None:
        """Idempotently remove a player, queue a writer sentinel and close."""
        with self._lock:
            if connection.cleaned:
                return
            connection.cleaned = True
            connection.closed = True
            connection.close_reason = reason
            for client_id, candidate in list(self._clients.items()):
                if candidate is connection:
                    self._clients.pop(client_id, None)
                    break
        session = connection.session
        username = self.state.name_of(session) if session is not None else ""
        username = username or connection.username
        if session is not None:
            session.closed = True
        if username:
            self.logger.log("INFO", "client disconnected", type="connection",
                            player=username, ip=connection.address[0], reason=reason)
            self.state.disconnect(username, reason=reason)

        # If the queue is full, discard stale events to ensure shutdown cannot
        # deadlock. The writer remains the only code that touches sendall().
        try:
            connection.outbound.put_nowait(_SENTINEL)
        except queue.Full:
            while True:
                try:
                    connection.outbound.get_nowait()
                except queue.Empty:
                    break
            try:
                connection.outbound.put_nowait(_SENTINEL)
            except queue.Full:  # defensive: another producer raced the cleanup
                pass
        if not drain:
            try:
                connection.sock.shutdown(socket.SHUT_RDWR)
            except OSError:
                pass
            self._close_socket(connection)

    @staticmethod
    def _close_socket(connection: ClientConnection) -> None:
        try:
            connection.sock.close()
        except OSError:
            pass

    @property
    def client_count(self) -> int:
        with self._lock:
            return len(self._clients)


def create_server(world: World, host: str = DEFAULT_HOST, port: int = DEFAULT_PORT,
                  logger: Optional[Any] = None) -> TapServer:
    """Construct a server without binding it (useful for tests)."""
    return TapServer(world=world, host=host, port=port, logger=logger)


def serve(world: World, host: str = DEFAULT_HOST, port: int = DEFAULT_PORT,
          logger: Optional[Any] = None) -> None:
    """Blocking server entry point with SIGINT/SIGTERM cleanup."""
    json_logger = logger if logger is not None else JsonLogger()
    server = TapServer(world=world, host=host, port=port, logger=json_logger)
    server.start()
    print(f"TAP server listening on {host}:{server.port_actual}", flush=True)
    stop_requested = threading.Event()

    def handle_signal(signum, frame):  # noqa: ARG001
        stop_requested.set()
        server.stop()

    for signal_name in ("SIGINT", "SIGTERM"):
        signal_number = getattr(signal, signal_name, None)
        if signal_number is not None:
            try:
                signal.signal(signal_number, handle_signal)
            except (ValueError, OSError):
                pass
    try:
        while not stop_requested.wait(0.5):
            if server._thread is not None and not server._thread.is_alive():
                break
    except KeyboardInterrupt:
        pass
    finally:
        server.stop()
