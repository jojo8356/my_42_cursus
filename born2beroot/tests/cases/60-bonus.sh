#!/bin/bash
# =============================================================================
#  tests/cases/60-bonus.sh — WordPress, the extra service, the adapted firewall
# =============================================================================
#  Chapter VII of the subject, in full:
#    * a partition layout with at least 7 partitions (checked in 20-partitions)
#    * WordPress, served by lighttpd, with MariaDB and PHP
#    * one extra service that is neither NGINX nor Apache2, and a justification
#    * the firewall adapted to the new ports
#    * the extra service must be useful (it is argued in the README)
# =============================================================================

set -uo pipefail

B2BR_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)
. "$B2BR_DIR/tests/lib/assert.sh"

if [ "${B2BR_IN_VM:-0}" != "1" ]; then
	printf 'this case must be run through tests/vm.sh\n' >&2
	exit 2
fi

. "$B2BR_DIR/config/vm.conf"

if [ "${B2BR_BONUS:-0}" != "1" ]; then
	printf '  (bonus part not installed: run with --bonus)\n'
	exit 0
fi

# --- WordPress ---------------------------------------------------------------
assert_ok "lighttpd is installed"       dpkg -s lighttpd
assert_ok "MariaDB is installed"        dpkg -s mariadb-server
assert_ok "PHP-CGI is installed"        dpkg -s php-cgi
assert_ok "PHP MySQL driver installed"  dpkg -s php-mysql

assert_in "the document root is $BONUS_WP_DIR" "$BONUS_WP_DIR" \
	"$(cat /etc/lighttpd/lighttpd.conf 2>/dev/null)"
assert_ok "index.php exists"            test -f "$BONUS_WP_DIR/index.php"
assert_ok "wp-config.php exists"        test -f "$BONUS_WP_DIR/wp-config.php"
assert_ok "wp-config.php names the database" \
	bash -c "grep -q \"$BONUS_WP_DB\" $BONUS_WP_DIR/wp-config.php"
assert_ok "wp-config.php is not world-readable" \
	bash -c "[ \"\$(stat -c %a $BONUS_WP_DIR/wp-config.php)\" -le 640 ]"

assert_ok "lighttpd is active"          systemctl is-active lighttpd
assert_ok "lighttpd is enabled at boot" systemctl is-enabled lighttpd
assert_ok "FastCGI is configured"       bash -c 'ls /etc/lighttpd/conf-enabled/*fastcgi* >/dev/null 2>&1'
assert_ok "http://127.0.0.1/ answers"   curl -fsS http://127.0.0.1/

# --- MariaDB -----------------------------------------------------------------
assert_ok "mariadb is active"           systemctl is-active mariadb
assert_ok "the database $BONUS_WP_DB exists" \
	bash -c "mariadb -N -B -e 'SHOW DATABASES' | grep -qx '$BONUS_WP_DB'"
assert_ok "MariaDB listens on loopback only" \
	bash -c "ss -tln 2>/dev/null | awk '/:3306/ {print \$4}' | grep -qvE '^(0\\.0\\.0\\.0|\\*|\\[::\\]):'"
assert_in "bind-address is 127.0.0.1" "127.0.0.1" \
	"$(cat /etc/mysql/mariadb.conf.d/60-born2beroot.cnf 2>/dev/null)"

# --- The extra service: vsftpd ----------------------------------------------
assert_ok "vsftpd is installed"         dpkg -s vsftpd
assert_ok "vsftpd is active"            systemctl is-active vsftpd
assert_ok "vsftpd is enabled at boot"   systemctl is-enabled vsftpd
assert_ok "vsftpd listens on $FTP_PORT" bash -c "ss -tln | grep -qE ':$FTP_PORT '"
assert_in "TLS is enabled"              "ssl_enable=YES" \
	"$(tr -d ' ' < /etc/vsftpd.conf 2>/dev/null)"
assert_in "each user is chrooted"       "chroot_local_user=YES" \
	"$(tr -d ' ' < /etc/vsftpd.conf 2>/dev/null)"
assert_ok "a TLS certificate was generated" test -s /etc/ssl/private/vsftpd.pem
assert_ok "anonymous access is disabled" \
	bash -c '! grep -qx "anonymous_enable=YES" /etc/vsftpd.conf'
assert_ok "no passwd file is used" \
	bash -c '! grep -qE "^(local_enable=YES.*)?(virtual_use_local_privs|pam_service_name=ftp)" /etc/vsftpd.conf'

# --- fail2ban ----------------------------------------------------------------
assert_ok "fail2ban is installed"       dpkg -s fail2ban
assert_ok "fail2ban is active"          systemctl is-active fail2ban
assert_ok "an sshd jail is configured" \
	bash -c 'grep -rq "sshd" /etc/fail2ban/jail.d/ 2>/dev/null'
assert_ok "a vsftpd jail is configured" \
	bash -c 'grep -rq "vsftpd" /etc/fail2ban/jail.d/ 2>/dev/null'
assert_in "fail2ban reports the sshd jail" "sshd" "$(fail2ban-client status 2>/dev/null)"

# --- The firewall is adapted, not disabled ----------------------------------
assert_in "port 80 is allowed"  "80"  "$(ufw status 2>/dev/null)"
assert_in "port 21 is allowed"  "21"  "$(ufw status 2>/dev/null)"
assert_in "the passive range is allowed" "40000" "$(ufw status 2>/dev/null)"
assert_in "the SSH port is still allowed" "$SSH_PORT" "$(ufw status 2>/dev/null)"

# --- The forbidden web servers are absent -----------------------------------
assert_ok "nginx is not installed"    bash -c '! dpkg -s nginx >/dev/null 2>&1'
assert_ok "apache2 is not installed"  bash -c '! dpkg -s apache2 >/dev/null 2>&1'

# --- Justification -----------------------------------------------------------
# The subject asks for the extra service to be justified.  The README carries
# the argument; this checks that it is actually written down and that it names
# the service, so the justification cannot be forgotten at the defence.
assert_ok "README justifies the extra service" \
	bash -c "grep -qi 'vsftpd' '$B2BR_DIR/README.md'"
assert_ok "README explains why vsftpd" \
	bash -c "grep -qiE 'why vsftpd' '$B2BR_DIR/README.md'"

assert_summary "bonus"
