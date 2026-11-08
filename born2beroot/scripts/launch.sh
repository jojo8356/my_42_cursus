#!/bin/bash
# =============================================================================
#  launch.sh — create the Born2beRoot virtual machine on the host
# =============================================================================
#  Run this on your own computer, not inside the machine.  It creates an empty
#  virtual machine with the disks described in config/partitions.layout, then
#  boots the Debian installer ISO so the partitioning can be done by hand (the
#  subject requires the layout to be created during the installation).
#
#  Usage:
#      scripts/launch.sh --iso <debian.iso> [--provider virtualbox|utm]
#                        [--name jojo8356] [--mem 2048] [--disk 20]
#                        [--snapshot|--delete|--status]
#
#  Providers:
#      virtualbox   Intel/AMD hosts (VirtualBox), the case the subject expects
#      utm          Apple Silicon hosts (UTM wraps QEMU)
#
#  After the Debian installation, copy this repository into the machine and run
#      sudo scripts/setup.sh --bonus
#      sudo scripts/verify.sh
# =============================================================================

set -euo pipefail

B2BR_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)
# shellcheck disable=SC1091
. "$B2BR_DIR/config/vm.conf"

PROVIDER="${B2BR_PROVIDER:-virtualbox}"
ISO=""
NAME="$VM_HOSTNAME"
MEM_MB="${VM_MEM_MB:-2048}"
DISK_GB="${VM_DISK_GB:-20}"
ACTION="create"

usage() {
	# Print the header: from the first comment after the title rule up to
	# (but excluding) the code below the closing rule.
	# Skip the shebang, the title line and the rule under it (3 lines), then
	# print every comment line until the closing rule.
	awk 'NR <= 4 { next }
	     /^# ={20,}[[:space:]]*$/ { exit }
	     !/^#/ { exit }
	     { sub(/^# ?/, ""); print }' "$0"
	exit "${1:-0}"
}

while [ $# -gt 0 ]; do
	case "$1" in
	--iso)      ISO="$2"; shift ;;
	--provider) PROVIDER="$2"; shift ;;
	--name)     NAME="$2"; shift ;;
	--mem)      MEM_MB="$2"; shift ;;
	--disk)     DISK_GB="$2"; shift ;;
	--snapshot) ACTION="snapshot" ;;
	--delete)   ACTION="delete" ;;
	--status)   ACTION="status" ;;
	-h|--help)  usage 0 ;;
	*)          printf 'unknown option: %s\n' "$1" >&2; usage 2 ;;
	esac
	shift
done

have() { command -v "$1" >/dev/null 2>&1; }

# --- VirtualBox --------------------------------------------------------------
virtualbox() {
	local vboxmanage
	vboxmanage=$(command -v VBoxManage || command -v vboxmanage || true)
	if [ -z "$vboxmanage" ]; then
		printf 'VBoxManage not found: install VirtualBox first (see docs/).\n' >&2
		exit 1
	fi
	case "$ACTION" in
	snapshot)
		"$vboxmanage" snapshot "$NAME" take "before-defence-$(date +%Y%m%d-%H%M)" \
			--description "clean state before the defence"
		printf 'snapshot taken\n'
		return 0 ;;
	delete)
		"$vboxmanage" unregistervm "$NAME" --delete
		printf 'machine %s deleted\n' "$NAME"
		return 0 ;;
	status)
		"$vboxmanage" showvminfo "$NAME" --machinereadable \
			| grep -E '^(name|memory|SATA-0-0|VMState)='
		return 0 ;;
	esac

	[ -n "$ISO" ] || { printf -- '--iso is required to create the machine\n' >&2; exit 2; }
	[ -f "$ISO" ] || { printf 'ISO not found: %s\n' "$ISO" >&2; exit 2; }

	printf 'Creating VirtualBox machine %s (%s MiB RAM, %s GiB disk)\n' \
		"$NAME" "$MEM_MB" "$DISK_GB"
	"$vboxmanage" createvm --name "$NAME" --ostype Debian_64 --register
	"$vboxmanage" modifyvm "$NAME" \
		--memory "$MEM_MB" --cpus 2 --vram 16 \
		--nic1 nat --nictype1 82540EM \
		--audio none --usb off --graphicscontroller vmsvga
	# Port forwarding: SSH 4242 is the only port the mandatory part exposes.
	"$vboxmanage" modifyvm "$NAME" \
		--natpf1 "ssh,tcp,,${SSH_PORT},,${SSH_PORT}"
	"$vboxmanage" createmedium disk --filename "$HOME/VirtualBox VMs/$NAME/$NAME.vdi" \
		--size $((DISK_GB * 1024)) --format VDI --variant Standard
	"$vboxmanage" storagectl "$NAME" --name SATA --add sata --controller IntelAhci
	"$vboxmanage" storageattach "$NAME" --storagectl SATA --port 0 --device 0 \
		--type hdd --medium "$HOME/VirtualBox VMs/$NAME/$NAME.vdi"
	"$vboxmanage" storagectl "$NAME" --name IDE --add ide
	"$vboxmanage" storageattach "$NAME" --storagectl IDE --port 1 --device 0 \
		--type dvddrive --medium "$ISO"
	"$vboxmanage" modifyvm "$NAME" --boot1 dvd --boot2 disk --boot3 none --boot4 none

	cat <<INFO

Machine created.  Now:

  1. start it:            VBoxManage startvm "$NAME"
  2. install Debian 12 (no graphical environment, but install SSH server)
  3. partition it exactly like config/partitions.layout:
         /boot            1 GiB   (ext4, primary)
         extended partition      (the rest)
         sda5             LUKS2, then LVM group $LVM_VG with:
           root        6 GiB     /
           swap        2 GiB
           home        4 GiB     /home
           var         2 GiB     /var
           srv         1 GiB     /srv
           tmp         1 GiB     /tmp
           var--log    2.5 GiB   /var/log
  4. inside the machine:  sudo scripts/setup.sh --bonus
  5. on the host:         scripts/signature.sh "$HOME/VirtualBox VMs/$NAME/$NAME.vdi"
INFO
}

# --- UTM (Apple Silicon) -----------------------------------------------------
utm() {
	if ! have utmctl; then
		printf 'utmctl not found: UTM can also be driven from its GUI.\n' >&2
		printf 'Create a Debian 12 VM with a %s GiB disk, then install:\n' "$DISK_GB"
		printf '  sudo scripts/setup.sh --bonus\n'
		exit 1
	fi
	case "$ACTION" in
	status) utmctl list ;;
	snapshot) printf 'UTM snapshots are taken from the GUI (Snapshots pane).\n' ;;
	delete) utmctl delete "$NAME" ;;
	*)
		[ -n "$ISO" ] || { printf -- '--iso is required\n' >&2; exit 2; }
		utmctl create "$NAME" --iso "$ISO" --memory "$MEM_MB" --disk "$DISK_GB"
		printf 'machine %s created; finish the install in UTM\n' "$NAME" ;;
	esac
}

case "$PROVIDER" in
virtualbox|vbox) virtualbox ;;
utm)             utm ;;
*)
	printf 'unknown provider: %s (expected virtualbox or utm)\n' "$PROVIDER" >&2
	exit 2 ;;
esac
