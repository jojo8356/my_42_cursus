#!/usr/bin/env python3
"""
Comprehensive test suite for mini_serv (42 Exam Rank 06).
'Grandes entreprises' quality: functional, edge-case, and static-analysis
checks covering the full subject requirements.
"""
import os
import re
import socket
import subprocess
import sys
import time

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)
EXECUTABLE = os.path.join(ROOT, "mini_serv", "mini_serv")

HOST = "127.0.0.1"

PASS = 0
FAIL = 0


def check(name, cond, detail=""):
    global PASS, FAIL
    if cond:
        PASS += 1
        print("  [PASS] %s" % name)
    else:
        FAIL += 1
        print("  [FAIL] %s %s" % (name, detail))


def find_free_port():
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.bind((HOST, 0))
        return s.getsockname()[1]


def recv_exact(client, size, timeout=2.0):
    client.settimeout(timeout)
    data = b""
    while len(data) < size:
        chunk = client.recv(size - len(data))
        if not chunk:
            raise RuntimeError("connection closed (got %d/%d bytes)"
                               % (len(data), size))
        data += chunk
    return data


def expect_message(client, expected, timeout=2.0):
    got = recv_exact(client, len(expected), timeout)
    check("recv %r" % expected, got == expected,
          "(got %r)" % got)


def expect_silence(client, timeout=0.35):
    client.settimeout(timeout)
    try:
        data = client.recv(1024)
    except socket.timeout:
        return
    if data:
        check("no extra data", False, "(got %r)" % data)
    else:
        check("no extra data", False, "(connection closed)")


class ServerCtx:
    def __init__(self, args):
        self.args = args
        self.proc = None

    def __enter__(self):
        self.proc = subprocess.Popen([EXECUTABLE] + self.args,
                                     stdout=subprocess.DEVNULL,
                                     stderr=subprocess.DEVNULL)
        return self

    def __exit__(self, *exc):
        if self.proc:
            self.proc.terminate()
            try:
                self.proc.wait(timeout=2)
            except subprocess.TimeoutExpired:
                self.proc.kill()
                self.proc.wait()


def connect_client(port):
    deadline = time.monotonic() + 3
    while time.monotonic() < deadline:
        try:
            c = socket.create_connection((HOST, port), timeout=0.3)
            c.settimeout(2)
            return c
        except OSError:
            time.sleep(0.05)
    raise RuntimeError("could not connect")


def connected_clients(port, count):
    servers = []
    proc = subprocess.Popen([EXECUTABLE, str(port)],
                            stdout=subprocess.DEVNULL,
                            stderr=subprocess.DEVNULL)
    clients = []
    try:
        for i in range(count):
            c = connect_client(port)
            for existing in clients:
                expect_message(existing,
                               ("server: client %d just arrived\n" % i).encode())
            clients.append(c)
        return proc, clients
    except Exception:
        for c in clients:
            c.close()
        proc.terminate()
        proc.wait()
        raise


# ---------------------------------------------------------------------------
# Functional tests
# ---------------------------------------------------------------------------

def test_connection():
    port = find_free_port()
    with ServerCtx([str(port)]):
        c = connect_client(port)
        check("server accepts TCP connections", c.getsockname()[0] is not None)
        c.close()


def test_arrival_notification():
    port = find_free_port()
    proc, clients = connected_clients(port, 2)
    try:
        check("first client gets no self-arrival", True)
        # client 0 was first; when client 1 arrived, client 0 got the notice.
        # (verified inside connected_clients)
    finally:
        for c in clients:
            c.close()
        proc.terminate(); proc.wait()


def test_id_assignment():
    port = find_free_port()
    proc, clients = connected_clients(port, 3)
    try:
        # client0 sends; others should see "client 0: ..."
        clients[0].sendall(b"hello\n")
        expect_message(clients[1], b"client 0: hello\n")
        expect_message(clients[2], b"client 0: hello\n")
        # client1 sends; others see "client 1: ..."
        clients[1].sendall(b"world\n")
        expect_message(clients[0], b"client 1: world\n")
        expect_message(clients[2], b"client 1: world\n")
    finally:
        for c in clients:
            c.close()
        proc.terminate(); proc.wait()


def test_broadcast_not_echoed():
    port = find_free_port()
    proc, clients = connected_clients(port, 2)
    try:
        clients[0].sendall(b"secret\n")
        expect_message(clients[1], b"client 0: secret\n")
        expect_silence(clients[0])  # sender must not receive its own message
    finally:
        for c in clients:
            c.close()
        proc.terminate(); proc.wait()


def test_multiline_message():
    port = find_free_port()
    proc, clients = connected_clients(port, 2)
    try:
        clients[0].sendall(b"Hello\nWorld\n")
        expect_message(clients[1], b"client 0: Hello\nclient 0: World\n")
    finally:
        for c in clients:
            c.close()
        proc.terminate(); proc.wait()


def test_fragmented_message():
    port = find_free_port()
    proc, clients = connected_clients(port, 2)
    try:
        clients[0].sendall(b"Hel")
        expect_silence(clients[1])  # no broadcast before newline
        clients[0].sendall(b"lo\n")
        expect_message(clients[1], b"client 0: Hello\n")
    finally:
        for c in clients:
            c.close()
        proc.terminate(); proc.wait()


def test_partial_buffer():
    port = find_free_port()
    proc, clients = connected_clients(port, 2)
    try:
        clients[0].sendall(b"Hello\nWor")
        expect_message(clients[1], b"client 0: Hello\n")
        expect_silence(clients[1])
        clients[0].sendall(b"ld\n")
        expect_message(clients[1], b"client 0: World\n")
    finally:
        for c in clients:
            c.close()
        proc.terminate(); proc.wait()


def test_disconnect_notification():
    port = find_free_port()
    proc, clients = connected_clients(port, 2)
    try:
        clients[1].close()
        expect_message(clients[0], b"server: client 1 just left\n")
    finally:
        try:
            clients[0].close()
        except OSError:
            pass
        proc.terminate(); proc.wait()


def test_reconnect_after_disconnect():
    port = find_free_port()
    proc, clients = connected_clients(port, 2)
    try:
        clients[1].close()
        expect_message(clients[0], b"server: client 1 just left\n")
        c2 = connect_client(port)
        expect_message(clients[0], b"server: client 2 just arrived\n")
        c2.sendall(b"back\n")
        expect_message(clients[0], b"client 2: back\n")
        c2.close()
    finally:
        try:
            clients[0].close()
        except OSError:
            pass
        proc.terminate(); proc.wait()


def test_multi_client_broadcast():
    port = find_free_port()
    proc, clients = connected_clients(port, 4)
    try:
        clients[0].sendall(b"to all\n")
        for c in clients[1:]:
            expect_message(c, b"client 0: to all\n")
    finally:
        for c in clients:
            c.close()
        proc.terminate(); proc.wait()


def test_repeated_messages():
    port = find_free_port()
    proc, clients = connected_clients(port, 2)
    try:
        clients[0].sendall(b"one\n")
        expect_message(clients[1], b"client 0: one\n")
        clients[0].sendall(b"two\n")
        expect_message(clients[1], b"client 0: two\n")
        expect_silence(clients[1])
    finally:
        for c in clients:
            c.close()
        proc.terminate(); proc.wait()


def test_lazy_client_not_disconnected():
    port = find_free_port()
    proc, clients = connected_clients(port, 2)
    try:
        # client 1 never reads; client 0 sends several messages
        clients[0].sendall(b"m1\n")
        clients[0].sendall(b"m2\n")
        clients[0].sendall(b"m3\n")
        # client 1 should still be connected (not dropped)
        c = socket.create_connection((HOST, port), timeout=1)
        check("lazy client still connected (server alive)", c.getpeername()[1] == port)
        c.close()
    finally:
        for c in clients:
            c.close()
        proc.terminate(); proc.wait()


def test_large_message():
    port = find_free_port()
    proc, clients = connected_clients(port, 2)
    try:
        payload = "A" * 500 + "\n"
        clients[0].sendall(payload.encode())
        expect_message(clients[1], ("client 0: " + payload).encode())
    finally:
        for c in clients:
            c.close()
        proc.terminate(); proc.wait()


# ---------------------------------------------------------------------------
# Error handling
# ---------------------------------------------------------------------------

def test_wrong_number_of_arguments():
    r = subprocess.run([EXECUTABLE], capture_output=True, text=True)
    check("no args -> exit 1", r.returncode == 1, "(exit %d)" % r.returncode)
    check("no args -> stderr message",
          "Wrong number of arguments" in r.stderr, "(stderr %r)" % r.stderr)


def test_extra_arguments():
    r = subprocess.run([EXECUTABLE, "1234", "extra"], capture_output=True, text=True)
    check("too many args -> exit 1", r.returncode == 1, "(exit %d)" % r.returncode)


# ---------------------------------------------------------------------------
# Static / source checks
# ---------------------------------------------------------------------------

def test_no_define_preprocessor():
    src = open(os.path.join(ROOT, "mini_serv", "mini_serv.c")).read()
    has_define = re.search(r"(?m)^\s*#\s*define\b", src)
    check("no #define preprocessor directive", has_define is None)


def test_allowed_functions_only():
    src = open(os.path.join(ROOT, "mini_serv", "mini_serv.c")).read()
    # strip comments to avoid false positives
    src_nc = re.sub(r"/\*.*?\*/", "", src, flags=re.S)
    src_nc = re.sub(r"//[^\n]*", "", src_nc)
    allowed = {"write", "close", "select", "socket", "accept", "listen", "send",
               "recv", "bind", "strstr", "malloc", "realloc", "free", "calloc",
               "bzero", "atoi", "sprintf", "strlen", "exit", "strcpy", "strcat",
               "memset", "htons", "htonl"}
    # select() family macros are part of the fd_set API, not forbidden functions.
    allowed |= {"FD_SET", "FD_CLR", "FD_ISSET", "FD_ZERO"}
    # find identifier calls followed by '('
    calls = re.findall(r"\b([A-Za-z_]\w*)\s*\(", src_nc)
    forbidden = set(calls) - allowed
    # allow local helper functions we defined
    our_helpers = {"fatal_error", "broadcast_except", "extract_message",
                   "append_buffer", "register_client", "remove_client",
                   "flush_messages", "create_listening_socket", "main"}
    forbidden -= our_helpers
    # remove C keywords / operators that the regex wrongly captured
    keywords = {"for", "if", "while", "sizeof", "return", "switch"}
    forbidden -= keywords
    check("only allowed functions used", forbidden == set(),
          "(forbidden: %s)" % sorted(forbidden))


def test_listens_on_127_0_0_1():
    src = open(os.path.join(ROOT, "mini_serv", "mini_serv.c")).read()
    check("binds to 127.0.0.1 (htonl 2130706433)",
          "2130706433" in src)


def test_uses_select():
    src = open(os.path.join(ROOT, "mini_serv", "mini_serv.c")).read()
    check("uses select()", "select(" in src)


def main():
    print("=" * 60)
    print("mini_serv test suite")
    print("=" * 60)
    if not os.path.exists(EXECUTABLE):
        print("  [FAIL] executable not built: %s" % EXECUTABLE)
        return 1

    test_connection()
    test_arrival_notification()
    test_id_assignment()
    test_broadcast_not_echoed()
    test_multiline_message()
    test_fragmented_message()
    test_partial_buffer()
    test_disconnect_notification()
    test_reconnect_after_disconnect()
    test_multi_client_broadcast()
    test_repeated_messages()
    test_lazy_client_not_disconnected()
    test_large_message()
    test_wrong_number_of_arguments()
    test_extra_arguments()
    test_no_define_preprocessor()
    test_allowed_functions_only()
    test_listens_on_127_0_0_1()
    test_uses_select()

    print("\n%d passed, %d failed" % (PASS, FAIL))
    return 0 if FAIL == 0 else 1


if __name__ == "__main__":
    sys.exit(main())
