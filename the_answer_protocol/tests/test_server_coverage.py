from __future__ import annotations

import queue
import socket

from server import ClientConnection, QueuedSession, TapServer

from test_server_edges import FakeSocket


def test_queued_session_overflow_without_callback_and_stop_without_listener(world):
    full = queue.Queue(maxsize=1)
    full.put("occupied")
    session = QueuedSession(full)
    session.send("overflow")
    assert session.closed and session.kicked
    server = TapServer(world)
    server.stop()
    server.stop()
    assert server.client_count == 0


def test_stop_closes_failing_listener_and_idempotent_cleanup(world):
    class BadListener:
        def close(self):
            raise OSError("close failed")
    server = TapServer(world)
    server._listener = BadListener()
    server.stop()
    assert server._listener is None


def test_accept_loop_exits_after_running_flag_cleared_on_timeout(world):
    server = TapServer(world)
    class StoppingListener:
        def accept(self):
            server._running.clear()
            raise socket.timeout()
    server._listener = StoppingListener()
    server._running.set()
    server.serve_forever()
    assert not server._running.is_set()


def test_finish_removes_matching_connection_after_skipping_other_clients(world):
    server = TapServer(world)
    target = ClientConnection(FakeSocket(), ("127.0.0.1", 9))
    server._clients[1] = object()
    server._clients[2] = target
    server._finish(target)
    assert 1 in server._clients and 2 not in server._clients
