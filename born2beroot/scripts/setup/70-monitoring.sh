#!/bin/bash
# =============================================================================
#  70-monitoring.sh — install monitoring.sh, schedule it with cron, run it once
# =============================================================================
#  Subject v5.2, chapter V:
#    * a bash script called monitoring.sh
#    * displays the required information on all terminals
#    * at server startup AND every 10 minutes (cron + wall)
#    * no errors displayed
# =============================================================================

set -euo pipefail
. "$(dirname "${BASH_SOURCE[0]}")/../lib/common.sh"
load_config

require_root
install_packages cron

SRC="$B2BR_SCRIPTS/monitoring.sh"

# --- 1. Install the script ---------------------------------------------------
if [ "$B2BR_DRY_RUN" = "1" ]; then
	log_dry "would install $MONITORING_SCRIPT (0755 root:root)"
else
	if cmp -s "$SRC" "$MONITORING_SCRIPT"; then
		log_skip "$MONITORING_SCRIPT already up to date"
	else
		install -D -m 0755 -o root -g root "$SRC" "$MONITORING_SCRIPT"
		log_ok "$MONITORING_SCRIPT installed (0755 root:root)"
	fi
	# Sanity: the script must be syntactically valid bash.
	bash -n "$MONITORING_SCRIPT" || die "monitoring.sh has a syntax error"
	log_ok "bash -n $MONITORING_SCRIPT: syntax OK"
fi

# --- 2. cron -----------------------------------------------------------------
enable_service cron 2>/dev/null || enable_service crond 2>/dev/null || true

if [ "$B2BR_DRY_RUN" = "1" ]; then
	log_dry "would add the crontab line: $MONITORING_CRON $MONITORING_SCRIPT"
else
	line="$MONITORING_CRON $MONITORING_SCRIPT"
	# root's crontab, idempotent: drop any previous monitoring entry first.
	# "crontab -l" exits 1 when root has no crontab yet, which is a normal
	# first-run situation and must not abort the script under "set -e".
	{
		crontab -l -u root 2>/dev/null | grep -vF "$MONITORING_SCRIPT" || true
		echo "$line"
	} | crontab -u root -
	log_ok "root crontab: $line"
	crontab -l -u root 2>/dev/null | sed 's/^/         /' || true
fi

# --- 3. Run once at boot -----------------------------------------------------
# cron cannot run "@reboot" reliably on every image, and the subject asks for a
# run "at server startup", so a oneshot systemd unit does it deterministically.
if [ "$B2BR_DRY_RUN" = "1" ]; then
	log_dry "would install born2beroot-monitoring.service (oneshot, at boot)"
else
	write_file /etc/systemd/system/born2beroot-monitoring.service 0644 root:root \
		"[Unit]" \
		"Description=Born2beRoot monitoring.sh (subject v5.2)" \
		"After=network-online.target cron.service" \
		"Wants=network-online.target" \
		"" \
		"[Service]" \
		"Type=oneshot" \
		"ExecStart=$MONITORING_SCRIPT" \
		"StandardOutput=journal" \
		"" \
		"[Install]" \
		"WantedBy=multi-user.target" || true
	systemctl daemon-reload >/dev/null 2>&1 || true
	systemctl enable born2beroot-monitoring.service >/dev/null 2>&1 || true
	log_ok "born2beroot-monitoring.service enabled (runs at boot)"
fi

# --- 4. Smoke test -----------------------------------------------------------
if [ "$B2BR_DRY_RUN" = "0" ]; then
	out=$("$MONITORING_SCRIPT" --no-wall 2>&1)
	rc=$?
	if [ "$rc" != "0" ] || printf '%s' "$out" | grep -qiE 'error|not found|command not found'; then
		log_fail "monitoring.sh emitted an error (rc=$rc):"
		printf '%s\n' "$out" | sed 's/^/         /' >&2
	else
		log_ok "monitoring.sh runs cleanly"
	fi
	# Keep a copy for the defence: the evaluator may ask to see it.
	printf '%s\n' "$out" > "$MONITORING_LOG" 2>/dev/null || true
fi

log_ok "70-monitoring.sh done"
