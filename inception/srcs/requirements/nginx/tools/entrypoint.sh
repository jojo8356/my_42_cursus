#!/bin/sh
# ============================================================================ #
#  NGINX entrypoint — generates a self-signed TLS certificate if missing,     #
#  then runs nginx in the foreground (real process, no infinite sleep / tail). #
# ============================================================================ #
set -e

DOMAIN_NAME="${DOMAIN_NAME:-inception.42.fr}"
CERT_DIR="/etc/nginx/certs"
CERT="$CERT_DIR/nginx.crt"
KEY="$CERT_DIR/nginx.key"

mkdir -p "$CERT_DIR"

if [ ! -f "$CERT" ] || [ ! -f "$KEY" ]; then
    echo ">> Generating self-signed TLS certificate for ${DOMAIN_NAME}..."
    openssl req -x509 -nodes -days 365 -newkey rsa:2048 \
        -keyout "$KEY" \
        -out "$CERT" \
        -subj "/CN=${DOMAIN_NAME}" >/dev/null 2>&1
    echo ">> Certificate generated."
fi

echo ">> Starting NGINX (TLS 1.2/1.3, port 443)..."
exec nginx -g 'daemon off;'
