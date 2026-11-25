# webserv

A HTTP/1.1 web server written in **C++98**, inspired by NGINX.

## Features

- **HTTP/1.1** server in strict C++98 (`-std=c++98 -Wall -Wextra -Werror`)
- **Single event loop** using `select()` — multiple concurrent clients, no thread per connection
- **NGINX-style configuration file** (`.conf`) with:
  - Multiple `server` blocks (different ports)
  - `location` blocks with **longest-prefix-match** routing
  - `listen`, `server_name`, `root`, `alias`, `index`
  - `error_page`, `client_max_body_size`
  - `allow_methods` / `limit_except`
  - `autoindex`, `cgi` / `cgi_pass` / `cgi_ext`, `upload_store`
  - `return` (redirection)
- **Methods**: GET, POST, DELETE, HEAD
- **Static files** with correct MIME types and streaming
- **Directory listing** (autoindex)
- **Custom error pages** (404, 403, etc.)
- **Redirections** (`return 301 <url>`)
- **CGI** via `fork()` + pipes with full RFC 3875 environment variables
  - Multiple script types (`.py`, `.pl`, ...)
  - CGI timeout → **504 Gateway Timeout**
- **Uploads**: multipart POST → `upload_store` (201), DELETE → 204
- **Keep-alive** persistent connections
- **Status codes**: 200, 201, 204, 301, 400, 404, 405, 413, 500, 501, 502, 504
- **Transfer-Encoding: chunked** request body decoding

## Bonus

- **Sessions / Cookies** — `webservsid` cookie on every response
- **Multiple CGI types** — Python, Perl, etc.
- **Date header** on every response
- **Access / Error logs** — `access.log`, `error.log`
- **Multiple server blocks** — separate ports

## Build

```bash
make
```

## Run

```bash
./webserv webserv.conf
```

The server listens on the ports defined in the config (8080 and 8081 by default).

## Test

```bash
python3 tests/run_tests.py
```

The test suite uses only the Python standard library (`http.client`, `socket`)
and covers static files, CGI, uploads, redirects, error pages, keep-alive,
chunked encoding, concurrency, and all bonus features.

## Configuration example

```nginx
server {
    listen 8080;
    server_name localhost;
    root ./www;
    index index.html;
    error_page 404 /error/404.html;
    client_max_body_size 1m;

    location /static {
        root ./www;
        allow_methods GET HEAD;
    }

    location /cgi-bin {
        alias ./www/cgi-bin;
        cgi .py python3;
        cgi .pl perl;
    }

    location /upload {
        alias ./www/upload;
        upload_store ./www/upload;
        allow_methods POST DELETE;
    }

    location /redirect {
        return 301 /;
    }

    location /list {
        root ./www;
        autoindex on;
    }
}
```

## Project structure

```
webserv/
├── Makefile          # C++98 build, -Wall -Wextra -Werror
├── webserv.conf      # NGINX-style config
├── includes/         # Headers
│   ├── Utils.hpp     # String/MIME/URL helpers
│   ├── Config.hpp    # Config structures + parser
│   ├── Request.hpp   # HTTP request
│   └── Server.hpp    # Server, ClientState, event loop
├── srcs/             # Sources
│   ├── Utils.cpp
│   ├── Config.cpp
│   ├── Request.cpp
│   ├── Server.cpp    # select() loop, routing, CGI, uploads
│   └── main.cpp
├── www/              # Web content
│   ├── index.html
│   ├── static/
│   ├── error/
│   ├── cgi-bin/      # hello.py, form.py, hello.pl
│   ├── upload/
│   └── list/
└── tests/            # Test suite (stdlib Python)
    ├── lib.py
    └── run_tests.py
```
