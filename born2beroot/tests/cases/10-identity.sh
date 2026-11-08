#!/bin/bash
# =============================================================================
#  tests/cases/10-identity.sh — hostname, users, groups, no GUI
# =============================================================================
#  Subject v5.2, chapter V:
#    * the hostname must be <login>42
#    * the login must belong to the sudo and user42 groups
#    * no graphical server may be installed (otherwise the grade is 0)
#    * the default systemd target must be multi-user
# =============================================================================

set -uo pipefail

B2BR_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)
. "$B2BR_DIR/tests/lib/assert.sh"

# The case files run *inside* the emulated machine: tests/vm.sh boots the image
# and passes this script through.  B2BR_IN_VM is set by the orchestrator.
if [ "${B2BR_IN_VM:-0}" != "1" ]; then
	printf 'this case must be run through tests/vm.sh\n' >&2
	exit 2
fi

. "$B2BR_DIR/config/vm.conf"

# --- Hostname ----------------------------------------------------------------
assert_eq "hostname is $VM_HOSTNAME"   "$VM_HOSTNAME"   "$(hostname)"
assert_eq "static hostname agrees"     "$VM_HOSTNAME"   "$(cat /etc/hostname)"
assert_in "/etc/hosts maps 127.0.1.1"  "$VM_HOSTNAME"   "$(awk '$1=="127.0.1.1"' /etc/hosts)"
assert_eq "hostname -f resolves"       "$VM_HOSTNAME"   "$(hostname -f 2>/dev/null)"

# --- The evaluator's account -------------------------------------------------
assert_ok "$VM_LOGIN exists"           id "$VM_LOGIN"
assert_in "$VM_LOGIN is in sudo"       "sudo"          "$(id -nG "$VM_LOGIN")"
assert_in "$VM_LOGIN is in user42"     "user42"        "$(id -nG "$VM_LOGIN")"
assert_ok "group user42 exists"        getent group user42
assert_ne "$VM_LOGIN's uid is not 0"   "0"             "$(id -u "$VM_LOGIN")"

# --- No graphical server -----------------------------------------------------
# "It is therefore forbidden to install X.org, Wayland, or any other equivalent
#  graphics server. Otherwise, your grade will be 0."
for pkg in xserver-xorg wayland weston gnome-shell gdm3 lightdm sddm; do
	if dpkg -s "$pkg" >/dev/null 2>&1; then
		assert_fail "no $pkg installed" "$pkg is installed: grade would be 0"
	else
		assert_ok "no $pkg installed"
	fi
done

assert_eq "default target is multi-user" "multi-user.target" "$(systemctl get-default 2>/dev/null)"
assert_ok "no display manager enabled"  bash -c '! systemctl is-enabled gdm3 >/dev/null 2>&1'
assert_ok "dbus-run-session present (proof of a headless install)" \
	test -x /usr/bin/dbus-run-session

# --- AppArmor ----------------------------------------------------------------
assert_ok "apparmor is enabled"        systemctl is-enabled apparmor
assert_ok "apparmor is active"         systemctl is-active apparmor

assert_summary "identity"
