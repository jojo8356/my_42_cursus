#!/bin/bash
# =============================================================================
#  tests/cases/20-partitions.sh — LUKS container, LVM groups, mount points
# =============================================================================
#  Mandatory part: "at least 2 encrypted partitions using LVM".
#  Bonus part:     "create at least 7 partitions with their mount points".
#
#  The expected layout is not duplicated here: it is read from
#  config/partitions.layout, the single source of truth shared by the setup
#  scripts, the verification script and this test.
# =============================================================================

set -uo pipefail

B2BR_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)
. "$B2BR_DIR/tests/lib/assert.sh"

if [ "${B2BR_IN_VM:-0}" != "1" ]; then
	printf 'this case must be run through tests/vm.sh\n' >&2
	exit 2
fi

. "$B2BR_DIR/config/vm.conf"

# --- The encrypted container -------------------------------------------------
assert_in "lsblk reports a crypt container" "crypt" "$(lsblk -o TYPE)"
assert_in "the container is $LUKS_NAME"      "$LUKS_NAME" "$(lsblk -o NAME)"
assert_in "LUKS2 is used"                    "LUKS2" "$(cryptsetup status "$LUKS_NAME" 2>/dev/null)"
assert_in "the container sits on /dev/sda5"  "sda5" "$(lsblk -o NAME)"

# --- The volume group --------------------------------------------------------
assert_ok "volume group $LVM_VG exists"  vgs "$LVM_VG"
assert_in "lvs lists $LVM_VG"           "$LVM_VG" "$(lvs 2>/dev/null)"

# --- Every logical volume declared in the layout -----------------------------
#  sda1        /boot   (plain ext4, outside the LUKS container)
#  LVMGroup-*  the seven volumes the bonus part requires
assert_in "sda1 is mounted on /boot"     "/boot" "$(findmnt -no TARGET /boot 2>/dev/null)"
assert_in "/ is on the LVM volume"       "$LVM_VG-root" "$(findmnt -no SOURCE / 2>/dev/null)"

while read -r name target; do
	[ -n "$name" ] || continue
	assert_in "$name exists"             "$name"   "$(lsblk -o NAME)"
done <<EOF
$(awk '/LVMGroup-/ { for (i=1;i<=NF;i++) if ($i ~ /^LVMGroup-/) { n=$i; sub(/^[|`-]+/, "", n); print n } }' \
	"$B2BR_DIR/config/partitions.layout" | sort -u | while read -r lv; do
		printf '%s\n' "$lv"
	done | head -20)
EOF

# --- The mount points of the bonus layout ------------------------------------
#  The subject's bonus asks for 7 partitions "with their mount points", so each
#  logical volume must appear in the mount table with the right target.
for pair in "$LVM_VG-home:/home" "$LVM_VG-var:/var" "$LVM_VG-srv:/srv" \
	"$LVM_VG-tmp:/tmp" "$LVM_VG-var--log:/var/log"; do
	lv=${pair%%:*}; mp=${pair##*:}
	assert_in "$lv mounted on $mp" "$mp" "$(findmnt -no TARGET "/dev/mapper/$lv" 2>/dev/null)"
done

assert_in "swap volume is active" "swap" "$(lsblk -o NAME,TYPE 2>/dev/null)"

# --- Mount options that matter ----------------------------------------------
assert_in "/tmp is mounted noexec"   "noexec" "$(findmnt -no OPTIONS /tmp 2>/dev/null)"
assert_in "/tmp is mounted nosuid"   "nosuid" "$(findmnt -no OPTIONS /tmp 2>/dev/null)"
assert_in "/tmp is mounted nodev"    "nodev"  "$(findmnt -no OPTIONS /tmp 2>/dev/null)"

assert_summary "partitions"
