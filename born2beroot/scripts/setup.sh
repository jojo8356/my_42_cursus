#!/bin/bash
# =============================================================================
#  setup.sh — orchestrate the whole Born2beRoot provisioning
# =============================================================================
#  Usage:
#     sudo ./scripts/setup.sh                # mandatory part only
#     sudo ./scripts/setup.sh --bonus        # mandatory + bonus parts
#     sudo ./scripts/setup.sh --dry-run      # render what would be done
#     sudo ./scripts/setup.sh --list         # list the steps
#
#  Every step is idempotent: running the script twice is harmless, which is the
#  behaviour the subject expects during the defence.
# =============================================================================

set -euo pipefail
. "$(dirname "${BASH_SOURCE[0]}")/lib/common.sh"
load_config

WITH_BONUS=0
for arg in "$@"; do
	case "$arg" in
	--bonus)   WITH_BONUS=1 ;;
	--dry-run) export B2BR_DRY_RUN=1 ;;
	--quiet)   export B2BR_QUIET=1 ;;
	--list)    LIST_ONLY=1 ;;
	-h|--help)
		sed -n '2,20p' "$0" | sed 's/^# \{0,1\}//'
		exit 0 ;;
	*) die "unknown option: $arg (try --help)" ;;
	esac
done

STEPS=(
	"00-system.sh:base system, hostname, no graphical environment, AppArmor"
	"10-partition.sh:LUKS + LVM layout (at least 2 encrypted partitions)"
	"20-users.sh:the 42 login, user42/sudo groups, password ageing"
	"30-ssh.sh:SSH on port 4242, root login forbidden"
	"40-ufw.sh:UFW active, only port 4242 open"
	"50-password.sh:strong password policy"
	"60-sudo.sh:strict sudo configuration"
	"70-monitoring.sh:monitoring.sh + cron + boot unit"
)
BONUS_STEPS=(
	"80-bonus-wordpress.sh:WordPress with lighttpd, MariaDB and PHP"
	"90-bonus-service.sh:vsftpd + fail2ban (service of my choice)"
)

if [ "${LIST_ONLY:-0}" = "1" ]; then
	printf 'Mandatory steps:\n'
	for s in "${STEPS[@]}"; do printf '  %-24s %s\n' "${s%%:*}" "${s#*:}"; done
	printf '\nBonus steps:\n'
	for s in "${BONUS_STEPS[@]}"; do printf '  %-24s %s\n' "${s%%:*}" "${s#*:}"; done
	exit 0
fi

require_root

# Inside the hermetic test VM the tree is mounted read-only at /opt/born2beroot.
if [ -d /opt/born2beroot ] && [ ! -w "$B2BR_DIR" ]; then
	B2BR_ROOT=/opt/born2beroot
	B2BR_CONFIG="$B2BR_ROOT/config"
	B2BR_SCRIPTS="$B2BR_ROOT/scripts"
	export B2BR_ROOT B2BR_CONFIG B2BR_SCRIPTS
fi

printf '%s\n' "============================================================"
printf ' Born2beRoot — provisioning %s\n' "$VM_HOSTNAME ($VM_OS $VM_OS_VERSION/$VM_OS_CODENAME)"
printf ' login=%s  bonus=%s  dry-run=%s\n' "$VM_LOGIN" \
	"$([ "$WITH_BONUS" = 1 ] && echo yes || echo no)" "$B2BR_DRY_RUN"
printf '%s\n' "============================================================"

# The bonus flag is exported so that the steps which share a requirement with
# the mandatory part (the firewall, in particular) know whether they may open
# the extra ports: "leave only port 4242 open" must hold when --bonus is absent.
export B2BR_BONUS="$WITH_BONUS"

run_step() {
	local spec="$1" script="${1%%:*}" label="${1#*:}"
	[ -x "$B2BR_SCRIPTS/setup/$script" ] || die "missing step: $script"
	printf '\n--- %s — %s\n' "$script" "$label"
	"$B2BR_SCRIPTS/setup/$script" || die "step $script failed"
}

for step in "${STEPS[@]}"; do
	run_step "$step"
done

if [ "$WITH_BONUS" = "1" ]; then
	for step in "${BONUS_STEPS[@]}"; do
		run_step "$step"
	done
fi

printf '\n%s\n' "============================================================"
printf ' Provisioning complete.  Next steps:\n'
printf '   1. sudo passwd %s          (set a compliant password)\n' "$VM_LOGIN"
printf '   2. sudo ./scripts/verify.sh  (self-check before the defence)\n'
printf '%s\n' "============================================================"
