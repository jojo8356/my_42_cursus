#!/usr/bin/env python3
"""Comprehensive test suite for webserv — 'grandes entreprises' quality."""
import os
import sys
import time
import socket
import struct
import threading

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from lib import (request, raw_request, start_server, stop_server,
                 check, summary, HOST)

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
BINARY = os.path.join(ROOT, "webserv")
CONF = os.path.join(ROOT, "webserv.conf")


def test_static():
    print("\n--- Static files ---")
    st, hd, body = request(8080, "GET", "/static/hello.txt")
    check("GET static 200", st == 200, "got %d" % st)
    check("static content-type text/plain",
          hd.get("Content-Type", "").startswith("text/plain"), hd.get("Content-Type", ""))
    check("static body", "Hello from a static file!" in body.decode(), body[:50])
    check("Date header present", "Date" in hd, "no Date header")
    check("Content-Length present", "Content-Length" in hd, "no CL")

    st, hd, body = request(8080, "HEAD", "/static/hello.txt")
    check("HEAD static 200", st == 200, "got %d" % st)
    check("HEAD empty body", body == b"", "body=%r" % body)

    st, hd, body = request(8080, "GET", "/")
    check("GET index 200", st == 200, "got %d" % st)
    check("index is html", "webserv" in body.decode(), "no webserv in index")


def test_404():
    print("\n--- 404 custom error page ---")
    st, hd, body = request(8080, "GET", "/nonexistent")
    check("404 status", st == 404, "got %d" % st)
    check("404 custom page", "404 Not Found" in body.decode(), "no custom page")


def test_405():
    print("\n--- 405 Method Not Allowed ---")
    st, hd, body = request(8080, "POST", "/static/hello.txt", body="x")
    check("405 status", st == 405, "got %d" % st)


def test_501():
    print("\n--- 501 Not Implemented ---")
    st, hd, body = request(8080, "PATCH", "/")
    check("501 status", st == 501, "got %d" % st)


def test_redirect():
    print("\n--- Redirection ---")
    st, hd, body = request(8080, "GET", "/redirect")
    check("301 status", st == 301, "got %d" % st)
    check("301 Location", hd.get("Location") == "/", "Location=%s" % hd.get("Location"))


def test_cgi():
    print("\n--- CGI ---")
    st, hd, body = request(8080, "GET", "/cgi-bin/hello.py?name=world")
    check("CGI hello 200", st == 200, "got %d" % st)
    check("CGI hello body", "Hello, world!" in body.decode(), body[:50])
    check("CGI content-type", "text/plain" in hd.get("Content-Type", ""), hd.get("Content-Type", ""))

    st, hd, body = request(8080, "POST", "/cgi-bin/form.py", body="a=1&b=2",
                           headers={"Content-Type": "application/x-www-form-urlencoded"})
    check("CGI form POST 200", st == 200, "got %d" % st)
    check("CGI form body", "a=1&b=2" in body.decode(), body[:50])


def test_upload():
    print("\n--- Uploads ---")
    boundary = "----testboundary1234"
    file_content = "hello upload content"
    body = (
        "--%s\r\n"
        'Content-Disposition: form-data; name="file"; filename="test_up.txt"\r\n'
        "Content-Type: text/plain\r\n"
        "\r\n"
        "%s\r\n"
        "--%s--\r\n" % (boundary, file_content, boundary)
    )
    st, hd, resp = request(8080, "POST", "/upload", body=body,
                           headers={"Content-Type": "multipart/form-data; boundary=%s" % boundary,
                                    "Content-Length": str(len(body))})
    check("upload 201", st == 201, "got %d" % st)
    # verify file exists
    fpath = os.path.join(ROOT, "www", "upload", "test_up.txt")
    check("uploaded file exists", os.path.isfile(fpath), "no file")
    if os.path.isfile(fpath):
        with open(fpath) as f:
            check("uploaded file content", f.read() == file_content, "wrong content")
    # delete
    st, hd, resp = request(8080, "DELETE", "/upload/test_up.txt")
    check("delete 204", st == 204, "got %d" % st)
    check("deleted file gone", not os.path.isfile(fpath), "file still there")


def test_413():
    print("\n--- 413 Payload Too Large ---")
    big = b"A" * (2 * 1024 * 1024)
    boundary = "----bigboundary"
    body = (
        "--%s\r\n"
        'Content-Disposition: form-data; name="file"; filename="big.txt"\r\n'
        "\r\n"
    ).encode() + big + ("\r\n--%s--\r\n" % boundary).encode()
    st, hd, resp = request(8080, "POST", "/upload", body=body,
                           headers={"Content-Type": "multipart/form-data; boundary=%s" % boundary,
                                    "Content-Length": str(len(body))})
    check("413 status", st == 413, "got %d" % st)


def test_400():
    print("\n--- 400 Bad Request ---")
    st, hd, body = request(8080, "POST", "/upload", body="not multipart",
                           headers={"Content-Type": "multipart/form-data; boundary=xxx"})
    check("400 status", st == 400, "got %d" % st)


def test_keepalive():
    print("\n--- Keep-Alive ---")
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.settimeout(3)
    s.connect((HOST, 8080))
    s.sendall(b"GET / HTTP/1.1\r\nHost: x\r\n\r\n")
    resp1 = s.recv(65536)
    s.sendall(b"GET /static/hello.txt HTTP/1.1\r\nHost: x\r\n\r\n")
    resp2 = s.recv(65536)
    s.close()
    check("keep-alive first response", b"HTTP/1.1 200" in resp1, resp1[:30])
    check("keep-alive second response", b"HTTP/1.1 200" in resp2, resp2[:30])


def test_chunked():
    print("\n--- Chunked Transfer-Encoding ---")
    chunked = (
        "POST /cgi-bin/form.py HTTP/1.1\r\n"
        "Host: x\r\n"
        "Transfer-Encoding: chunked\r\n"
        "Content-Type: application/x-www-form-urlencoded\r\n"
        "\r\n"
        "5\r\nhello\r\n"
        "6\r\n world\r\n"
        "0\r\n\r\n"
    )
    raw = chunked.encode()
    resp = raw_request(8080, raw)
    check("chunked 200", b"HTTP/1.1 200" in resp, resp[:40])
    check("chunked body decoded", b"hello world" in resp, resp[-80:])


def test_concurrency():
    print("\n--- Concurrent clients ---")
    results = []
    def worker(i):
        try:
            st, hd, body = request(8080, "GET", "/static/hello.txt")
            results.append((i, st, body))
        except Exception as e:
            results.append((i, -1, str(e)))
    threads = []
    for i in range(20):
        t = threading.Thread(target=worker, args=(i,))
        threads.append(t)
        t.start()
    for t in threads:
        t.join()
    ok = sum(1 for _, st, _ in results if st == 200)
    check("20 concurrent all 200", ok == 20, "%d ok" % ok)


def test_multipart_get():
    print("\n--- Autoindex / directory listing ---")
    st, hd, body = request(8080, "GET", "/upload")
    # /upload restricts methods to POST/DELETE -> GET returns 405
    check("upload dir GET 405 (methods restricted)", st == 405, "got %d" % st)
    # /static/ is a directory with no index -> should serve index or 404
    st, hd, body = request(8080, "GET", "/static/")
    check("static dir GET handled", st in (200, 404), "got %d" % st)


def test_cgi_env():
    print("\n--- CGI env variables ---")
    # The hello.py script uses QUERY_STRING; verify it works via query
    st, hd, body = request(8080, "GET", "/cgi-bin/hello.py?name=envtest")
    check("CGI query string", "Hello, envtest!" in body.decode(), body[:50])


def test_cookies():
    print("\n--- Bonus: Cookies / Sessions ---")
    st, hd, body = request(8080, "GET", "/")
    check("Set-Cookie present", "Set-Cookie" in hd, "no Set-Cookie")
    check("webservsid cookie", "webservsid" in hd.get("Set-Cookie", ""), hd.get("Set-Cookie", ""))


def test_logs():
    print("\n--- Bonus: Logs ---")
    request(8080, "GET", "/")
    request(8080, "GET", "/nonexistent")
    acc = os.path.join(ROOT, "access.log")
    err = os.path.join(ROOT, "error.log")
    check("access.log exists", os.path.isfile(acc), "no access.log")
    check("error.log exists", os.path.isfile(err), "no error.log")
    if os.path.isfile(acc):
        with open(acc) as f:
            check("access.log has entries", len(f.read().strip()) > 0, "empty")


def test_autoindex():
    print("\n--- Bonus: Autoindex directory listing ---")
    st, hd, body = request(8080, "GET", "/list/")
    check("autoindex 200", st == 200, "got %d" % st)
    check("autoindex html", "file.txt" in body.decode(), "no listing")


def test_second_server():
    print("\n--- Bonus: Multiple server blocks ---")
    st, hd, body = request(8081, "GET", "/")
    check("second server 200", st == 200, "got %d" % st)
    check("second server index", "webserv" in body.decode(), "no index")


def test_perl_cgi():
    print("\n--- Bonus: Multiple CGI types (Perl) ---")
    st, hd, body = request(8080, "GET", "/cgi-bin/hello.pl?name=perl")
    check("Perl CGI 200", st == 200, "got %d" % st)
    check("Perl CGI body", "Hello, perl!" in body.decode(), body[:50])


def main():
    print("=" * 60)
    print("webserv test suite")
    print("=" * 60)
    proc = start_server(BINARY, CONF, ROOT)
    try:
        test_static()
        test_404()
        test_405()
        test_501()
        test_redirect()
        test_cgi()
        test_cgi_env()
        test_upload()
        test_413()
        test_400()
        test_keepalive()
        test_chunked()
        test_concurrency()
        test_multipart_get()
        test_cookies()
        test_logs()
        test_autoindex()
        test_second_server()
        test_perl_cgi()
    finally:
        stop_server(proc)
    ok = summary()
    sys.exit(0 if ok else 1)


if __name__ == "__main__":
    main()
