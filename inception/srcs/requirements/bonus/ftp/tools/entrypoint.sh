#!/bin/bash
# ============================================================================ #
#  FTP entrypoint (BONUS) — creates the FTP user, then runs vsftpd.           #
#  vsftpd daemonizes, so we run it in the background and wait on it (real     #
#  process management — no infinite sleep/tail).                             #
# ============================================================================ #
set -e

FTP_USER="${FTP_USER:-ftpuser}"
FTP_PASSWORD="$(cat /run/secrets/db_password)"
FTP_HOME="/srv/ftp"

# Create the FTP user on first run
if ! id "$FTP_USER" >/dev/null 2>&1; then
    echo ">> Creating FTP user (${FTP_USER})..."
    useradd -d "$FTP_HOME" -s /usr/sbin/nologin "$FTP_USER"
    echo "${FTP_USER}:${FTP_PASSWORD}" | chpasswd
fi

mkdir -p "$FTP_HOME"
chown "$FTP_USER":"$FTP_USER" "$FTP_HOME"

echo ">> Starting vsftpd (port 21)..."
vsftpd /etc/vsftpd.conf &
FTP_PID=$!

# Keep the container alive while vsftpd runs
wait "$FTP_PID"
