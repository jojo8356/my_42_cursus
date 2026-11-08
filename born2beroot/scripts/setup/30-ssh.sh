#!/bin/bash
# =============================================================================
#  30-ssh.sh — SSH on port 4242, root login forbidden
# =============================================================================
#  Subject v5.2, chapter V:
#    * "An SSH service must be running on port 4242"
#    * "it must not be possible to connect using SSH as root"
#  The reference output in the subject shows:
#    tcp LISTEN 0 128 0.0.0.0:4242 ... users:(("sshd",pid=...,fd=3))
# =============================================================================

set -euo pipefail
. "$(dirname "${BASH_SOURCE[0]}")/../lib/common.sh"
load_config

require_root
install_packages openssh-server

SSHD_CONFIG=/etc/ssh/sshd_config
SSHD_DIR=/etc/ssh/sshd_config.d

# The openssh-server package normally creates /etc/ssh; create it defensively
# so the script also works on a minimal image or in the hermetic test VM.
install -d -m 0755 -o root -g root /etc/ssh

# --- 1. Install the audited configuration -----------------------------------
# sshd_config ships with the distribution; rather than rewriting it, the
# project's own file is rendered from the template with the configured port.
rendered=$(sed -e "s/^Port .*/Port ${SSH_PORT}/" \
	"$B2BR_CONFIG/sshd_config")

if [ -f "$SSHD_CONFIG" ] && [ "$(cat "$SSHD_CONFIG")" = "$rendered" ]; then
	log_skip "$SSHD_CONFIG already correct"
else
	if [ "$B2BR_DRY_RUN" = "1" ]; then
		log_dry "would install $SSHD_CONFIG (Port $SSH_PORT, PermitRootLogin no)"
	else
		cp -a "$SSHD_CONFIG" "$SSHD_CONFIG.b2br-backup" 2>/dev/null || true
		printf '%s\n' "$rendered" > "$SSHD_CONFIG"
		chmod 644 "$SSHD_CONFIG"
		log_ok "$SSHD_CONFIG installed (Port $SSH_PORT, PermitRootLogin no)"
	fi
fi

# --- 2. Neutralise drop-in overrides ----------------------------------------
# Debian's default sshd_config starts with "Include /etc/ssh/sshd_config.d/*.conf".
# A drop-in (for instance 50-cloud-init.conf) may re-enable "PermitRootLogin yes"
# and silently break the project, so every drop-in is renamed away and the
# Include line is disabled.
if [ "$B2BR_DRY_RUN" = "1" ]; then
	log_dry "would disable drop-ins in $SSHD_DIR and the Include directive"
else
	if [ -d "$SSHD_DIR" ]; then
		for f in "$SSHD_DIR"/*.conf; do
			[ -e "$f" ] || continue
			mv "$f" "$f.disabled-by-born2beroot"
			log_ok "drop-in disabled: $f"
		done
	fi
	if grep -qE '^[[:space:]]*Include[[:space:]]+/etc/ssh/sshd_config\.d/' "$SSHD_CONFIG"; then
		sed -i -E 's|^[[:space:]]*Include[[:space:]]+/etc/ssh/sshd_config\.d/.*|# & (disabled by born2beroot)|' \
			"$SSHD_CONFIG"
		log_ok "Include directive commented out in $SSHD_CONFIG"
	fi
fi

# --- 3. Validate and restart -------------------------------------------------
if [ "$B2BR_DRY_RUN" = "0" ]; then
	if command -v sshd >/dev/null 2>&1; then
		sshd -t -f "$SSHD_CONFIG" || die "sshd configuration is invalid"
		log_ok "sshd -t: configuration accepted"
	fi
	enable_service ssh
	enable_service sshd 2>/dev/null || true
fi

# --- 4. Report ---------------------------------------------------------------
if [ "$B2BR_DRY_RUN" = "0" ] && command -v ss >/dev/null 2>&1; then
	if ss -tln 2>/dev/null | grep -c ":${SSH_PORT} "; then
		log_ok "sshd is listening on port $SSH_PORT"
	else
		log_fail "nothing is listening on port $SSH_PORT yet"
	fi
fi

log_ok "30-ssh.sh done"
