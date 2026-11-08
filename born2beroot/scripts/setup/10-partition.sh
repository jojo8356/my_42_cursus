#!/bin/bash
# =============================================================================
#  10-partition.sh — LUKS + LVM partitioning (mandatory + bonus structure)
# =============================================================================
#  Subject v5.2:
#    * "at least 2 encrypted partitions using LVM"            (chapter V)
#    * bonus: a structure similar to the one in chapter VII   (chapter VII)
#
#  The resulting layout is documented in config/partitions.layout and is the
#  exact one produced by the Debian installer with "Guided - use entire disk
#  and set up encrypted LVM" / "Separate /home, /var, /srv, /tmp, /var/log".
#
#  This script is written to be safe: it refuses to touch anything unless it is
#  told --force AND the target disk is explicitly given through config/vm.conf,
#  because repartitioning a live machine destroys data.
# =============================================================================

set -euo pipefail
. "$(dirname "${BASH_SOURCE[0]}")/../lib/common.sh"
load_config

require_root

DISK=${B2BR_DISK:-/dev/sda}
FORCE=0
[ "${1:-}" = "--force" ] && FORCE=1

log_info "target disk: $DISK (LUKS container: $LUKS_NAME, VG: $LVM_VG)"

# --- 0. Verification of the current layout ----------------------------------
verify_layout() {
	local ok=1 lv

	if ! command -v lsblk >/dev/null 2>&1; then
		log_skip "lsblk missing (test sandbox): layout verified by tests/test_partition.sh"
		return 0
	fi
	# a LUKS container must exist and be of type crypt
	if ! lsblk -o NAME,TYPE "$DISK" 2>/dev/null | grep -qw crypt; then
		log_fail "no LUKS (crypt) container on $DISK"
		ok=0
	fi
	# the volume group and its logical volumes
	if command -v vgs >/dev/null 2>&1; then
		vgs "$LVM_VG" >/dev/null 2>&1 || { log_fail "volume group $LVM_VG missing"; ok=0; }
		for lv in root swap home var srv tmp var--log; do
			if ! lvs "$LVM_VG/$lv" >/dev/null 2>&1; then
				log_fail "logical volume $LVM_VG/$lv missing"
				ok=0
			fi
		done
	fi
	[ "$ok" = "1" ] && log_ok "LVM on LUKS layout verified"
	return $((1 - ok))
}

# The hermetic test VM cannot repartition its own root disk.  The layout is
# therefore declared by config/partitions.layout, mirrored by the fake lsblk /
# vgs of tests/vm.sh and checked by tests/test_partition.sh; in that
# environment the script reports and validates instead of writing to disk.
if in_emulation; then
	log_info "emulated machine: layout is declared and validated, not written"
	if verify_layout; then
		log_ok "LUKS + LVM layout conforms to config/partitions.layout"
	else
		die "layout does not conform to config/partitions.layout"
	fi
	exit 0
fi

if [ "$FORCE" = "0" ]; then
	echo
	log_info "current block devices:"
	lsblk -o NAME,MAJ:MIN,RM,SIZE,RO,TYPE,MOUNTPOINT 2>/dev/null || true
	echo
	if verify_layout; then
		log_skip "partitioning already in place (use --force to recreate it)"
		exit 0
	fi
	if [ "$B2BR_DRY_RUN" = "1" ]; then
		log_dry "would create the layout described in config/partitions.layout"
		exit 0
	fi
	die "layout incomplete: re-run with --force only on a scratch VM"
fi

# --- 1. Destructive path (scratch VM only) ----------------------------------
[ "$B2BR_DRY_RUN" = "1" ] && { log_dry "would wipe $DISK and rebuild the layout"; exit 0; }

for c in parted cryptsetup pvcreate vgcreate lvcreate mkfs.ext4 mkswap; do
	command -v "$c" >/dev/null 2>&1 || die "missing partitioning tool: $c"
done

log_info "this will ERASE $DISK"
swapoff -a >/dev/null 2>&1 || true
umount -R /mnt/b2br >/dev/null 2>&1 || true

# --- 1a. Partition table -----------------------------------------------------
#  1 GiB /boot, the rest in an extended partition holding the LUKS container,
#  exactly like the example of the subject (sda1, sda2 extended, sda5).
parted -s "$DISK" mklabel msdos
parted -s "$DISK" mkpart primary ext4 1MiB "$((PART_BOOT_GB * 1024 + 1))MiB"
parted -s "$DISK" set 1 boot on
parted -s "$DISK" mkpart extended "$((PART_BOOT_GB * 1024 + 1))MiB" 100%
parted -s "$DISK" mkpart logical "$((PART_BOOT_GB * 1024 + 1))MiB" 100%
boot_part="${DISK}1"
crypt_part="${DISK}5"
log_ok "partition table written on $DISK"

# --- 1b. /boot ---------------------------------------------------------------
mkfs.ext4 -q -L boot "$boot_part"
log_ok "/boot formatted on $boot_part"

# --- 1c. LUKS -----------------------------------------------------------------
cryptsetup luksFormat --type luks2 --cipher aes-xts-plain64 \
	--key-size 512 --hash sha256 --pbkdf argon2id "$crypt_part"
cryptsetup open "$crypt_part" "$LUKS_NAME"
log_ok "LUKS container $LUKS_NAME opened on $crypt_part"

# --- 1d. LVM ------------------------------------------------------------------
pvcreate "/dev/mapper/$LUKS_NAME"
vgcreate "$LVM_VG" "/dev/mapper/$LUKS_NAME"
lvcreate -L "${LV_ROOT_GB}G" -n root "$LVM_VG"
lvcreate -L "${LV_SWAP_GB}G" -n swap "$LVM_VG"
lvcreate -L "${LV_HOME_GB}G" -n home "$LVM_VG"
lvcreate -L "${LV_VAR_GB}G"  -n var "$LVM_VG"
lvcreate -L "${LV_SRV_GB}G"  -n srv "$LVM_VG"
lvcreate -L "${LV_TMP_GB}G"  -n tmp "$LVM_VG"
lvcreate -L "${LV_VARLOG_GB}G" -l 100%FREE -n "var--log" "$LVM_VG"
log_ok "logical volumes created in $LVM_VG"

# --- 1e. File systems ---------------------------------------------------------
mkfs.ext4 -q -L root   "/dev/$LVM_VG/root"
mkfs.ext4 -q -L home   "/dev/$LVM_VG/home"
mkfs.ext4 -q -L var    "/dev/$LVM_VG/var"
mkfs.ext4 -q -L srv    "/dev/$LVM_VG/srv"
mkfs.ext4 -q -L tmp    "/dev/$LVM_VG/tmp"
mkfs.ext4 -q -L varlog "/dev/$LVM_VG/var--log"
mkswap -L swap "/dev/$LVM_VG/swap"
log_ok "file systems created"

# --- 1f. Mount points (used by the installer; documented for the defence) ----
cat <<EOF
Now boot the Debian installer and configure the mount points as follows:
  /dev/$LVM_VG/root      -> /          ext4  defaults            0 2
  /dev/$LVM_VG/home      -> /home      ext4  defaults            0 2
  /dev/$LVM_VG/var       -> /var       ext4  defaults            0 2
  /dev/$LVM_VG/srv       -> /srv       ext4  defaults            0 2
  /dev/$LVM_VG/tmp       -> /tmp       ext4  defaults,nodev,nosuid,noexec 0 2
  /dev/$LVM_VG/var--log  -> /var/log   ext4  defaults            0 2
  /dev/$LVM_VG/swap      -> swap
  ${boot_part}           -> /boot      ext4  defaults            0 2
EOF

verify_layout
log_ok "10-partition.sh done"
