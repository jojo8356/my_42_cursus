from __future__ import annotations

import protocol
from server import ClientConnection, QueuedSession, TapServer
from test_server_edges import FakeSocket


def test_tcp_reader_drains_oversized_partial_lines_and_recovers(world):
    server = TapServer(world)
    sock = FakeSocket([b"x" * (protocol.MAX_LINE_BYTES + 1), b"more data",
                       b"still discarded\nCONNECT recover\n", b""])
    connection = ClientConnection(sock, ("127.0.0.1", 99))
    connection.session = QueuedSession(connection.outbound)
    server._reader_loop(connection)
    lines = []
    while not connection.outbound.empty():
        lines.append(connection.outbound.get_nowait())
    assert lines == [protocol.GREETING, "ERR 413 LINE_TOO_LONG", "EVT STATS players=1",
                     "OK Connected", server_module_sentinel()]
    assert connection.username == "recover"


def server_module_sentinel():
    from server import _SENTINEL
    return _SENTINEL


def test_tcp_reader_rejects_complete_oversized_line_and_finishes(world):
    server = TapServer(world)
    sock = FakeSocket([b"x" * (protocol.MAX_LINE_BYTES + 1) + b"\n", b""])
    connection = ClientConnection(sock, ("127.0.0.1", 100))
    server._reader_loop(connection)
    assert connection.outbound.get_nowait() == protocol.GREETING
    assert connection.outbound.get_nowait() == "ERR 413 LINE_TOO_LONG"
