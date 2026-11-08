#!/bin/bash
# =============================================================================
#  lib/common.sh — shared helpers for the Born2beRoot setup scripts
# =============================================================================
#  Sourced (never executed) by every scripts/setup/*.sh file.  It provides:
#    * configuration loading (config/vm.conf)
#    * uniform logging with the [ OK ] / [SKIP] / [FAIL] prefixes
#    * a strict idempotent "write this file once" primitive
#    * a tiny dry-run engine so the very same script can render a preview
#    * root / emulation guards
# =============================================================================

set -o pipefail

# --- Locations ---------------------------------------------------------------
# On a real VM the project lives wherever it was cloned; inside the hermetic
# test VM (tests/vm.sh) it is exposed read-only at /opt/born2beroot and
# B2BR_ROOT points there instead, so the very same scripts run unchanged.
B2BR_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)
B2BR_ROOT=${B2BR_ROOT:-$B2BR_DIR}
B2BR_CONFIG="$B2BR_ROOT/config"
B2BR_SCRIPTS="$B2BR_ROOT/scripts"

# --- Options (exported to the sub-scripts) -----------------------------------
B2BR_DRY_RUN=${B2BR_DRY_RUN:-0}
# Whether the bonus part is being provisioned.  Steps that serve both parts (the
# firewall above all) read this flag; setup.sh exports it once, per run.
B2BR_BONUS=${B2BR_BONUS:-0}
export B2BR_BONUS
B2BR_QUIET=${B2BR_QUIET:-0}

# --- Colours (disabled when stdout is not a terminal) ------------------------
if [ -t 1 ] && [ "${NO_COLOR:-}" = "" ]; then
	C_OK=$'\033[1;32m'
	C_WARN=$'\033[1;33m'
	C_ERR=$'\033[1;31m'
	C_INFO=$'\033[1;34m'
	C_OFF=$'\033[0m'
else
	C_OK='' C_WARN='' C_ERR='' C_INFO='' C_OFF=''
fi

# --- Logging -----------------------------------------------------------------
log_info() { [ "$B2BR_QUIET" = "1" ] || printf '%s[INFO]%s %s\n' "$C_INFO" "$C_OFF" "$*"; }
log_ok()   { [ "$B2BR_QUIET" = "1" ] || printf '%s[ OK ]%s %s\n' "$C_OK" "$C_OFF" "$*"; }
log_skip() { [ "$B2BR_QUIET" = "1" ] || printf '%s[SKIP]%s %s\n' "$C_WARN" "$C_OFF" "$*"; }
log_dry()  { [ "$B2BR_QUIET" = "1" ] || printf '%s[DRY ]%s %s\n' "$C_INFO" "$C_OFF" "$*"; }
log_fail() { printf '%s[FAIL]%s %s\n' "$C_ERR" "$C_OFF" "$*" >&2; }

die() { log_fail "$*"; exit 1; }

# --- Configuration -----------------------------------------------------------
load_config() {
	local conf="${1:-$B2BR_CONFIG/vm.conf}"

	[ -r "$conf" ] || die "cannot read configuration: $conf"
	# shellcheck disable=SC1090
	. "$conf"
	VM_HOSTNAME="${VM_LOGIN}42"
	VM_FQDN="${VM_HOSTNAME}.${VM_DOMAIN}"
}

# --- Guards ------------------------------------------------------------------
require_root() {
	[ "$(id -u)" = "0" ] || die "$(basename "$0") must be run as root (use sudo)."
}

# In the hermetic test environment the machine runs inside user+mount+UTS
# namespaces: being uid 0 there is enough, and /run/b2br-emulated marks it.
in_emulation() { [ -e /run/b2br-emulated ]; }

# --- Idempotent file writing -------------------------------------------------
# write_file <path> <mode> <owner> <content...>
# Writes only when the content differs, so re-running a setup script is a no-op
# and the caller can report [SKIP] instead of touching an already correct file.
write_file() {
	local path="$1" mode="$2" owner="$3"
	shift 3
	local content tmp rc=0

	content=$(printf '%s\n' "$@")
	if [ -f "$path" ] && [ "$(cat "$path")" = "$content" ]; then
		log_skip "$path already up to date"
		return 1
	fi
	if [ "$B2BR_DRY_RUN" = "1" ]; then
		log_dry "would write $path ($mode $owner, $(printf '%s' "$content" | wc -l) lines)"
		return 0
	fi
	tmp=$(mktemp) || die "mktemp failed"
	printf '%s\n' "$content" > "$tmp"
	# Ownership first; if the account cannot be mapped (nested user namespace
	# of the test VM, or a service account missing from a minimal image), fall
	# back to root:root, which is what a package post-install would leave.
	if ! install -D -m "$mode" -o "${owner%%:*}" -g "${owner##*:}" "$tmp" "$path" 2>/dev/null; then
		install -D -m "$mode" "$tmp" "$path" || rc=1
		if [ "$rc" = "0" ]; then
			rm -f "$tmp"
			log_ok "$path written ($mode root:root, $owner not mappable here)"
			return 0
		fi
	fi
	rm -f "$tmp"
	[ "$rc" = "0" ] || die "cannot install $path"
	log_ok "$path written ($mode $owner)"
	return 0
}

# append_unique <file> <line> — adds <line> once, keeps the file idempotent
append_unique() {
	local file="$1" line="$2"

	[ -f "$file" ] || : > "$file"
	if grep -qxF -- "$line" "$file"; then
		log_skip "$file already contains: $line"
		return 1
	fi
	if [ "$B2BR_DRY_RUN" = "1" ]; then
		log_dry "would append to $file: $line"
		return 0
	fi
	printf '%s\n' "$line" >> "$file"
	log_ok "appended to $file: $line"
}

# set_kv <file> <key> <separator> <value> — rewrites "key<sep>value" in place
set_kv() {
	local file="$1" key="$2" sep="$3" value="$4"
	local tmp

	[ -f "$file" ] || : > "$file"
	if [ "$B2BR_DRY_RUN" = "1" ]; then
		log_dry "would set $key$sep$value in $file"
		return 0
	fi
	tmp=$(mktemp) || die "mktemp failed"
	grep -vE "^[[:space:]]*${key}([[:space:]]|${sep}|$)" "$file" > "$tmp" || true
	printf '%s%s%s\n' "$key" "$sep" "$value" >> "$tmp"
	cat "$tmp" > "$file"
	rm -f "$tmp"
	log_ok "$file: $key$sep$value"
}

# --- Package management ------------------------------------------------------
# install_packages <pkg...> — installs missing packages, idempotent.
# In the hermetic test environment apt-get is a simulator that records the
# packages in a real dpkg status database, so the very same call path is used.
install_packages() {
	local missing=() p

	for p in "$@"; do
		dpkg -s "$p" >/dev/null 2>&1 || missing+=("$p")
	done
	if [ "${#missing[@]}" -eq 0 ]; then
		log_skip "packages already installed: $*"
		return 0
	fi
	if [ "$B2BR_DRY_RUN" = "1" ]; then
		log_dry "would install: ${missing[*]}"
		return 0
	fi
	log_info "installing: ${missing[*]}"
	DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends \
		"${missing[@]}" >/dev/null 2>&1 || die "apt-get install failed: ${missing[*]}"
	log_ok "installed: ${missing[*]}"
}

# --- Service management ------------------------------------------------------
# enable_service <unit> — enable + (re)start, idempotent, silent.
enable_service() {
	local unit="$1"

	if [ "$B2BR_DRY_RUN" = "1" ]; then
		log_dry "would enable and restart $unit"
		return 0
	fi
	systemctl enable "$unit" >/dev/null 2>&1
	systemctl restart "$unit" >/dev/null 2>&1 \
		|| service "$unit" restart >/dev/null 2>&1 \
		|| true
	log_ok "service $unit enabled and (re)started"
}

# --- Misc --------------------------------------------------------------------
# version_ge <a> <b> — true when a >= b (integer comparison)
version_ge() { [ "$(printf '%s\n%s\n' "$1" "$2" | sort -V | head -n1)" = "$2" ]; }

# require_cmd <cmd...> — fail early with a readable message
require_cmd() {
	local c
	for c in "$@"; do
		command -v "$c" >/dev/null 2>&1 || die "required command not found: $c"
	done
}
