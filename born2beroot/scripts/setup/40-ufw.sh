#!/bin/bash
# =============================================================================
#  40-ufw.sh — UFW firewall: active, only port 4242 open on the mandatory part
# =============================================================================
#  Subject v5.2, chapter V:
#    * configure the OS with the UFW firewall and leave only port 4242 open
#    * the firewall must be active when the virtual machine is launched
#  Chapter VII: the bonus services are allowed to open more ports, provided the
#  UFW rules are adapted (they are, in config/ufw_rules.sh).
# =============================================================================

set -euo pipefail
. "$(dirname "${BASH_SOURCE[0]}")/../lib/common.sh"
load_config

require_root
install_packages ufw

# --- 1. Server mode, no interactive prompt ----------------------------------
if [ "$B2BR_DRY_RUN" = "1" ]; then
	log_dry "would apply config/ufw_rules.sh (deny incoming, allow $SSH_PORT/tcp, bonus ports)"
else
	# A package installation would otherwise block on a prompt.
	sed -i 's/^ENABLED=.*/ENABLED=yes/' /etc/ufw/ufw.conf 2>/dev/null || true
	ufw --force reset >/dev/null 2>&1 || true
	# Render the rule set with the configured ports, then apply it.
	sed -e "s/4242/${SSH_PORT}/g" -e "s/80\/tcp/${HTTP_PORT}\/tcp/g" \
		-e "s/ 21\/tcp/ ${FTP_PORT}\/tcp/g" \
		-e "s/40000:40010/${FTP_PASSIVE_MIN}:${FTP_PASSIVE_MAX}/g" \
		"$B2BR_CONFIG/ufw_rules.sh" > /run/b2br-ufw-rules.sh
	bash /run/b2br-ufw-rules.sh >/dev/null 2>&1 \
		|| die "applying the UFW rule set failed"
	rm -f /run/b2br-ufw-rules.sh
	log_ok "UFW rule set applied and firewall enabled"
fi

# --- 2. Start at boot --------------------------------------------------------
enable_service ufw

# --- 3. Report the exact output the subject expects --------------------------
if [ "$B2BR_DRY_RUN" = "0" ]; then
	ufw status verbose 2>/dev/null || true
	if ufw status 2>/dev/null | grep -c "Status: active"; then
		log_ok "UFW is active"
	else
		log_fail "UFW is not active"
	fi
	if ufw status 2>/dev/null | grep -qE "^${SSH_PORT}(/tcp)?[[:space:]]+ALLOW"; then
		log_ok "port $SSH_PORT is allowed"
	else
		log_fail "port $SSH_PORT is not allowed"
	fi
fi

log_ok "40-ufw.sh done"
