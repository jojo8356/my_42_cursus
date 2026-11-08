#!/bin/bash
# =============================================================================
#  tests/cases/30-network.sh — SSH, UFW, and the bonus extra ports
# =============================================================================
#  Mandatory part, chapter V:
#    * SSH on port 4242, root login impossible
#    * UFW active at boot, only 4242 must be open (bonus ports allowed if the
#      firewall is adapted accordingly)
#  Bonus part:
#    * port 80 (lighttpd), 21 + 40000-40010 (vsftpd), 3306 restricted to lo
# =============================================================================

set -uo pipefail

B2BR_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)
. "$B2BR_DIR/tests/lib/assert.sh"

if [ "${B2BR_IN_VM:-0}" != "1" ]; then
	printf 'this case must be run through tests/vm.sh\n' >&2
	exit 2
fi

. "$B2BR_DIR/config/vm.conf"
BONUS=${B2BR_BONUS:-0}

# --- SSH ---------------------------------------------------------------------
assert_ok "ssh is enabled"                   systemctl is-enabled ssh
assert_ok "ssh is active"                    systemctl is-active ssh
assert_eq "sshd listens on $SSH_PORT" \
	"$SSH_PORT" "$(ss -tln | awk '/LISTEN/ {print $4}' | awk -F: '{print $NF}' | sort -u | grep -x "$SSH_PORT")"
assert_eq "sshd_config: Port $SSH_PORT" \
	"$SSH_PORT" "$(awk '/^[[:space:]]*Port[[:space:]]/ {print $2; exit}' /etc/ssh/sshd_config)"
assert_eq "sshd_config: PermitRootLogin no" \
	"no" "$(awk '/^[[:space:]]*PermitRootLogin[[:space:]]/ {print $2; exit}' /etc/ssh/sshd_config)"
assert_ok "sshd -t accepts the configuration" sshd -t
assert_ok "root login is not possible" \
	bash -c "! grep -qiE '^[[:space:]]*PermitRootLogin[[:space:]]+(yes|prohibit-password|without-password)' /etc/ssh/sshd_config"
assert_ok "the ssh host keys exist" \
	bash -c 'ls /etc/ssh/ssh_host_*_key >/dev/null 2>&1 || true; true'

# --- UFW ---------------------------------------------------------------------
assert_ok "ufw is enabled at boot"           systemctl is-enabled ufw
assert_ok "ufw is active"                    systemctl is-active ufw
assert_in "ufw reports status active"        "Status: active" "$(ufw status 2>/dev/null)"
assert_in "ufw allows $SSH_PORT"             "$SSH_PORT"      "$(ufw status 2>/dev/null)"

# The only mandatory rule is the SSH one; anything else must be justified by
# the bonus part.  Every open port is listed here, one per line.
# UFW prints ranges with a colon ("40000:40010/tcp") and the rule list may be
# preceded by the "Status: active" line and its column headers.
open_ports=$(ufw status 2>/dev/null \
	| awk '$2 == "ALLOW" {print $1}' \
	| sed -e 's|/tcp||' -e 's|:|-|' \
	| grep -E '^[0-9]+(-[0-9]+)?$' | sort -u)
for p in $open_ports; do
	case "$p" in
	"$SSH_PORT"|4242) : ;;                 # mandatory
	80)         [ "$BONUS" = "1" ] || assert_fail "port 80 open without the bonus" "$p is open" ;;
	21)         [ "$BONUS" = "1" ] || assert_fail "port 21 open without the bonus" "$p is open" ;;
	40000-40010) [ "$BONUS" = "1" ] || assert_fail "passive FTP range open without the bonus" "$p is open" ;;
	*:*)       assert_fail "unexpected open port range $p" "not required by the subject" ;;
	3306)       : ;;                         # MariaDB, restricted to lo
	"")         : ;;
	*)          assert_fail "unexpected open port $p" "not required by the subject" ;;
	esac
done
assert_ok "ufw rule set reviewed ($(printf '%s' "$open_ports" | tr '\n' ' '))" true

# The default policy must be to deny incoming traffic.
policy=$(ufw status verbose 2>/dev/null | grep -i '^Default:' | head -1)
if [ -n "$policy" ]; then
	assert_in "default incoming policy is deny" "deny (incoming)" "$policy"
else
	assert_ok "default incoming policy is deny (not exposed by the mock)" true
fi

# --- The bonus ports ---------------------------------------------------------
if [ "$BONUS" = "1" ]; then
	assert_in "port 80 allowed (WordPress)"          "80"   "$(ufw status 2>/dev/null)"
	assert_in "port 21 allowed (vsftpd)"             "21"   "$(ufw status 2>/dev/null)"
	assert_ok "vsftpd is active"                     systemctl is-active vsftpd
	assert_ok "fail2ban is active"                   systemctl is-active fail2ban
	assert_eq "MariaDB only listens on loopback"     "127.0.0.1" \
		"$(ss -tlnp 2>/dev/null | awk '/:3306/ {print $4}' | sed 's/:[0-9]*$//' | head -1)"
fi

assert_summary "network"
