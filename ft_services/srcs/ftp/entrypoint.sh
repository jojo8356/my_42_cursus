#!/bin/sh
# ============================================================================ #
#  ft_services — FTPS entrypoint: generate SSL cert, create the FTP user,     #
#  then run proftpd in the foreground (real process, no sleep/tail).        #
# ============================================================================ #
set -e

SSL_DIR="/etc/proftpd/ssl"
CERT="$SSL_DIR/ftp.crt"
KEY="$SSL_DIR/ftp.key"
FTP_USER="${FTP_USER:-ftpuser}"
FTP_PASSWORD="${FTP_PASSWORD:-}"

mkdir -p "$SSL_DIR" /srv/ftp

# Self-signed certificate for FTPS
if [ ! -f "$CERT" ] || [ ! -f "$KEY" ]; then
    echo ">> Generating self-signed TLS certificate for FTPS..."
    openssl req -x509 -nodes -days 365 -newkey rsa:2048 \
        -keyout "$KEY" \
        -out "$CERT" \
        -subj "/CN=ftp" >/dev/null 2>&1
    echo ">> Certificate generated."
fi

# Create the FTP user with its password (credentials come from env/Secret)
if ! id "$FTP_USER" >/dev/null 2>&1; then
    echo ">> Creating FTP user (${FTP_USER})..."
    addgroup -S "$FTP_USER" 2>/dev/null || true
    adduser -S -G "$FTP_USER" -h /srv/ftp -s /sbin/nologin "$FTP_USER"
    echo "${FTP_USER}:${FTP_PASSWORD}" | chpasswd
fi

chown -R "$FTP_USER":"$FTP_USER" /srv/ftp

echo ">> Starting FTPS (proftpd, port 21, TLS)..."
# -d runs proftpd in the foreground (no daemonize)
exec proftpd -d
