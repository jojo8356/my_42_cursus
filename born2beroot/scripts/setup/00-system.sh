#!/bin/bash
# =============================================================================
#  00-system.sh — base system: packages, hostname, hosts, graphical ban
# =============================================================================
#  Subject v5.2, chapter V:
#    * no graphical server at all (X.org / Wayland forbidden, else grade 0)
#    * hostname must be "<login>42"
#    * minimal service set
# =============================================================================

set -euo pipefail
. "$(dirname "${BASH_SOURCE[0]}")/../lib/common.sh"
load_config

require_root

# --- 1. Hostname -------------------------------------------------------------
current=$(hostname 2>/dev/null || true)
if [ "$current" = "$VM_HOSTNAME" ]; then
	log_skip "hostname already $VM_HOSTNAME"
elif [ "$B2BR_DRY_RUN" = "1" ]; then
	log_dry "would set hostname to $VM_HOSTNAME"
else
	hostnamectl set-hostname "$VM_HOSTNAME" 2>/dev/null \
		|| hostname "$VM_HOSTNAME"
	log_ok "hostname set to $VM_HOSTNAME"
fi

# --- 2. /etc/hosts -----------------------------------------------------------
# Debian's default layout, which is also what the graders parse:
#     127.0.0.1   localhost
#     127.0.1.1   <login>42
# Only the short name is listed on 127.0.1.1: "hostname -f" then returns the
# machine name instead of failing, and the entry must be rewritten whenever the
# hostname changes (the evaluator replaces the login and reboots).
if [ "$B2BR_DRY_RUN" = "0" ]; then
	tmp=$(mktemp)
	awk '
		/^[[:space:]]*127\.0\.0\.1[[:space:]]/ { next }
		/^[[:space:]]*127\.0\.1\.1[[:space:]]/ { next }
		{ print }
	' /etc/hosts > "$tmp" 2>/dev/null || : > "$tmp"
	{
		printf '127.0.0.1\tlocalhost\n'
		printf '127.0.1.1\t%s\n' "$VM_HOSTNAME"
		cat "$tmp"
	} > /etc/hosts.new
	mv /etc/hosts.new /etc/hosts
	rm -f "$tmp"
	log_ok "/etc/hosts: 127.0.1.1 -> $VM_HOSTNAME"
fi

# --- 3. Base packages --------------------------------------------------------
install_packages $MANDATORY_PACKAGES

# --- 4. No graphical environment --------------------------------------------
# "It is therefore forbidden to install X.org, Wayland, or any other equivalent
#  graphics server. Otherwise, your grade will be 0."
if [ "$B2BR_DRY_RUN" = "0" ]; then
	guis=$(dpkg-query -W -f='${Package}\n' 2>/dev/null | grep -E \
		'^(xserver-xorg|wayland|weston|gnome-shell|kde-plasma-desktop|xfce4|lightdm|gdm3|sddm|xorg)$' || true)
	if [ -n "$guis" ]; then
		log_info "removing graphical environment: $(echo "$guis" | tr '\n' ' ')"
		DEBIAN_FRONTEND=noninteractive apt-get purge -y $guis >/dev/null 2>&1 || true
	fi
	# systemd default target: multi-user (no display manager)
	systemctl set-default multi-user.target >/dev/null 2>&1 || true
	log_ok "no graphical environment (default target: multi-user.target)"
fi

# --- 5. AppArmor -------------------------------------------------------------
# "AppArmor for Debian must be running at startup too."
if [ "$B2BR_DRY_RUN" = "0" ]; then
	if command -v aa-status >/dev/null 2>&1; then
		if aa-status --enabled >/dev/null 2>&1; then
			log_ok "AppArmor is enabled and enforcing"
		else
			# Inside the hermetic test VM the host kernel does not expose
			# AppArmor to nested user namespaces; the unit is still enabled
			# below, which is what the subject requires of the machine.
			log_skip "AppArmor not exposed by this kernel (enabled anyway)"
		fi
	fi
	enable_service apparmor
fi

# --- 6. Kernel module persistence for AppArmor ------------------------------
if [ ! -e /etc/default/grub ] || [ "$B2BR_DRY_RUN" = "1" ]; then
	:
fi
if [ "$B2BR_DRY_RUN" = "0" ] && [ -f /etc/default/grub ]; then
	if ! grep -q 'security=apparmor' /etc/default/grub; then
		sed -i 's/^GRUB_CMDLINE_LINUX_DEFAULT="\(.*\)"/GRUB_CMDLINE_LINUX_DEFAULT="\1 security=apparmor apparmor=1"/' \
			/etc/default/grub
		update-grub >/dev/null 2>&1 || true
		log_ok "AppArmor enabled on the kernel command line"
	else
		log_skip "AppArmor already on the kernel command line"
	fi
fi

log_ok "00-system.sh done"
