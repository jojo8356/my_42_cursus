#!/bin/sh
# Container entrypoint: prepare TLS, run migrations, start daphne + nginx.
set -e

CERT_DIR=/etc/nginx/certs

# 1. Generate a self-signed TLS certificate if none is mounted (subject:
#    HTTPS/wss mandatory). In production, mount real certs over this dir.
if [ ! -f "$CERT_DIR/server.crt" ]; then
    mkdir -p "$CERT_DIR"
    openssl req -x509 -nodes -newkey rsa:2048 -days 365 \
        -keyout "$CERT_DIR/server.key" \
        -out "$CERT_DIR/server.crt" \
        -subj "/CN=localhost" 2>/dev/null
fi

# 2. Apply database migrations (idempotent).
python manage.py migrate --noinput

# 3. Start the ASGI server (daphne) in the background. It serves both HTTP
#    and WebSocket traffic on 127.0.0.1:8000.
daphne -b 127.0.0.1 -p 8000 config.asgi:application &

# 4. Start nginx in the foreground (proxies 80->443 redirect and 443->daphne).
exec nginx -g 'daemon off;'
