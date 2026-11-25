#!/usr/bin/env python3
import os
import sys

length = int(os.environ.get("CONTENT_LENGTH", "0"))
body = sys.stdin.read(length) if length else ""
out = "Form received.\n" + body + "\n"
print("Status: 200 OK")
print("Content-Type: text/plain")
print("Content-Length: %d" % len(out))
print()
print(out)
