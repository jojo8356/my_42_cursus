# ============================================================================ #
#  ft_irc test suite — framework + integration harness.                      #
#  Static tests validate the Makefile/structure; integration tests start the #
#  real server and exercise every command with real socket clients.          #
# ============================================================================ #
import os
import re
import time
import socket
import subprocess

TESTS_DIR = os.path.dirname(os.path.abspath(__file__))
PROJECT_ROOT = os.path.dirname(TESTS_DIR)
BINARY = os.path.join(PROJECT_ROOT, "ircserv")

_RESULTS = {"passed": 0, "failed": 0, "errors": []}


class CheckFailed(Exception):
    pass


def check(condition, label, detail=""):
    if condition:
        _RESULTS["passed"] += 1
        return condition
    msg = label if not detail else f"{label} — {detail}"
    _RESULTS["failed"] += 1
    _RESULTS["errors"].append(msg)
    raise CheckFailed(msg)


def expect(condition, label, detail=""):
    if condition:
        _RESULTS["passed"] += 1
        return True
    msg = label if not detail else f"{label} — {detail}"
    _RESULTS["failed"] += 1
    _RESULTS["errors"].append(msg)
    return False


def reset_results():
    _RESULTS["passed"] = 0
    _RESULTS["failed"] = 0
    _RESULTS["errors"] = []


def get_results():
    return _RESULTS


def read_file(rel):
    with open(os.path.join(PROJECT_ROOT, rel), "r", encoding="utf-8") as fh:
        return fh.read()


def path_exists(rel):
    return os.path.exists(os.path.join(PROJECT_ROOT, rel))


# --------------------------------------------------------------------------- #
#  Integration harness                                                          #
# --------------------------------------------------------------------------- #
class IrcClient:
    """A minimal IRC client over a raw TCP socket with line buffering."""

    def __init__(self, port, password=None, nick=None, user=None,
                 realname="Tester", op_password=None):
        self.sock = socket.create_connection(("127.0.0.1", port), timeout=3)
        self.sock.settimeout(2.0)
        self.buf = b""
        self.nick = nick
        if password:
            self.send("PASS " + password)
        if nick:
            self.send("NICK " + nick)
        if user:
            self.send("USER " + user + " 0 * :" + realname)
        # drain registration replies
        self.drain(0.3)

    def send(self, line):
        self.sock.sendall((line + "\r\n").encode())

    def _recv_some(self, timeout):
        try:
            data = self.sock.recv(4096)
            if data:
                self.buf += data
                return True
        except socket.timeout:
            pass
        return False

    def drain(self, duration=0.3):
        end = time.time() + duration
        while time.time() < end:
            self._recv_some(0.1)
        return self.buf.decode(errors="replace")

    def recv_until(self, pattern, timeout=2.0):
        """Read until the buffer contains `pattern` (regex). Returns buffer."""
        end = time.time() + timeout
        while time.time() < end:
            if re.search(pattern, self.buf.decode(errors="replace")):
                return self.buf.decode(errors="replace")
            self._recv_some(0.1)
        return self.buf.decode(errors="replace")

    def recv_all(self, duration=0.4):
        end = time.time() + duration
        while time.time() < end:
            self._recv_some(0.1)
        return self.buf.decode(errors="replace")

    def clear(self):
        """Drop any already-buffered data (avoid stale pattern matches)."""
        self.buf = b""

    def close(self):
        try:
            self.send("QUIT :bye")
            self.sock.close()
        except Exception:
            try:
                self.sock.close()
            except Exception:
                pass


def start_server(port, password="secret123", op_password="oppass"):
    """Start ircserv in the background and wait until it is listening."""
    proc = subprocess.Popen(
        [BINARY, str(port), password, op_password],
        stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    # wait for the listening socket
    for _ in range(40):
        try:
            s = socket.create_connection(("127.0.0.1", port), timeout=0.3)
            s.close()
            return proc
        except OSError:
            time.sleep(0.1)
    raise RuntimeError("server did not start on port %d" % port)


def stop_server(proc):
    proc.terminate()
    try:
        proc.wait(timeout=3)
    except subprocess.TimeoutExpired:
        proc.kill()
