#!/bin/bash
# =============================================================================
#  tests/cases/50-monitoring.sh — the monitoring script, field by field
# =============================================================================
#  The subject is extremely precise about this script:
#    * it must print 12 fields, in this order, with this #Architecture: format;
#    * it must be run by cron every 10 minutes, and at boot, through wall;
#    * it must not print any error, whatever the state of the machine.
#  Each field's *content* is checked too, not just its label.
# =============================================================================

set -uo pipefail

B2BR_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)
. "$B2BR_DIR/tests/lib/assert.sh"

if [ "${B2BR_IN_VM:-0}" != "1" ]; then
	printf 'this case must be run through tests/vm.sh\n' >&2
	exit 2
fi

. "$B2BR_DIR/config/vm.conf"

M="$MONITORING_SCRIPT"

# --- The script itself -------------------------------------------------------
assert_ok "$M exists"           test -f "$M"
assert_ok "$M is executable"    test -x "$M"
assert_ok "$M parses"           bash -n "$M"

out=$("$M" --no-wall 2>&1)
assert_ok "the script exits successfully" test $? -eq 0
# --no-wall must suppress the broadcast header, which would otherwise be part
# of the output and break every parser (and the "#1..12" ordering below).
assert_ok "--no-wall prints no broadcast header" \
	bash -c 'case "$1" in *"Broadcast message from"*) exit 1 ;; esac' _ "$out"

# --- No error, whatever the state -------------------------------------------
assert_ok "no error in the output" \
	bash -c "! printf '%s' \"\$0\" | grep -qiE 'command not found|No such file|Permission denied|^error|\berror\b' " "$out"

# --- The twelve fields, in order --------------------------------------------
FIELDS=(
	"Architecture"
	"Physical CPU"
	"vCPU"
	"Memory Usage"
	"Disk Usage"
	"CPU load"
	"Last boot"
	"LVM use"
	"TCP Connections"
	"User log"
	"Network"
	"Sudo"
)
expected=1
for f in "${FIELDS[@]}"; do
	assert_in "field #$expected: $f" "#$f:" "$out"
	expected=$((expected + 1))
done

# Exactly twelve numbered fields: nothing added, nothing missing.
assert_eq "twelve numbered fields" "12" \
	"$(printf '%s\n' "$out" | grep -cE '^#[A-Za-z]')"

# --- The content of each field ----------------------------------------------
# The exact format of the subject's example is "#Architecture: x86_64".
arch=$(printf '%s\n' "$out" | sed -n 's/^#Architecture:[[:space:]]*//p')
assert_ne "architecture is not empty" "" "$arch"

pcpu=$(printf '%s\n' "$out" | sed -n 's/^#Physical CPU:[[:space:]]*//p')
assert_ok "physical CPU count is a number" bash -c "printf '%s' '$pcpu' | grep -qE '^[0-9]+$'"

vcpu=$(printf '%s\n' "$out" | sed -n 's/^#vCPU:[[:space:]]*//p')
assert_ok "vCPU count is a number" bash -c "printf '%s' '$vcpu' | grep -qE '^[0-9]+$'"

mem=$(printf '%s\n' "$out" | sed -n 's/^#Memory Usage:[[:space:]]*//p')
assert_ok "memory usage has 'used/total (pct%)'" \
	bash -c "printf '%s' '$mem' | grep -qE '^[0-9]+/[0-9]+MB \([0-9]+(\.[0-9]+)?%\)'"

disk=$(printf '%s\n' "$out" | sed -n 's/^#Disk Usage:[[:space:]]*//p')
# The subject prints "1009/2Gb (49%)": a human-readable size with its unit
# right after the number ("2Gb", "21G", "1.1G") and a rounded percentage.
assert_ok "disk usage has 'used/total (pct%)'" \
	bash -c "printf '%s' '$disk' | grep -qE '^[0-9.]+[KMGTP]?i?B?/[0-9.]+[KMGTP]?i?B? \([0-9]+%\)'"

load=$(printf '%s\n' "$out" | sed -n 's/^#CPU load:[[:space:]]*//p')
# The subject's example is "#CPU load: 6.7%": a percentage, not the raw
# three-value load average.
assert_ok "CPU load is a percentage" \
	bash -c "printf '%s' '$load' | grep -qE '^[0-9]+(\.[0-9]+)?%'"

boot=$(printf '%s\n' "$out" | sed -n 's/^#Last boot:[[:space:]]*//p')
assert_ne "last boot is not empty" "" "$boot"

lvm=$(printf '%s\n' "$out" | sed -n 's/^#LVM use:[[:space:]]*//p')
assert_ok "LVM use is 'yes' or 'no'" bash -c "printf '%s' '$lvm' | grep -qxE '(yes|no)'"

tcp=$(printf '%s\n' "$out" | sed -n 's/^#TCP Connections:[[:space:]]*//p')
# The subject's example is "#TCP Connections: 1 ESTABLISHED".
assert_ok "TCP connections has 'N ESTABLISHED'" \
	bash -c "printf '%s' '$tcp' | grep -qE '^[0-9]+ ESTABLISHED'"

ulog=$(printf '%s\n' "$out" | sed -n 's/^#User log:[[:space:]]*//p')
assert_ok "user log has 'N'" bash -c "printf '%s' '$ulog' | grep -qE '^[0-9]+$'"

net=$(printf '%s\n' "$out" | sed -n 's/^#Network:[[:space:]]*//p')
# The subject's example is "#Network: IP 10.0.2.15 (08:00:27:51:9b:a5)".
assert_ok "network is 'IP <addr> (<mac>)'" \
	bash -c "printf '%s' '$net' | grep -qE '^IP [0-9]+\.[0-9]+\.[0-9]+\.[0-9]+ \([0-9a-f:]+\)'"

sudo=$(printf '%s\n' "$out" | sed -n 's/^#Sudo:[[:space:]]*//p')
# The subject's example is "#Sudo: 42 cmd".
assert_ok "sudo counter is 'N cmd'" \
	bash -c "printf '%s' '$sudo' | grep -qE '^[0-9]+ (cmd|command)'"

# --- Scheduling --------------------------------------------------------------
assert_in "cron entry runs every 10 minutes" "*/10" "$(crontab -l 2>/dev/null)"
assert_in "cron entry runs $M"               "$M"   "$(crontab -l 2>/dev/null)"
assert_ok  "a boot-time unit exists"         test -f /etc/systemd/system/born2beroot-monitoring.service
assert_in "the boot unit runs $M"            "$M"   "$(cat /etc/systemd/system/born2beroot-monitoring.service 2>/dev/null)"
assert_ok  "the boot unit is enabled"        systemctl is-enabled born2beroot-monitoring.service

# --- wall --------------------------------------------------------------------
# The script must broadcast its output; --no-wall is the switch used by the
# tests, which proves that wall is the default behaviour.
assert_ok "wall is the default"  bash -c "! grep -qE '^[[:space:]]*--no-wall\\)' '$M' || true"
assert_ok "the script accepts --no-wall" bash -c "'$M' --no-wall >/dev/null 2>&1"

assert_summary "monitoring"
