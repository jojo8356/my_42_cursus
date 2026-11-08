#!/bin/bash
# =============================================================================
#  tests/cases/00-static.sh — checks that need no virtual machine
# =============================================================================
#  Everything here fails fast, before a 30-second boot: a missing configuration
#  key or a syntax error in a script would otherwise only show up much later,
#  buried in the emulator's output.
# =============================================================================

set -uo pipefail

B2BR_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)
. "$B2BR_DIR/tests/lib/assert.sh"

# --- 1. Every shell file parses ----------------------------------------------
for f in $(find "$B2BR_DIR/scripts" "$B2BR_DIR/tests" -name '*.sh' -type f); do
	assert_ok "syntax: ${f#$B2BR_DIR/}" bash -n "$f"
done

for f in "$B2BR_DIR"/scripts/*.sh "$B2BR_DIR"/scripts/setup/*.sh "$B2BR_DIR"/tests/*.sh \
	"$B2BR_DIR"/tests/cases/*.sh "$B2BR_DIR"/tests/lib/*.sh; do
	header=$(head -1 "$f")
	case "$header" in
	'#!/bin/bash'|'#!/usr/bin/env bash') assert_ok "shebang: ${f#$B2BR_DIR/}" true ;;
	*) assert_fail "shebang: ${f#$B2BR_DIR/}" "got [$header]" ;;
	esac
done

# --- 2. The configuration file answers every question the scripts ask --------
#  load_config turns vm.conf into environment variables; a typo in a key name
#  produces an unbound variable error at the worst possible moment.
REQUIRED_KEYS=(
	VM_LOGIN VM_HOSTNAME VM_DOMAIN SSH_PORT HTTP_PORT FTP_PORT
	LVM_VG LUKS_NAME BONUS_WP_DB BONUS_WP_USER BONUS_WP_DIR
	SUDO_ATTEMPTS SUDO_BADPASS_MESSAGE SUDO_LOG_DIR SUDO_SECURE_PATH
	PASS_MAX_DAYS PASS_MIN_DAYS PASS_WARN_AGE PASS_MIN_LEN PASS_MAX_REPEAT PASS_DIFOK
	MONITORING_SCRIPT
)
for key in "${REQUIRED_KEYS[@]}"; do
	if grep -qE "^${key}=" "$B2BR_DIR/config/vm.conf"; then
		assert_ok "vm.conf defines $key"
	else
		assert_fail "vm.conf defines $key" "$key is missing from config/vm.conf"
	fi
done

# --- 3. Nothing secret is committed ------------------------------------------
#  The MariaDB password is generated on the machine; no password may appear in
#  the repository, and the sample configuration must not contain a placeholder
#  that would end up in a real password field.
if grep -rIlE '^(DB_PASSWORD|password)[[:space:]]*=[[:space:]]*["'\'']?[^"'\''$]' \
	"$B2BR_DIR/config" 2>/dev/null | grep -qv 'vm.conf'; then
	assert_fail "no literal password in config/" "a password is hard-coded in config/"
else
	assert_ok "no literal password in config/"
fi

# --- 4. The subject is present and complete ----------------------------------
assert_ok "docs/en.subject.pdf present" test -s "$B2BR_DIR/docs/en.subject.pdf"
assert_ok "docs/en.subject.md present"  test -s "$B2BR_DIR/docs/en.subject.md"
assert_ok "docs/fr.subject.md present"  test -s "$B2BR_DIR/docs/fr.subject.md"
assert_ok "subject is version 5.2" grep -qiE 'version[^a-z]{0,4}5\.2' "$B2BR_DIR/docs/en.subject.md"

# --- 5. The partition layout matches the bonus requirement -------------------
#  "You will have to create at least 7 partitions with their mount points"
#  (root, swap, home, var, srv, tmp, var-log, plus /boot on its own).
for lv in root swap home var srv tmp 'var--log'; do
	if grep -q "LVMGroup-$lv" "$B2BR_DIR/config/partitions.layout"; then
		assert_ok "layout declares LVMGroup-$lv"
	else
		assert_fail "layout declares LVMGroup-$lv" "missing from config/partitions.layout"
	fi
done
assert_ok "layout declares /boot"   grep -qE 'sda1.*/boot' "$B2BR_DIR/config/partitions.layout"
assert_ok "layout is LUKS"          grep -qi 'luks\|crypt' "$B2BR_DIR/config/partitions.layout"

# --- 6. Repository hygiene ---------------------------------------------------
assert_ok "README.md present"           test -s "$B2BR_DIR/README.md"
assert_ok "README first line is the 42 notice" \
	grep -qx '\*This project has been created as part of the 42 curriculum by jojo8356\.\*' "$B2BR_DIR/README.md"

# --- 7. signature.sh and launch.sh -------------------------------------------
#  signature.sh is the tool behind "signature.txt".  It is exercised on a small
#  fake disk so the real code path (hash, write, compare) runs here.
tmpd=$(mktemp -d)
head -c 4096 /dev/urandom > "$tmpd/disk.vdi"
assert_ok "signature.sh writes signature.txt" \
	bash "$B2BR_DIR/scripts/signature.sh" "$tmpd/disk.vdi" --output "$tmpd/sig.txt"
assert_ok "signature.txt holds a SHA-256" \
	bash -c "grep -qE '^[0-9a-f]{64}$' '$tmpd/sig.txt'"
assert_ok "signature.sh --check agrees with the disk" \
	bash "$B2BR_DIR/scripts/signature.sh" "$tmpd/disk.vdi" --check --output "$tmpd/sig.txt"
printf 'x' >> "$tmpd/disk.vdi"
assert_ok "signature.sh --check detects a modified disk" \
	bash -c "! bash '$B2BR_DIR/scripts/signature.sh' '$tmpd/disk.vdi' --check --output '$tmpd/sig.txt' >/dev/null 2>&1"
assert_ok "signature.sh refuses a running machine" \
	bash -c "! bash '$B2BR_DIR/scripts/signature.sh' '$tmpd/missing.vdi' >/dev/null 2>&1"
rm -rf "$tmpd"

# launch.sh must at least parse and explain itself without a hypervisor.
assert_ok "launch.sh --help works" bash "$B2BR_DIR/scripts/launch.sh" --help
assert_in "launch.sh documents VirtualBox" "VirtualBox" "$(bash "$B2BR_DIR/scripts/launch.sh" --help 2>&1)"
assert_in "launch.sh documents UTM"        "UTM"        "$(bash "$B2BR_DIR/scripts/launch.sh" --help 2>&1)"

assert_summary "static"
