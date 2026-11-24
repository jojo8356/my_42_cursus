#!/bin/sh
# ============================================================================ #
#  ft_services — NGINX entrypoint: generate self-signed TLS cert if missing,  #
#  then run nginx in the foreground (real process, no sleep/tail).           #
# ============================================================================ #
set -e

CERT_DIR="/etc/nginx/ssl"
CERT="$CERT_DIR/nginx.crt"
KEY="$CERT_DIR/nginx.key"

mkdir -p "$CERT_DIR"

if [ ! -f "$CERT" ] || [ ! -f "$KEY" ]; then
    echo ">> Generating self-signed TLS certificate for nginx..."
    openssl req -x509 -nodes -days 365 -newkey rsa:2048 \
        -keyout "$KEY" \
        -out "$CERT" \
        -subj "/CN=nginx" >/dev/null 2>&1
    echo ">> Certificate generated."
fi

echo ">> Starting nginx (80/443, TLS 1.2/1.3)..."
exec nginx -g 'daemon off;'
