#!/usr/bin/env bash
# =============================================================================
#  vm.sh — a *hermetic Born2beRoot VM* to run and test the project without
#          touching the host system
# =============================================================================
#  A Born2beRoot project is a configured machine: /etc/ssh/sshd_config,
#  /etc/sudoers.d/*, /etc/pam.d/common-password, the root crontab, the dpkg
#  database, a hostname, a firewall...  Testing it therefore requires a machine
#  whose /etc may be rewritten freely.
#
#  This script builds exactly that: inside private user + mount + UTS + PID
#  namespaces it mounts a fresh tmpfs on /etc, /var/log, /home, /srv, /media and
#  /var/www, imports a minimal Debian root filesystem (real passwd/group/shadow/
#  pam/sudo/ubuntu binaries are used unchanged), then runs the caller's command
#  as uid 0.  Nothing outside the workspace and the tmpfs is modified, and the
#  namespaces disappear when the command exits.
#
#  Usage:
#     tests/vm.sh --snapshot <dir> -- <command...>    build/reuse a VM image
#     tests/vm.sh --run <dir> -- <command...>         boot the image, run a cmd
#     tests/vm.sh --reset <dir>                       discard a dirty image
#
#  The image directory contains the persistent state of the virtual disk
#  (etc/, var/, srv/, pkgs/) plus a `state` file, which makes tests fast: build
#  once, boot many times, and reset when a test needs a pristine machine.
# =============================================================================

set -euo pipefail

B2BR_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)
BASE_IMAGE_DEFAULT="$B2BR_DIR/tests/.vm-base"

# -----------------------------------------------------------------------------
#  Host-side preparation of the base root filesystem
# -----------------------------------------------------------------------------
# The base image is built once from the running Debian userland.  Only the
# files an administrative project touches are copied, and everything is owned
# by the current user so that the unprivileged namespace root can rewrite it.
prepare_base() {
	local base="$1"

	[ -f "$base/.ready" ] && return 0
	rm -rf "$base"
	mkdir -p "$base/etc/default" "$base/var/log" "$base/var/lib/dpkg" \
		"$base/srv" "$base/home" "$base/www" "$base/root" "$base/opt" \
		"$base/run" "$base/media" "$base/tmp" \
		"$base/usr_local/bin" "$base/usr_local/sbin"

	# --- account database, PAM stack, sudo, PAM modules --------------------
	local f
	for f in passwd group shadow gshadow login.defs shells services \
		hostname hosts nsswitch.conf os-release debian_version; do
		[ -e "/etc/$f" ] || continue
		cp -L "/etc/$f" "$base/etc/$f" 2>/dev/null && continue
		# /etc/shadow and /etc/gshadow are root-only on the host: read them
		# through sudo and hand them to the current user so that the
		# unprivileged namespace root can rewrite them.
		if command -v sudo >/dev/null 2>&1; then
			sudo -n cat "/etc/$f" > "$base/etc/$f" 2>/dev/null || rm -f "$base/etc/$f"
			sudo -n chown "$(id -u):$(id -g)" "$base/etc/$f" 2>/dev/null || true
		fi
	done
	chmod 600 "$base/etc/shadow" "$base/etc/gshadow" 2>/dev/null || true
	chmod 440 "$base/etc/sudoers" 2>/dev/null || true
	for d in pam.d security skel default sudoers.d; do
		[ -d "/etc/$d" ] && cp -RL "/etc/$d" "$base/etc/$d" 2>/dev/null || true
	done
	# /etc/ssl/openssl.cnf is the configuration file of every TLS operation.
	[ -d /etc/ssl ] && cp -RL /etc/ssl "$base/etc/ssl" 2>/dev/null || true
	# /etc/alternatives holds the symlink targets of /usr/bin/{awk,cc,...};
	# without it every "update-alternatives" link dangles inside the tmpfs.
	[ -d /etc/alternatives ] && cp -RL /etc/alternatives "$base/etc/alternatives" 2>/dev/null || true
	# /etc/sudoers is root-only as well.
	if [ -e /etc/sudoers ]; then
		cp -L /etc/sudoers "$base/etc/sudoers" 2>/dev/null || {
			sudo -n cat /etc/sudoers > "$base/etc/sudoers" 2>/dev/null || rm -f "$base/etc/sudoers"
			sudo -n chown "$(id -u):$(id -g)" "$base/etc/sudoers" 2>/dev/null || true
		}
	fi
	[ -e /etc/sudo.conf ] && cp -L /etc/sudo.conf "$base/etc/sudo.conf" 2>/dev/null || true
	# The PAM modules themselves are binaries: copy the whole directory so
	# pam_pwquality.so can really be loaded (its absence is a project failure).
	[ -d /usr/lib/x86_64-linux-gnu/security ] && \
		mkdir -p "$base/usr/lib/x86_64-linux-gnu" && \
		cp -a /usr/lib/x86_64-linux-gnu/security "$base/usr/lib/x86_64-linux-gnu/" 2>/dev/null || true

	# --- an empty, valid dpkg database ------------------------------------
	printf '' > "$base/var/lib/dpkg/status"
	mkdir -p "$base/var/lib/dpkg/info" "$base/var/lib/dpkg/updates"
	for f in available diversions statoverride; do
		: > "$base/var/lib/dpkg/$f"
	done

	chmod -R u+rwX "$base"
	touch "$base/.ready"
}

# -----------------------------------------------------------------------------
#  Host-side creation of a VM image from the base
# -----------------------------------------------------------------------------
new_image() {
	local dir="$1"

	rm -rf "$dir"
	mkdir -p "$dir"
	cp -a "$BASE_IMAGE_DEFAULT/." "$dir/"
	mkdir -p "$dir/etc" "$dir/var/log" "$dir/srv" "$dir/www"
	# Start with a clean machine: no project file may survive.
	rm -f "$dir/etc/ssh/sshd_config" 2>/dev/null || true
	rm -rf "$dir/etc/sudoers.d" "$dir/etc/lighttpd" "$dir/etc/vsftpd.conf" \
		"$dir/etc/fail2ban" "$dir/etc/mysql" 2>/dev/null || true
	printf 'fresh\n' > "$dir/state"
}

# -----------------------------------------------------------------------------
#  Boot: run a command inside the namespaces, against the image
# -----------------------------------------------------------------------------
boot() {
	local dir="$1"
	shift
	local abs_dir
	abs_dir=$(cd "$dir" && pwd)

	# The command is passed through the environment: quoting it as a single
	# string is what makes "vm.sh --run img -- bash -c '...'" work unchanged.
	export B2BR_VM_DIR="$abs_dir"
	export B2BR_VM_CMD="$*"
	# Default to the project this tester belongs to, so that the scripts under
	# test are the ones from this checkout and never a stale copy.
	export B2BR_VM_PROJECT="${B2BR_VM_PROJECT:-$B2BR_DIR}"
	# The login is needed inside for the "who" simulator, whose output several
	# community testers cross-check against the hostname.
	if [ -f "$B2BR_DIR/config/vm.conf" ]; then
		# shellcheck disable=SC1091
		. "$B2BR_DIR/config/vm.conf"
	fi
	export B2BR_VM_LOGIN="${VM_LOGIN:-jojo8356}"
	# openssl.cnf lives outside /etc; pass it through so the bonus script can
	# generate the vsftpd certificate inside the namespaces.
	# The SSL configuration is copied outside /home (reachable from inside the
	# namespaces) so the bonus script can generate the vsftpd certificate.
	STAGE=$(mktemp -d /tmp/b2br-stage.XXXXXX)
	mkdir -p "$STAGE/ssl"
	[ -f "$BASE_IMAGE_DEFAULT/etc/ssl/openssl.cnf" ] && \
		cp -f "$BASE_IMAGE_DEFAULT/etc/ssl/openssl.cnf" "$STAGE/ssl/openssl.cnf"
	export B2BR_HOST_SSL_CNF="$STAGE/ssl/openssl.cnf"
	export B2BR_VM_STAGE="$STAGE"

	unshare --user --map-root-user --mount --uts --pid --fork --propagation private \
		bash "$B2BR_DIR/tests/.vm-inside.sh"
}

# -----------------------------------------------------------------------------
#  CLI
# -----------------------------------------------------------------------------
main() {
	local mode="" dir=""

	case "${1:-}" in
	--snapshot|--run)
		mode="$1"; dir="${2:-}"
		shift 2 || true
		[ "${1:-}" = "--" ] && shift
		;;
	--reset)
		rm -rf "${2:-}"
		echo "reset ${2:-}"
		exit 0
		;;
	*)
		sed -n '3,30p' "$0" | sed 's/^# \{0,1\}//'
		exit 1
		;;
	esac

	[ -n "$dir" ] || { echo "vm.sh: missing image directory" >&2; exit 2; }
	prepare_base "$BASE_IMAGE_DEFAULT"

	if [ "$mode" = "--snapshot" ] || [ ! -d "$dir" ]; then
		new_image "$dir"
	fi

	boot "$dir" "$@"
	rc=$?
	[ -n "${STAGE:-}" ] && rm -rf "$STAGE"
	return $rc
}

main "$@"
