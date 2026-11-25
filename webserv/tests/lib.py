#!/usr/bin/env python3
"""Test harness for webserv — uses only Python stdlib (http.client, socket)."""
import http.client
import socket
import time
import os
import sys

HOST = "127.0.0.1"

PASS = 0
FAIL = 0
FAILURES = []


def check(name, cond, detail=""):
    global PASS, FAIL
    if cond:
        PASS += 1
        print("  [PASS] %s" % name)
    else:
        FAIL += 1
        FAILURES.append("%s %s" % (name, detail))
        print("  [FAIL] %s %s" % (name, detail))


def request(port, method, path, body=None, headers=None, host=HOST,
            timeout=5):
    """Send a raw HTTP request via http.client and return (status, headers, body)."""
    conn = http.client.HTTPConnection(host, port, timeout=timeout)
    hdrs = dict(headers) if headers else {}
    conn.request(method, path, body=body, headers=hdrs)
    resp = conn.getresponse()
    data = resp.read()
    result = (resp.status, dict(resp.getheaders()), data)
    conn.close()
    return result


def raw_request(port, raw_data, host=HOST, timeout=5):
    """Send raw bytes to the server and return the raw response."""
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.settimeout(timeout)
    s.connect((host, port))
    s.sendall(raw_data)
    chunks = []
    try:
        while True:
            d = s.recv(65536)
            if not d:
                break
            chunks.append(d)
    except socket.timeout:
        pass
    s.close()
    return b"".join(chunks)


def start_server(binary, conf, cwd):
    """Start the webserv binary as a subprocess and wait for the port."""
    import subprocess
    proc = subprocess.Popen(
        [binary, conf], cwd=cwd,
        stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    # wait for ports to open
    for _ in range(50):
        try:
            ok = True
            for port in (8080, 8081):
                s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                s.settimeout(0.2)
                s.connect((HOST, port))
                s.close()
            if ok:
                return proc
        except (socket.error, ConnectionRefusedError):
            time.sleep(0.1)
    proc.kill()
    raise RuntimeError("server did not start on port 8080")


def stop_server(proc):
    proc.terminate()
    try:
        proc.wait(timeout=3)
    except subprocess.TimeoutExpired:
        proc.kill()


def summary():
    print("\n" + "=" * 60)
    print("RESULT: %d passed, %d failed" % (PASS, FAIL))
    if FAILURES:
        print("Failures:")
        for f in FAILURES:
            print("  - " + f)
    print("=" * 60)
    return FAIL == 0
