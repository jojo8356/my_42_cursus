#!/usr/bin/env python3
import os
import sys

name = os.environ.get("QUERY_STRING", "")
for pair in name.split("&"):
    if pair.startswith("name="):
        name = pair[5:]
        break
body = "Hello, %s! (via CGI)" % name
print("Status: 200 OK")
print("Content-Type: text/plain")
print("Content-Length: %d" % len(body))
print()
print(body)
