#!/bin/bash
# =============================================================================
#  verify.sh — one-shot self-check of every subject requirement on the VM
# =============================================================================
#  Prints one line per requirement with [ OK ] or [FAIL] and exits non-zero if
#  anything is wrong.  This is the script to run right before the defence, and
#  the same checks are exercised by tests/test_*.sh in the test suite.
# =============================================================================

set -uo pipefail
. "$(dirname "${BASH_SOURCE[0]}")/lib/common.sh"
load_config

PASS=0
FAIL=0

ok()   { PASS=$((PASS + 1)); printf '%s[ OK ]%s %s\n' "$C_OK"   "$C_OFF" "$*"; }
ko()   { FAIL=$((FAIL + 1)); printf '%s[FAIL]%s %s\n' "$C_ERR"  "$C_OFF" "$*"; }
head_() { printf '\n%s== %s ==%s\n' "$C_INFO" "$*" "$C_OFF"; }

check() { if eval "$2" >/dev/null 2>&1; then ok "$1"; else ko "$1"; fi; }

head_ "Identity"
check "hostname is $VM_HOSTNAME"            "[ \"\$(hostname)\" = \"$VM_HOSTNAME\" ]"
check "user $VM_LOGIN exists"               "id $VM_LOGIN"
check "$VM_LOGIN is in group sudo"          "id -nG $VM_LOGIN | grep -qw sudo"
check "$VM_LOGIN is in group user42"        "id -nG $VM_LOGIN | grep -qw user42"

head_ "No graphical environment"
check "no X.org / Wayland server installed" "! dpkg-query -W 2>/dev/null | grep -qE '^(xserver-xorg|wayland|weston|gnome-shell|gdm3|lightdm|sddm)'"
check "default target is multi-user"        "systemctl get-default | grep -c multi-user"

head_ "Block devices: LUKS + LVM"
check "a LUKS (crypt) container exists"     "lsblk -o TYPE | grep -qw crypt"
check "LVM volumes are visible"             "lsblk -o TYPE | grep -qw lvm"
check "volume group $LVM_VG exists"         "vgs $LVM_VG"
check "/ is on a logical volume"            "findmnt -no SOURCE / | grep -c '/dev/mapper/'"

head_ "SSH"
check "sshd is enabled"                     "systemctl is-enabled ssh | grep -c enabled"
check "sshd listens on port $SSH_PORT"      "ss -tln | grep -c \":$SSH_PORT \""
check "PermitRootLogin no"                  "grep -qE '^\s*PermitRootLogin\s+no' /etc/ssh/sshd_config"
check "Port $SSH_PORT in sshd_config"       "grep -qE '^[[:space:]]*Port[[:space:]]+$SSH_PORT[[:space:]]*$' /etc/ssh/sshd_config"
check "sshd_t: configuration valid"         "sshd -t"

head_ "Firewall"
check "UFW is active"                       "ufw status | grep -c 'Status: active'"
check "port $SSH_PORT allowed"              "ufw status | grep -qE '^$SSH_PORT(/tcp)?\s+ALLOW'"
check "UFW enabled at boot"                 "systemctl is-enabled ufw | grep -c enabled"

head_ "Password policy"
check "PASS_MAX_DAYS = $PASS_MAX_DAYS"      "grep -qE '^PASS_MAX_DAYS[[:space:]]+$PASS_MAX_DAYS[[:space:]]*$' /etc/login.defs"
check "PASS_MIN_DAYS = $PASS_MIN_DAYS"      "grep -qE '^PASS_MIN_DAYS[[:space:]]+$PASS_MIN_DAYS[[:space:]]*$' /etc/login.defs"
check "PASS_WARN_AGE = $PASS_WARN_AGE"      "grep -qE '^PASS_WARN_AGE[[:space:]]+$PASS_WARN_AGE[[:space:]]*$' /etc/login.defs"
check "minlen = $PASS_MIN_LEN"              "grep -qE 'minlen=$PASS_MIN_LEN|minlen = $PASS_MIN_LEN' /etc/security/pwquality.conf /etc/pam.d/common-password"
check "ucredit/lcredit/dcredit = -1"        "grep -qE 'ucredit=-1' /etc/pam.d/common-password"
check "maxrepeat = $PASS_MAX_REPEAT"        "grep -qE 'maxrepeat=$PASS_MAX_REPEAT|maxrepeat = $PASS_MAX_REPEAT' /etc/security/pwquality.conf /etc/pam.d/common-password"
check "difok = $PASS_DIFOK"                 "grep -qE 'difok=$PASS_DIFOK|difok = $PASS_DIFOK' /etc/security/pwquality.conf /etc/pam.d/common-password"
check "reject_username / usercheck"         "grep -qE 'usercheck=1|usercheck = 1|reject_username' /etc/security/pwquality.conf /etc/pam.d/common-password"
check "enforce_for_root"                    "grep -qE 'enforce_for_root' /etc/security/pwquality.conf /etc/pam.d/common-password"
check "chage of $VM_LOGIN: 2/30/7"          "chage -l $VM_LOGIN | grep -c 'Maximum number of days between password change.*30'"
check "chage of root: 2/30/7"               "chage -l root | grep -c 'Maximum number of days between password change.*30'"

head_ "sudo"
check "3 password tries"                    "grep -rqE 'passwd_tries=3' /etc/sudoers"
check "custom bad password message"         "grep -rqE 'badpass_message=' /etc/sudoers"
check "log_input and log_output"            "grep -cE 'log_input' /etc/sudoers && grep -cE 'log_output' /etc/sudoers"
check "log in /var/log/sudo"                "grep -rE 'logfile=\"/var/log/sudo/' /etc/sudoers"
check "/var/log/sudo exists"                "[ -d /var/log/sudo ]"
check "requiretty"                          "grep -rE 'requiretty' /etc/sudoers"
check "secure_path restricted"              "grep -cE '^Defaults[[:space:]]+secure_path' /etc/sudoers"
check "sudoers syntax valid"                "visudo -c"

head_ "Monitoring script"
check "$MONITORING_SCRIPT installed"        "[ -x $MONITORING_SCRIPT ]"
check "$MONITORING_SCRIPT syntax valid"     "bash -n $MONITORING_SCRIPT"
check "cron entry every 10 minutes"         "crontab -l -u root | grep -qE '^\*/10 \* \* \* \* .*monitoring.sh'"
check "runs at boot"                        "systemctl is-enabled born2beroot-monitoring.service"
if [ -x "$MONITORING_SCRIPT" ]; then
	out=$("$MONITORING_SCRIPT" --no-wall 2>&1)
	for field in Architecture 'Physical CPU' vCPU 'Memory Usage' 'Disk Usage' \
		'CPU load' 'Last boot' 'LVM use' 'TCP Connections' 'User log' Network Sudo; do
		if printf '%s' "$out" | grep -c "#$field:"; then
			ok "monitoring field #$field"
		else
			ko "monitoring field #$field missing"
		fi
	done
	if printf '%s' "$out" | grep -qiE 'error|command not found'; then
		ko "monitoring.sh printed an error"
	else
		ok "monitoring.sh prints no error"
	fi
fi

head_ "Bonus"
if dpkg -s lighttpd >/dev/null 2>&1; then
	check "lighttpd installed"              "dpkg -s lighttpd"
	check "MariaDB installed"               "dpkg -s mariadb-server"
	check "PHP installed"                   "dpkg -s php-cgi"
	check "WordPress files present"         "[ -f $BONUS_WP_DIR/wp-config.php ] || [ -f $BONUS_WP_DIR/index.php ]"
	check "lighttpd document root is $BONUS_WP_DIR" "grep -q \"document-root.*$BONUS_WP_DIR\" /etc/lighttpd/lighttpd.conf"
	check "lighttpd answers on port 80"     "curl -fsS http://127.0.0.1/ || ss -tln | grep -c ':80 '"
	check "no forbidden web server"         "! dpkg -s nginx >/dev/null 2>&1 && ! dpkg -s apache2 >/dev/null 2>&1"
	check "vsftpd installed"                "dpkg -s vsftpd"
	check "fail2ban installed"              "dpkg -s fail2ban"
	check "port 80 allowed in UFW"          "ufw status | grep -qE '^80(/tcp)?\s+ALLOW'"
else
	printf '%s[SKIP]%s bonus part not installed (run setup.sh --bonus)\n' "$C_WARN" "$C_OFF"
fi

printf '\n============================================================\n'
printf ' %d checks passed, %d failed\n' "$PASS" "$FAIL"
printf '============================================================\n'
[ "$FAIL" -eq 0 ]
