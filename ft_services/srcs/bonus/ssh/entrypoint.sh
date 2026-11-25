#!/bin/sh
# ============================================================================ #
#  ft_services — SSH entrypoint (BONUS)                                      #
#  Sets up the authorized key / user from Secret, runs sshd in foreground.  #
# ============================================================================ #
set -e

SSH_USER="${SSH_USER:-42}"
SSH_PASSWORD="${SSH_PASSWORD:-}"

# Configure sshd: allow password auth for the app user
mkdir -p /run/sshd
sed -i 's/^#\?PasswordAuthentication.*/PasswordAuthentication yes/' /etc/ssh/sshd_config

# Create the SSH user with the password from the Secret
if ! id "$SSH_USER" >/dev/null 2>&1; then
    echo ">> Creating SSH user (${SSH_USER})..."
    addgroup -S "$SSH_USER" 2>/dev/null || true
    adduser -S -G "$SSH_USER" -h /home/"$SSH_USER" -s /bin/sh "$SSH_USER"
    echo "${SSH_USER}:${SSH_PASSWORD}" | chpasswd
fi

echo ">> Starting SSH (port 22)..."
exec /usr/sbin/sshd -D
