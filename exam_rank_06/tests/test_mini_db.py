#!/usr/bin/env python3
"""
Comprehensive test suite for mini_db (42 Exam Rank 06).
'Grandes entreprises' quality: functional, edge-case, persistence, and
static-analysis checks covering the full subject requirements.
"""
import os
import re
import socket
import subprocess
import sys
import tempfile
import time

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)
EXECUTABLE = os.path.join(ROOT, "mini_db", "mini_db")

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


class ServerCtx:
    """Start mini_db with a temp persistence path; clean up afterwards."""

    def __init__(self, db_path):
        self.db_path = db_path
        self.port = find_free_port()
        self.proc = None
        self.log = None

    def __enter__(self):
        self.log = tempfile.NamedTemporaryFile(mode="w+", delete=False)
        self.proc = subprocess.Popen([EXECUTABLE, str(self.port), self.db_path],
                                     stdout=self.log, stderr=subprocess.STDOUT)
        # Wait for "ready" by polling the log file.
        deadline = time.monotonic() + 3
        while time.monotonic() < deadline:
            if self.proc.poll() is not None:
                raise RuntimeError("server exited early")
            self.log.flush()
            try:
                content = open(self.log.name).read()
            except OSError:
                content = ""
            if "ready" in content:
                time.sleep(0.1)  # let the select loop settle
                return self
            time.sleep(0.05)
        raise RuntimeError("server did not print ready")

    def stop(self, sigint=True):
        if self.proc is None:
            return
        if sigint:
            self.proc.send_signal(subprocess.signal.SIGINT)
        else:
            self.proc.terminate()
        try:
            self.proc.wait(timeout=3)
        except subprocess.TimeoutExpired:
            self.proc.kill()
            self.proc.wait()
        self.proc = None
        if self.log:
            try:
                self.log.close()
            except Exception:
                pass
            try:
                os.remove(self.log.name)
            except OSError:
                pass

    def __exit__(self, *exc):
        self.stop()


def cmd(port, request, timeout=4.0):
    """Send a request to a fresh connection and read one response line."""
    c = socket.create_connection((HOST, port), timeout=timeout)
    c.settimeout(timeout)
    c.sendall((request + "\n").encode())
    data = b""
    deadline = time.monotonic() + timeout
    while time.monotonic() < deadline:
        try:
            chunk = c.recv(4096)
        except socket.timeout:
            break
        if not chunk:
            break
        data += chunk
        if b"\n" in data:
            break
    c.close()
    return data.decode()


def session(port, requests):
    """Send multiple commands over a SINGLE persistent connection."""
    c = socket.create_connection((HOST, port), timeout=2.0)
    c.settimeout(2.0)
    c.sendall("".join(r + "\n" for r in requests).encode())
    responses = []
    # read until we have as many newline-terminated responses as requests
    buf = b""
    deadline = time.monotonic() + 3
    while len([x for x in buf.decode().split("\n") if x]) < len(requests):
        if time.monotonic() > deadline:
            break
        try:
            chunk = c.recv(4096)
        except socket.timeout:
            break
        if not chunk:
            break
        buf += chunk
    c.close()
    return [x for x in buf.decode().split("\n") if x]


# ---------------------------------------------------------------------------
# Functional tests
# ---------------------------------------------------------------------------

def test_ready_message():
    path = "/tmp/mdb_ready_%d.txt" % os.getpid()
    if os.path.exists(path):
        os.remove(path)
    try:
        with ServerCtx(path):
            check("prints ready on startup", True)
    except RuntimeError as e:
        check("prints ready on startup", False, str(e))
    finally:
        if os.path.exists(path):
            os.remove(path)


def test_post_get_delete():
    path = "/tmp/mdb_cmds_%d.txt" % os.getpid()
    if os.path.exists(path):
        os.remove(path)
    try:
        with ServerCtx(path) as s:
            check("POST returns 0", cmd(s.port, "POST A B").strip() == "0")
            check("POST second returns 0", cmd(s.port, "POST B C").strip() == "0")
            check("GET existing returns 0 B",
                  cmd(s.port, "GET A").strip() == "0 B")
            check("GET missing returns 1",
                  cmd(s.port, "GET Z").strip() == "1")
            check("DELETE existing returns 0",
                  cmd(s.port, "DELETE A").strip() == "0")
            check("GET after DELETE returns 1",
                  cmd(s.port, "GET A").strip() == "1")
            check("DELETE missing returns 1",
                  cmd(s.port, "DELETE A").strip() == "1")
    finally:
        if os.path.exists(path):
            os.remove(path)


def test_unknown_command():
    path = "/tmp/mdb_unk_%d.txt" % os.getpid()
    if os.path.exists(path):
        os.remove(path)
    try:
        with ServerCtx(path) as s:
            check("unknown command returns 2",
                  cmd(s.port, "FOOBAR").strip() == "2")
            check("unknown with args returns 2",
                  cmd(s.port, "FOOBAR x y").strip() == "2")
    finally:
        if os.path.exists(path):
            os.remove(path)


def test_persistent_connection():
    path = "/tmp/mdb_persist_%d.txt" % os.getpid()
    if os.path.exists(path):
        os.remove(path)
    try:
        with ServerCtx(path) as s:
            resp = session(s.port, ["POST K V1", "GET K", "POST K V2", "GET K"])
            check("persistent connection multi-command",
                  resp == ["0", "0 V1", "0", "0 V2"], "(got %r)" % resp)
    finally:
        if os.path.exists(path):
            os.remove(path)


def test_persistence_sigint_and_reload():
    path = "/tmp/mdb_save_%d.txt" % os.getpid()
    if os.path.exists(path):
        os.remove(path)
    proc = None
    try:
        s = ServerCtx(path)
        s.__enter__()
        cmd(s.port, "POST X valueX")
        cmd(s.port, "POST Y valueY")
        s.stop(sigint=True)  # SIGINT -> save & exit
        check("db file created on SIGINT", os.path.exists(path))
        if os.path.exists(path):
            content = open(path).read()
            check("db file contains X valueX", "X valueX" in content)
            check("db file contains Y valueY", "Y valueY" in content)
        # restart and verify reload
        s2 = ServerCtx(path)
        s2.__enter__()
        check("GET X after reload", cmd(s2.port, "GET X").strip() == "0 valueX")
        check("GET Y after reload", cmd(s2.port, "GET Y").strip() == "0 valueY")
        s2.stop(sigint=True)
    finally:
        if os.path.exists(path):
            os.remove(path)


def test_empty_and_malformed():
    path = "/tmp/mdb_mal_%d.txt" % os.getpid()
    if os.path.exists(path):
        os.remove(path)
    try:
        with ServerCtx(path) as s:
            check("GET without key returns 2",
                  cmd(s.port, "GET").strip() == "2")
            check("POST without value returns 2",
                  cmd(s.port, "POST A").strip() == "2")
            check("DELETE without key returns 2",
                  cmd(s.port, "DELETE").strip() == "2")
            check("empty line returns 2",
                  cmd(s.port, "").strip() == "2")
    finally:
        if os.path.exists(path):
            os.remove(path)


def test_large_values():
    path = "/tmp/mdb_large_%d.txt" % os.getpid()
    if os.path.exists(path):
        os.remove(path)
    try:
        with ServerCtx(path) as s:
            big = "V" * 800
            check("POST large value returns 0",
                  cmd(s.port, "POST KEY " + big).strip() == "0")
            got = cmd(s.port, "GET KEY")
            check("GET large value intact",
                  got.strip() == "0 " + big, "(len %d)" % len(got))
    finally:
        if os.path.exists(path):
            os.remove(path)


def test_overwrite_value():
    path = "/tmp/mdb_over_%d.txt" % os.getpid()
    if os.path.exists(path):
        os.remove(path)
    try:
        with ServerCtx(path) as s:
            cmd(s.port, "POST K old")
            cmd(s.port, "POST K new")
            check("POST overwrites value",
                  cmd(s.port, "GET K").strip() == "0 new")
    finally:
        if os.path.exists(path):
            os.remove(path)


def test_multiple_clients():
    path = "/tmp/mdb_multi_%d.txt" % os.getpid()
    if os.path.exists(path):
        os.remove(path)
    try:
        with ServerCtx(path) as s:
            cmd(s.port, "POST shared hello")
            # two concurrent clients reading the same key
            check("client1 GET shared",
                  cmd(s.port, "GET shared").strip() == "0 hello")
            check("client2 GET shared",
                  cmd(s.port, "GET shared").strip() == "0 hello")
    finally:
        if os.path.exists(path):
            os.remove(path)


# ---------------------------------------------------------------------------
# Error handling
# ---------------------------------------------------------------------------

def test_wrong_number_of_arguments():
    r = subprocess.run([EXECUTABLE], capture_output=True, text=True)
    check("no args -> exit 1", r.returncode == 1, "(exit %d)" % r.returncode)
    check("no args -> stderr message",
          "Wrong number of arguments" in r.stderr, "(stderr %r)" % r.stderr)

    r = subprocess.run([EXECUTABLE, "1234"], capture_output=True, text=True)
    check("one arg -> exit 1", r.returncode == 1, "(exit %d)" % r.returncode)


# ---------------------------------------------------------------------------
# Static checks
# ---------------------------------------------------------------------------

def test_cpp_files_exist():
    check("mini_db.cpp exists",
          os.path.exists(os.path.join(ROOT, "mini_db", "mini_db.cpp")))
    check("mini_db.hpp exists",
          os.path.exists(os.path.join(ROOT, "mini_db", "mini_db.hpp")))


def test_listens_on_127_0_0_1():
    src = open(os.path.join(ROOT, "mini_db", "mini_db.cpp")).read()
    check("binds to 127.0.0.1 (htonl 2130706433)",
          "2130706433" in src)


def test_ready_printed():
    src = open(os.path.join(ROOT, "mini_db", "mini_db.cpp")).read()
    check("prints ready", '"ready"' in src)


def test_commands_handled():
    src = open(os.path.join(ROOT, "mini_db", "mini_db.cpp")).read()
    check("handles POST", "POST" in src)
    check("handles GET", "GET" in src)
    check("handles DELETE", "DELETE" in src)


def test_persistence_handling():
    src = open(os.path.join(ROOT, "mini_db", "mini_db.cpp")).read()
    check("SIGINT handler", "SIGINT" in src)
    check("save on signal", "save" in src)
    check("load on startup", "load" in src)


def main():
    print("=" * 60)
    print("mini_db test suite")
    print("=" * 60)
    # Clean up any leftover servers from previous runs (match the full path so
    # we never kill this test process itself).
    try:
        subprocess.run(["pkill", "-9", "-f", EXECUTABLE], capture_output=True)
    except OSError:
        pass
    if not os.path.exists(EXECUTABLE):
        print("  [FAIL] executable not built: %s" % EXECUTABLE)
        return 1

    test_ready_message()
    test_post_get_delete()
    test_unknown_command()
    test_persistent_connection()
    test_persistence_sigint_and_reload()
    test_empty_and_malformed()
    test_large_values()
    test_overwrite_value()
    test_multiple_clients()
    test_wrong_number_of_arguments()
    test_cpp_files_exist()
    test_listens_on_127_0_0_1()
    test_ready_printed()
    test_commands_handled()
    test_persistence_handling()

    print("\n%d passed, %d failed" % (PASS, FAIL))
    return 0 if FAIL == 0 else 1


if __name__ == "__main__":
    sys.exit(main())
