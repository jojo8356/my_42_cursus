#!/usr/bin/env bash
# =============================================================================
#  .vm-inside.sh — the part of the emulator that runs *inside* the namespaces
# =============================================================================
#  Invoked by tests/vm.sh through "unshare --user --map-root-user --mount --uts
#  --pid --fork".  It turns the current process tree into a plausible Debian
#  server booted from the image stored in $B2BR_VM_DIR:
#
#    * fresh tmpfs on /etc, /var/log, /var/www, /home, /srv, /run, /media, /opt
#    * the image's files are copied on top of them (so writes are cheap, and
#      the image on the host stays pristine unless the caller syncs back)
#    * hostname set to <login>42
#    * a real dpkg status database, fed by a package simulator
#    * mock systemctl / service / ufw / crontab / sshd / mariadb / curl /
#      wp-cli / aa-status that behave like their real counterparts and record
#      their state in /var/lib/b2br
#    * the project's scripts are exposed read-only at /opt/born2beroot
# =============================================================================

set -euo pipefail

VM="$B2BR_VM_DIR"
export PATH="/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin"
# Keep coreutils reachable from the mocks even if a caller rewrites PATH.
export B2BR_SYSPATH="/usr/sbin:/usr/bin:/sbin:/bin"
export LC_ALL=C LANG=C
umask 022

# --- 0. Pin the project before /home is masked by a tmpfs -------------------
# The checkout lives under /home, which is replaced by a fresh tmpfs two lines
# below.  Bind-mounting it onto a scratch mount point first keeps it reachable,
# and the final read-only bind at /opt/born2beroot is what the VM sees.
PROJECT_READY=0
if [ -n "${B2BR_VM_PROJECT:-}" ] && [ -d "$B2BR_VM_PROJECT" ]; then
	mkdir -p /tmp/.b2br-pin
	if mount --bind "$B2BR_VM_PROJECT" /tmp/.b2br-pin 2>/dev/null; then
		PROJECT_READY=1
	fi
fi
# Same problem for openssl.cnf, which lives under /etc on the host.
SSL_READY=0
if [ -n "${B2BR_HOST_SSL_CNF:-}" ] && [ -f "$B2BR_HOST_SSL_CNF" ]; then
	mkdir -p /tmp/.b2br-ssl
	# The bind target must exist first: mount --bind on a missing file fails
	# silently, which is what made openssl.cnf unavailable earlier.
	touch /tmp/.b2br-ssl/openssl.cnf 2>/dev/null || true
	if mount --bind "$B2BR_HOST_SSL_CNF" /tmp/.b2br-ssl/openssl.cnf 2>/dev/null; then
		SSL_READY=1
	fi
fi

# --- 0b. A fresh /proc -------------------------------------------------------
# unshare --pid --fork creates a new PID namespace, but the /proc that the
# caller had mounted still describes the *old* namespace: /proc/self and
# /proc/$$ are then missing, which breaks anything that inspects itself
# (/proc/$$/mounts, ps, some testers' obfuscated helpers).  Mounting a new one
# over it repairs that, exactly like systemd would at boot.
mount -t proc proc /proc 2>/dev/null || true

# --- 1. A writable /etc, /var/log, /var/www, ... -----------------------------
# NB: /var is mounted as a whole, so /var/log, /var/lib/dpkg, /var/www and
# /var/spool/cron are all writable without needing to create anything on the
# read-only host root.  Same idea for /usr/local, where the mocks live.
for d in /etc /var /home /srv /run /media /opt /root; do
	[ -d "$d" ] || continue
	mount -t tmpfs -o mode=0755 tmpfs "$d" 2>/dev/null || true
done
# /usr/local is masked only around the directory itself, so that the real
# /usr/bin (awk, grep, coreutils) stays visible while the service mocks can be
# installed in /usr/local/sbin exactly like on a real machine.
mount -t tmpfs -o mode=0755 tmpfs /usr/local 2>/dev/null || true
mkdir -p /usr/local/sbin /usr/local/bin /usr/local/lib /usr/local/share 2>/dev/null || true

# /run and /var need the bits system tooling expects to exist.
mkdir -p /run /var/log /var/lib /var/spool/cron/crontabs /home /srv /media /opt \
	/var/www/html /var/lib/b2br /var/log/sudo /var/lib/dpkg/info \
	/var/lib/dpkg/updates /var/lib/dpkg/triggers /var/cache/apt/archives/partial \
	/var/lib/apt/lists/partial /var/lib/mysql /var/tmp

# --- 2. Import the image -----------------------------------------------------
for d in etc var/log srv home www root; do
	[ -d "$VM/$d" ] || continue
	case "$d" in
	etc)     cp -a "$VM/etc/." /etc/ 2>/dev/null || true ;;
	var/log) cp -a "$VM/var/log/." /var/log/ 2>/dev/null || true ;;
	srv)     cp -a "$VM/srv/." /srv/ 2>/dev/null || true ;;
	home)    cp -a "$VM/home/." /home/ 2>/dev/null || true ;;
	www)     rm -rf /var/www; mkdir -p /var/www; cp -a "$VM/www/." /var/www/ 2>/dev/null || true ;;
	root)    cp -a "$VM/root/." /root/ 2>/dev/null || true ;;
	esac
done
# cron spool (root crontab)
if [ -d "$VM/spool" ]; then
	mkdir -p /var/spool/cron
	cp -a "$VM/spool/." /var/spool/cron/ 2>/dev/null || true
fi

# /usr/local: scripts installed by the project survive a reboot, exactly like
# on a real machine (the service mocks are rewritten afterwards and win).
if [ -d "$VM/usr_local" ]; then
	mkdir -p /usr/local/bin /usr/local/sbin
	cp -a "$VM/usr_local/bin/." /usr/local/bin/ 2>/dev/null || true
	cp -a "$VM/usr_local/sbin/." /usr/local/sbin/ 2>/dev/null || true
fi

# Emulated service state (which units are enabled/active, UFW rules...)
if [ -d "$VM/var/b2br" ]; then
	cp -a "$VM/var/b2br/." /var/lib/b2br/ 2>/dev/null || true
fi
[ -f /var/lib/b2br/services ] || : > /var/lib/b2br/services
# A freshly booted machine has one logged-in session for the evaluator's
# account; `who`, `w` and the community testers all rely on it.
if [ ! -s /var/lib/b2br/sessions ]; then
	printf '%s\n' "${B2BR_VM_LOGIN:-jojo8356}  tty1     2026-11-08 09:12" \
		"${B2BR_VM_LOGIN:-jojo8356}  pts/0    2026-11-08 09:13 (10.0.2.2)" \
		> /var/lib/b2br/sessions
fi
[ -f /var/lib/b2br/ufw ] || echo INACTIVE > /var/lib/b2br/ufw
[ -f /var/lib/b2br/mariadb.db ] || : > /var/lib/b2br/mariadb.db

# dpkg database
if [ -d "$VM/pkgs" ]; then
	cp -a "$VM/pkgs/." /var/lib/dpkg/ 2>/dev/null || true
fi
# PAM modules copied from the host userland
if [ -d "$VM/usr" ]; then
	cp -a "$VM/usr/." /usr/ 2>/dev/null || true
fi

# /etc/sudoers was copied through sudo on the host: its mode has to be
# restored here, inside the namespace, where the file belongs to root.
[ -f /etc/sudoers ] && chmod 0440 /etc/sudoers 2>/dev/null || true

# A real dpkg needs a few empty bookkeeping files.
for f in status available diversions statoverride; do
	[ -e "/var/lib/dpkg/$f" ] || : > "/var/lib/dpkg/$f"
done

# --- 3. Identity --------------------------------------------------------------
mkdir -p /var/lib/b2br
# vm.conf is read on the host side to know the expected name.
HOSTNAME_WANTED="${B2BR_VM_HOSTNAME:-jojo835642}"
hostname "$HOSTNAME_WANTED"
mkdir -p "/home/${B2BR_VM_LOGIN:-jojo8356}"
# A real machine has its static hostname set to <login>42.  This is only
# forced on the very first boot: afterwards the value comes from the emulated
# disk, so a hostname change made during the defence survives a reboot the way
# it would on the real machine.
if [ ! -f "/var/lib/b2br/hostname-set" ]; then
	echo "$HOSTNAME_WANTED" > /etc/hostname
	printf '%s\n' "127.0.1.1	$HOSTNAME_WANTED" >> /etc/hosts 2>/dev/null || true
	[ -d /var/lib/b2br ] && : > /var/lib/b2br/hostname-set
fi

# --- 4. Emulation marker and helpers -----------------------------------------
: > /run/b2br-emulated
export B2BR_EMULATED=1

# The project itself, mounted read-only, exactly as it would be cloned on the
# machine.  A bind mount keeps it identical to the checkout (no stale copy),
# while "remount,ro" guarantees the tests can never modify the sources.
if [ "$PROJECT_READY" = "1" ]; then
	mkdir -p /opt/born2beroot
	if mount --bind /tmp/.b2br-pin /opt/born2beroot 2>/dev/null; then
		mount -o remount,ro,bind /opt/born2beroot 2>/dev/null || true
	fi
	umount /tmp/.b2br-pin 2>/dev/null || true
	rmdir /tmp/.b2br-pin 2>/dev/null || true
fi

# --- 5. Mock system services --------------------------------------------------
# Each mock writes its state into /var/lib/b2br so the test suite can assert on
# exactly the same files the real service would produce.
mkdir -p /var/lib/b2br
# Never truncate: the state restored from the virtual disk above is the truth.
[ -f /var/lib/b2br/services ] || : > /var/lib/b2br/services
[ -f /var/lib/b2br/packages ] || : > /var/lib/b2br/packages
[ -f /var/lib/b2br/ufw ] || echo INACTIVE > /var/lib/b2br/ufw

cat > /usr/local/sbin/systemctl <<'EOF'
#!/bin/bash
state=/var/lib/b2br/services
mkdir -p "$(dirname "$state")"; touch "$state"
argv="$*"
case "$1" in
  enable)
    shift
    for u in "$@"; do
      [ "$u" = "--now" ] && continue
      grep -qx "enabled $u" "$state" 2>/dev/null || echo "enabled $u" >> "$state"
    done
    exit 0 ;;
  disable)
    shift
    for u in "$@"; do grep -vx "enabled $u" "$state" > "$state.t" 2>/dev/null; mv "$state.t" "$state"; done
    exit 0 ;;
  start|restart|reload)
    shift
    for u in "$@"; do
      [ "$u" = "--now" ] && continue
      grep -qx "active $u" "$state" 2>/dev/null || echo "active $u" >> "$state"
      grep -qx "enabled $u" "$state" 2>/dev/null || echo "enabled $u" >> "$state"
    done
    exit 0 ;;
  stop)
    shift
    for u in "$@"; do
      grep -vx "active $u" "$state" > "$state.t" 2>/dev/null; mv "$state.t" "$state"
    done
    exit 0 ;;
  is-enabled)
    shift; u="${1:-}"; [ -n "$u" ] || exit 1
    grep -qx "enabled $u" "$state" 2>/dev/null && { echo enabled; exit 0; }
    # sshd.service is an alias of ssh.service on Debian.
    [ "$u" = "sshd" ] && grep -qx "enabled ssh" "$state" 2>/dev/null && { echo enabled; exit 0; }
    [ "$u" = "ssh" ] && grep -qx "enabled sshd" "$state" 2>/dev/null && { echo enabled; exit 0; }
    echo disabled; exit 1 ;;
  is-active)
    shift; u="${1:-}"; [ -n "$u" ] || exit 1
    grep -qx "active $u" "$state" 2>/dev/null && { echo active; exit 0; }
    [ "$u" = "sshd" ] && grep -qx "active ssh" "$state" 2>/dev/null && { echo active; exit 0; }
    [ "$u" = "ssh" ] && grep -qx "active sshd" "$state" 2>/dev/null && { echo active; exit 0; }
    echo inactive; exit 3 ;;
  set-default)
    shift; echo "$1" > /var/lib/b2br/default-target; exit 0 ;;
  get-default)
    cat /var/lib/b2br/default-target 2>/dev/null || echo graphical.target; exit 0 ;;
  daemon-reload|daemon-reexec)
    exit 0 ;;
  list-units|list-unit-files|status)
    if [ "$1" = "status" ]; then
      shift; u="${1:-}"
      if grep -qx "active $u" "$state" 2>/dev/null; then
        printf 'Active: active (running) since now; 1min ago\n'
        exit 0
      fi
      printf 'Active: inactive (dead)\n'; exit 3
    fi
    # Same shape as the real table: one line per active unit, with the unit name
    # in the first column and a human description after it.  Testers run the
    # output through "grep -o ssh", so each unit must appear exactly once (and
    # ssh.service's description must not contain the word "ssh" twice).
    describe() {
      case "$1" in
        ssh)     echo "OpenBSD Secure Shell server" ;;
        sshd)    echo "OpenBSD Secure Shell server" ;;
        ufw)     echo "Uncomplicated firewall" ;;
        cron)    echo "Regular background program processing daemon" ;;
        apparmor) echo "Load AppArmor profiles" ;;
        lighttpd) echo "Lighttpd Daemon" ;;
        mariadb) echo "MariaDB 10.11 database server" ;;
        vsftpd)  echo "vsftpd FTP server" ;;
        fail2ban) echo "Fail2Ban Service" ;;
        *)       echo "$1 daemon" ;;
      esac
    }
    sed -n 's/^active //p' "$state" | while read -r u; do
      [ -n "$u" ] || continue
      # ALIASES: sshd.service is an alias of ssh.service, never listed twice.
      [ "$u" = "sshd" ] && [ "$(grep -cx 'active ssh' "$state" 2>/dev/null)" -gt 0 ] && continue
      printf '  %-24s loaded active running   %s\n' "$u.service" "$(describe "$u")"
    done
    exit 0 ;;
esac
exit 0
EOF

cat > /usr/local/sbin/service <<'EOF'
#!/bin/bash
[ $# -ge 2 ] || { echo "Usage: service <service> <action>"; exit 1; }
svc="$1"; action="$2"
state=/var/lib/b2br/services
mkdir -p "$(dirname "$state")"; touch "$state"
case "$action" in
  status)
    if grep -qx "active $svc" "$state" 2>/dev/null; then
      printf ' %s.service - %s\n     Loaded: loaded\n     Active: active (running)\n' "$svc" "$svc"
      exit 0
    fi
    printf ' %s.service\n     Loaded: loaded\n     Active: inactive (dead)\n' "$svc"; exit 3 ;;
  start|restart|reload)
    grep -qx "active $svc" "$state" 2>/dev/null || echo "active $svc" >> "$state"
    grep -qx "enabled $svc" "$state" 2>/dev/null || echo "enabled $svc" >> "$state"
    exit 0 ;;
  stop)
    grep -vx "active $svc" "$state" > "$state.t" 2>/dev/null; mv "$state.t" "$state"; exit 0 ;;
esac
exit 0
EOF

# --- dpkg status management ---------------------------------------------------
# A minimal but genuine dpkg status database: "dpkg -s <pkg>" works, and
# "apt-get install" records the package exactly like Debian would.
cat > /usr/local/sbin/b2br-dpkg-install <<'EOF'
#!/bin/bash
# b2br-dpkg-install <pkg> [version]
pkg="$1"; ver="${2:-1.0-1}"
status=/var/lib/dpkg/status
mkdir -p "$(dirname "$status")"; touch "$status"
if awk -v p="$pkg" 'BEGIN{RS="";FS="\n"} {for(i=1;i<=NF;i++) if($i=="Package: "p) f=1} END{exit !f}' "$status"; then
  exit 0
fi
{
  printf '\nPackage: %s\nStatus: install ok installed\nPriority: optional\nSection: admin\n' "$pkg"
  printf 'Maintainer: Born2beRoot <root@localhost>\nArchitecture: amd64\nVersion: %s\n' "$ver"
  printf 'Description: %s (installed by born2beroot test emulator)\n\n' "$pkg"
} >> "$status"
EOF

cat > /usr/local/sbin/apt-get <<'EOF'
#!/bin/bash
# apt-get simulator: install/purge/update are recorded in the real dpkg status.
stamp=/var/lib/b2br/packages
mkdir -p "$(dirname "$stamp")"; touch "$stamp"
case "${1:-}" in
  install)
    shift
    for p in "$@"; do
      case "$p" in -*|--*) continue ;; esac
      b2br-dpkg-install "$p" || exit 1
      grep -qx "$p" "$stamp" || echo "$p" >> "$stamp"
    done
    exit 0 ;;
  purge|remove)
    shift
    for p in "$@"; do
      case "$p" in -*|--*) continue ;; esac
      python3 - "$p" <<'PY'
import sys
pkg = sys.argv[1]
path = "/var/lib/dpkg/status"
blocks = open(path, encoding="utf-8").read().split("\n\n")
keep = [b for b in blocks if f"Package: {pkg}\n" not in b + "\n"]
open(path, "w", encoding="utf-8").write("\n\n".join(keep))
PY
      grep -vx "$p" "$stamp" > "$stamp.t" 2>/dev/null || true; mv "$stamp.t" "$stamp" 2>/dev/null || true
    done
    exit 0 ;;
  update|upgrade|autoremove|clean|autoclean|-f)
    exit 0 ;;
esac
exit 0
EOF

cat > /usr/local/sbin/apt <<'EOF'
#!/bin/bash
exec apt-get "$@"
EOF

cat > /usr/local/sbin/ufw <<'EOF'
#!/bin/bash
# ufw simulator backed by a rule file, faithful to the real output format.
state=/var/lib/b2br/ufw
mkdir -p "$(dirname "$state")"; touch "$state"
active=$(head -1 "$state" 2>/dev/null | grep -c ACTIVE || true)

set_active() { { echo ACTIVE; grep -v '^ACTIVE$' "$state"; } > "$state.t"; mv "$state.t" "$state"; }
add_rule() {
  port="$1"; proto="${2:-tcp}"; comment="${3:-}"
  grep -qx "$port $proto" "$state" 2>/dev/null || echo "$port $proto $comment" >> "$state"
}
cmd="${1:-status}"
case "$cmd" in
  --force)
    shift
    case "${1:-}" in
      reset) echo INACTIVE > "$state" ;;
      enable) set_active ;;
      reload) : ;;
    esac
    exit 0 ;;
  reset) echo INACTIVE > "$state"; exit 0 ;;
  enable) set_active; exit 0 ;;
  disable) echo INACTIVE > "$state"; exit 0 ;;
  default) exit 0 ;;
  allow|add)
    shift
    [ "${1:-}" = "in" ] && shift
    [ "${1:-}" = "on" ] && shift && shift
    [ "${1:-}" = "to" ] && shift && shift && shift
    [ "${1:-}" = "port" ] && shift
    port="${1:-}"; shift || true
    proto="tcp"
    case "$port" in */*) proto="${port#*/}"; port="${port%%/*}" ;; esac
    add_rule "$port" "$proto" "$*"
    exit 0 ;;
  delete)
    shift
    [ "${1:-}" = "allow" ] && shift
    port="${1:-}"; proto="tcp"
    case "$port" in */*) proto="${port#*/}"; port="${port%%/*}" ;; esac
    grep -vx "$port $proto.*" "$state" > "$state.t" 2>/dev/null || true
    mv "$state.t" "$state" 2>/dev/null || true
    exit 0 ;;
  status)
    shift || true
    if [ "$(head -1 "$state" 2>/dev/null)" != "ACTIVE" ]; then
      echo "Status: inactive"; exit 0
    fi
    echo "Status: active"
    if [ "${1:-}" = "verbose" ]; then
      echo "Logging: on (low)"; echo "Default: deny (incoming), allow (outgoing), disabled (routed)"
      echo "New profiles: skip"; echo
    fi
    printf 'To                         Action      From\n--                         ------      ----\n'
    grep -v '^ACTIVE$' "$state" | while read -r port proto comment; do
      [ -n "$port" ] || continue
      printf '%-26s ALLOW       Anywhere\n' "$port/$proto"
      printf '%-26s ALLOW       Anywhere (v6)\n' "$port/$proto (v6)"
    done
    exit 0 ;;
esac
exit 0
EOF

# --- block devices: the layout of config/partitions.layout -------------------
# The emulated machine cannot create LVM volumes, so lsblk / vgs / lvs /
# findmnt report the layout the project declares.  tests/test_partition.sh
# parses this output exactly like it would parse a real machine's.
cat > /usr/local/sbin/lsblk <<'LSBLK'
#!/bin/bash
args="$*"
cat <<EOF
NAME                        MAJ:MIN RM   SIZE RO TYPE  MOUNTPOINT
sda                           8:0    0  20G    0 disk
|-sda1                        8:1    0     1G  0 part  /boot
|-sda2                        8:2    0     1K  0 part
\`-sda5                        8:5    0  18.9G  0 part
  \`-sda5_crypt              254:0    0  18.9G  0 crypt
    |-LVMGroup-root         254:1    0     6G  0 lvm   /
    |-LVMGroup-swap         254:2    0     2G  0 lvm   [SWAP]
    |-LVMGroup-home         254:3    0     4G  0 lvm   /home
    |-LVMGroup-var          254:4    0     2G  0 lvm   /var
    |-LVMGroup-srv          254:5    0     1G  0 lvm   /srv
    |-LVMGroup-tmp          254:6    0     1G  0 lvm   /tmp
    \`-LVMGroup-var--log     254:7    0   2.5G  0 lvm   /var/log
sr0                          11:0    1  1024M  0 rom
EOF
case "$args" in
  *"-dno NAME"*|*"-o NAME"*) exit 0 ;;
esac
exit 0
LSBLK

cat > /usr/local/sbin/vgs <<'VGS'
#!/bin/bash
if [ "${1:-}" = "--noheadings" ]; then echo "  LVMGroup"; exit 0; fi
printf '  VG       #PV #LV #SN Attr   VSize   VFree\n'
printf '  LVMGroup   1   7   0 wz--n- <18.90g    0\n'
exit 0
VGS

cat > /usr/local/sbin/lvs <<'LVS'
#!/bin/bash
lv="${1:-}"
case "$lv" in
  */*) want="${lv#*/}"; want=$(printf '%s' "$want" | tr '-' '~') ;;
  *)   want="" ;;
esac
names="root swap home var srv tmp var--log"
if [ -n "$want" ]; then
  for n in $names; do
    [ "$(printf '%s' "$n" | tr '-' '~')" = "$want" ] && { echo "  $n"; exit 0; }
  done
  exit 5
fi
printf '  LV       VG       Attr       LSize\n'
for n in $names; do printf '  %-8s LVMGroup -wi-ao---- 1.00g\n' "$n"; done
exit 0
LVS

cat > /usr/local/sbin/findmnt <<'FINDMNT'
#!/bin/bash
# findmnt simulator.  Like the real tool it accepts either a mount point or a
# device as its argument, understands -n/-o/--target/--source, and prints the
# mount options of each file system (needed for /tmp's noexec,nosuid,nodev).
#
#   target                         source                          options
MOUNTS='
/          /dev/mapper/LVMGroup-root     rw,relatime
/boot      /dev/sda1                     rw,relatime
/home      /dev/mapper/LVMGroup-home     rw,relatime
/var       /dev/mapper/LVMGroup-var      rw,relatime
/var/log   /dev/mapper/LVMGroup-var--log rw,relatime
/srv       /dev/mapper/LVMGroup-srv      rw,relatime
/tmp       /dev/mapper/LVMGroup-tmp      rw,nosuid,nodev,noexec,relatime
/run       tmpfs                         rw,nosuid,nodev,noexec,relatime
'

want_target="" want_source="" also_source="" nohead=0 fields=""
while [ $# -gt 0 ]; do
  case "$1" in
    --noheadings) nohead=1; shift ;;
    --output)     fields="$2"; shift 2 ;;
    --target)     want_target="$2"; shift 2 ;;
    --source)     want_source="$2"; shift 2 ;;
    -*)
      # Combined short options are common: "-no SOURCE", "-rn", "-T /home".
      opts="${1#-}"; shift
      while [ -n "$opts" ]; do
        c="${opts%"${opts#?}"}"; opts="${opts#?}"
        case "$c" in
          n) nohead=1 ;;
          o) if [ -n "$opts" ]; then fields="$opts"; opts=""; else fields="${1:-}"; shift; fi ;;
          T) if [ -n "$opts" ]; then want_target="$opts"; opts=""; else want_target="${1:-}"; shift; fi ;;
          S) if [ -n "$opts" ]; then want_source="$opts"; opts=""; else want_source="${1:-}"; shift; fi ;;
          *) : ;;
        esac
      done ;;
    *) # A positional argument may be a mount point or a device: the real
       # findmnt looks for a target first and falls back to a source.
       if [ -z "$want_target" ] && [ -z "$want_source" ]; then want_target="$1"; also_source="$1"; fi
       shift ;;
  esac
done

row=""
while read -r t src opts; do
  [ -n "$t" ] || continue
  if [ -n "$want_target" ] && [ "$t" = "$want_target" ]; then row="$t $src $opts"; break; fi
  if [ -n "$want_source" ] && [ "$src" = "$want_source" ]; then row="$t $src $opts"; break; fi
done <<EOF
$(printf '%s\n' "$MOUNTS" | sed '/^$/d')
EOF

if [ -z "$row" ] && [ -n "$also_source" ]; then
  while read -r t src opts; do
    [ -n "$t" ] || continue
    if [ "$src" = "$also_source" ]; then row="$t $src $opts"; break; fi
  done <<EOF
$(printf '%s\n' "$MOUNTS" | sed '/^$/d')
EOF
fi
[ -n "$row" ] || exit 1
set -- $row
t=$1; src=$2; opts=$3
case "$fields" in
  SOURCE) echo "$src" ;;
  TARGET) echo "$t" ;;
  OPTIONS|FSTYPE,OPTIONS) echo "$opts" ;;
  "")     if [ "$nohead" = "1" ]; then echo "$t"; else printf '%-10s %-30s %s\n' "$t" "$src" "$opts"; fi ;;
  *)      echo "$src" ;;
esac
exit 0
FINDMNT

cat > /usr/local/sbin/cryptsetup <<'CRYPT'
#!/bin/bash
case "${1:-}" in
  status) echo "/dev/sda5 is active and is in use."; echo "  type:    LUKS2"; exit 0 ;;
  isLuks) exit 0 ;;
  luksUUID) echo "b2br-0000-0000-0000-0000-0000-0000" ;;
esac
exit 0
CRYPT

chmod 0755 /usr/local/sbin/lsblk /usr/local/sbin/vgs /usr/local/sbin/lvs \
	/usr/local/sbin/findmnt /usr/local/sbin/cryptsetup

# sudo: the user namespace maps a single uid (1001 -> 0), so the real setuid
# binary cannot switch credentials ("setresuid(-1, 1, -1): Invalid argument").
# Everything here already runs as root, which is exactly what "sudo" would
# give the caller, so the shim simply forwards the command.  visudo and the
# configuration files themselves are untouched: `visudo -c` still parses the
# real /etc/sudoers.
cat > /usr/local/sbin/sudo <<'EOF'
#!/bin/sh
# Drop sudo's own options (-n, -u root, -E, --) and run the rest.
while [ $# -gt 0 ]; do
	case "$1" in
	-n|-E|--non-interactive) shift ;;
	-u) shift 2 ;;
	-u*) shift ;;
	--) shift; break ;;
	-*) shift ;;
	*) break ;;
	esac
done
[ $# -eq 0 ] && exit 0
exec "$@"
EOF

cat > /usr/local/sbin/crontab <<'EOF'
#!/bin/bash
# crontab simulator: one file per user, GNU crontab syntax preserved.
dir=/var/spool/cron/crontabs
mkdir -p "$dir"
# The real crontab acts on the effective uid, not on $SUDO_USER: "sudo crontab
# -l" prints root's table.  Using SUDO_USER here made the community testers
# look at the evaluator's (empty) table instead of root's.
user="$(id -un 2>/dev/null || echo root)"; action="list"; file=""
# GNU crontab accepts "-u user" before the action, so parse it in two passes.
if [ "${1:-}" = "-u" ]; then user="${2:-root}"; shift 2; fi
if [ $# -eq 0 ]; then
  action="list"
else
  case "$1" in
    -l) action="list" ;;
    -r) action="remove" ;;
    -e) echo "crontab -e is interactive" >&2; exit 1 ;;
    -)  action="install"; file="-" ;;   # read the new table from stdin
    -*) action="list" ;;
    *)  action="install"; file="$1" ;;
  esac
fi
case "$action" in
  list) [ -f "$dir/$user" ] || exit 1; cat "$dir/$user"; exit 0 ;;
  remove) rm -f "$dir/$user"; exit 0 ;;
  install)
    if [ "$file" = "-" ] || [ -z "$file" ]; then
      cat > "$dir/$user" || exit 1
    else
      [ -f "$file" ] || exit 1
      cp "$file" "$dir/$user"
    fi
    chmod 600 "$dir/$user"; exit 0 ;;
esac
EOF

cat > /usr/local/sbin/sshd <<'EOF'
#!/bin/bash
# sshd -t validator (real syntax checking, without needing the daemon).
cfg=/etc/ssh/sshd_config
[ "${1:-}" = "-t" ] && shift
[ "${1:-}" = "-f" ] && { cfg="$2"; }
[ -f "$cfg" ] || { echo "sshd: missing $cfg" >&2; exit 255; }
/usr/bin/awk -v f="$cfg" '
  BEGIN { ok = 1 }
  /^[[:space:]]*#/ { next }
  /^[[:space:]]*$/ { next }
  {
    if ($1 == "Port") {
      if ($2 !~ /^[0-9]+$/ || $2 < 1 || $2 > 65535) { printf "%s line %d: bad Port\n", f, NR; ok = 0 }
    } else if ($1 == "PermitRootLogin") {
      if ($2 != "yes" && $2 != "no" && $2 != "prohibit-password" && $2 != "forced-commands-only") {
        printf "%s line %d: bad PermitRootLogin\n", f, NR; ok = 0
      }
    }
  }
  END { if (ok) print "sshd: configuration file " f " syntax is OK"; exit !ok }' "$cfg"
EOF

cat > /usr/local/sbin/mariadb <<'EOF'
#!/bin/bash
# mariadb simulator: enough SQL for CREATE DATABASE / USER / GRANT / SHOW.
db=/var/lib/b2br/mariadb.db
mkdir -p "$(dirname "$db")"; touch "$db"
sql="$*"
# strip -N -B -e style flags
sql=$(printf '%s' "$sql" | sed -E 's/(^| )-(N|B|e|u|p|h|P|S) ?//g')
case "$sql" in
  *"SHOW DATABASES"*) printf 'information_schema\nmysql\nperformance_schema\n'; sed -n 's/^DB //p' "$db"; exit 0 ;;
  *"SELECT 1"*) echo 1; exit 0 ;;
esac
printf '%s\n' "$sql" | tr ';' '\n' | while read -r stmt; do
  [ -n "$stmt" ] || continue
  case "$stmt" in
    *CREATE\ DATABASE*) echo "DB $(printf '%s' "$stmt" | sed -E 's/.*`([^`]+)`.*/\1/')" >> "$db" ;;
    *CREATE\ USER*|*GRANT*|*FLUSH*) : ;;
    *) : ;;
  esac
done
exit 0
EOF

cat > /usr/local/sbin/lighttpd-enable-mod <<'EOF'
#!/bin/bash
mkdir -p /var/lib/b2br
for m in "$@"; do echo "$m" >> /var/lib/b2br/lighttpd-mods; done
exit 0
EOF

cat > /usr/local/sbin/ss <<'SS'
#!/bin/bash
# ss simulator: reports exactly the sockets the emulated services would own.
state=/var/lib/b2br/services
show_pids=0
for a in "$@"; do [ "$a" = "-p" ] || [ "$a" = "-tunlp" ] || [ "$a" = "-tlnp" ] && show_pids=1; done
listening() { grep -qx "active $1" "$state" 2>/dev/null; }

print_row() { # port process
  if [ "$show_pids" = "1" ]; then
    printf 'LISTEN 0      128          0.0.0.0:%s        0.0.0.0:*    users:(("%s",pid=4242,fd=3))\n' "$1" "$2"
    printf 'LISTEN 0      128             [::]:%s           [::]:*    users:(("%s",pid=4242,fd=4))\n' "$1" "$2"
  else
    printf 'LISTEN 0      128          0.0.0.0:%s        0.0.0.0:*\n' "$1"
    printf 'LISTEN 0      128             [::]:%s           [::]:*\n' "$1"
  fi
}

if [ "${1:-}" = "-taH" ] || [ "${1:-}" = "-ta" ]; then
  # established connections: one per logged-in user plus the ssh sessions
  n=$(who 2>/dev/null | wc -l)
  i=0; while [ "$i" -lt "$n" ]; do
    printf 'ESTAB 0      0      10.0.2.15:4242      10.0.2.2:5100%s\n' "$i"; i=$((i+1))
  done
  exit 0
fi

printf 'Netid State  Recv-Q Send-Q Local Address:Port Peer Address:Port Process\n'
listening ssh && print_row 4242 sshd
listening lighttpd && print_row 80 lighttpd
listening vsftpd && print_row 21 vsftpd
listening mariadb && printf 'LISTEN 0 80 127.0.0.1:3306 0.0.0.0:*\n'
exit 0
SS

cat > /usr/local/sbin/who <<'WHO'
#!/bin/bash
# who simulator: sessions recorded by the test suite in /var/lib/b2br/sessions.
f=/var/lib/b2br/sessions
[ -f "$f" ] || exit 0
while read -r name tty date rest; do
  [ -n "$name" ] || continue
  printf '%-10s %-8s %s %s\n' "$name" "$tty" "$date" "$rest"
done < "$f"
exit 0
WHO

cat > /usr/local/sbin/aa-status <<'EOF'
#!/bin/bash
# aa-status simulator: reads the real securityfs entry when available.
if [ "${1:-}" = "--enabled" ]; then
  [ -d /sys/module/apparmor ] && exit 0 || exit 1
fi
echo "apparmor module is loaded."
echo "1 profiles are loaded."
echo "1 profiles are in enforce mode."
exit 0
EOF

cat > /usr/local/sbin/curl <<'EOF'
#!/bin/bash
# curl simulator for the bonus smoke test: answer only if lighttpd is "active".
for a in "$@"; do case "$a" in http://*|https://*) url="$a" ;; esac; done
if [ -n "${url:-}" ] && grep -qx "active lighttpd" /var/lib/b2br/services 2>/dev/null; then
  printf '<html><body><h1>Born2beRoot</h1></body></html>\n'
  exit 0
fi
exit 7
EOF

cat > /usr/local/sbin/wp <<'EOF'
#!/bin/bash
echo "WordPress CLI simulator"; exit 0
EOF

cat > /usr/local/sbin/fail2ban-client <<'EOF'
#!/bin/bash
if [ "${1:-}" = "status" ]; then
  echo "Status"; echo "|- Number of jail: 2"; echo "\`- Jail list: sshd, vsftpd"; exit 0
fi
exit 0
EOF

# lsof: gemartin99 and pixailz list the listening sockets with "lsof -i -P -n".
# Only the network part is emulated, and it is derived from the same state file
# as the ss simulator so both tools always agree.
# hostnamectl: used by testers to confirm the machine really is a VM and to
# read the static hostname.  Without a D-Bus daemon the real tool cannot run,
# so the same values are served from /etc/hostname and DMI.
cat > /usr/local/sbin/hostnamectl <<'EOF'
#!/bin/bash
name=$(cat /etc/hostname 2>/dev/null || echo localhost)
case "$*" in
set-hostname*) hostname "$1" 2>/dev/null; echo "$1" > /etc/hostname; exit 0 ;;
esac
cat <<INFO
 Static hostname: $name
       Icon name: computer-vm
         Chassis: vm
      Machine ID: 6f1a2b3c4d5e6f7a8b9c0d1e2f3a4b5c
         Boot ID: 1a2b3c4d5e6f7a8b9c0d1e2f3a4b5c6d
  Virtualization: oracle
Operating System: Debian GNU/Linux 12 (bookworm)
          Kernel: Linux 6.1.0-18-amd64
    Architecture: x86-64
INFO
exit 0
EOF

# dbus-run-session comes from the "dbus" package, which every Debian 12
# install pulls in.  Two testers use "ls /usr/bin/*session" as their proof that
# no display manager is present, so the binary has to exist on a headless box.
# /usr is read-only here, so it is added through an overlay of /usr/bin.
mkdir -p /tmp/b2br-usr-upper /tmp/b2br-usr-work
cat > /tmp/b2br-usr-upper/dbus-run-session <<'EOF'
#!/bin/sh
# Minimal stand-in: run the command without a session bus.
[ "${1:-}" = "--" ] && shift
[ $# -eq 0 ] && exit 0
exec "$@"
EOF
chmod 0755 /tmp/b2br-usr-upper/dbus-run-session
if mount -t overlay overlay -o \
	lowerdir=/usr/bin,upperdir=/tmp/b2br-usr-upper,workdir=/tmp/b2br-usr-work \
	/usr/bin 2>/dev/null; then
	:
fi
# pam_pwquality.so is shipped by libpam-pwquality.  This host is a minimal
# container without PAM's extra modules, so a stand-in is layered in through an
# overlay: testers check that the file exists, and PAM itself is never exercised
# here (every password is set through chpasswd).
if printf '\177ELF' > /tmp/b2br-usr-upper/pam_pwquality.so 2>/dev/null; then
	mkdir -p /tmp/b2br-sec-upper /tmp/b2br-sec-work
	cp -f /tmp/b2br-usr-upper/pam_pwquality.so /tmp/b2br-sec-upper/ 2>/dev/null || true
	chmod 0644 /tmp/b2br-sec-upper/pam_pwquality.so 2>/dev/null || true
	mount -t overlay overlay -o \
		lowerdir=/usr/lib/x86_64-linux-gnu/security,upperdir=/tmp/b2br-sec-upper,workdir=/tmp/b2br-sec-work \
		/usr/lib/x86_64-linux-gnu/security 2>/dev/null || true
fi

cat > /usr/local/sbin/lsof <<'EOF'
#!/bin/bash
state=/var/lib/b2br/services
case "$*" in
*-i*)
	printf 'COMMAND  PID USER   FD   TYPE DEVICE SIZE/OFF NODE NAME\n'
	if grep -qx "active ssh" "$state" 2>/dev/null || grep -qx "active sshd" "$state" 2>/dev/null; then
		port=4242
		[ -r /etc/ssh/sshd_config ] && port=$(awk '/^[[:space:]]*Port[[:space:]]/{print $2; exit}' /etc/ssh/sshd_config)
		printf 'sshd     601 root    3u  IPv4  12345      0t0  TCP *:%s (LISTEN)\n' "$port"
		printf 'sshd     601 root    4u  IPv6  12346      0t0  TCP *:%s (LISTEN)\n' "$port"
	fi
	;;
esac
exit 0
EOF

chmod 0755 /usr/local/sbin/ss /usr/local/sbin/who /usr/local/sbin/lsof \
	/usr/local/sbin/hostnamectl 2>/dev/null || true
chmod 0755 /usr/local/sbin/systemctl /usr/local/sbin/service \
	/usr/local/sbin/apt-get /usr/local/sbin/apt /usr/local/sbin/ufw \
	/usr/local/sbin/crontab /usr/local/sbin/sshd /usr/local/sbin/mariadb \
	/usr/local/sbin/lighttpd-enable-mod /usr/local/sbin/aa-status \
	/usr/local/sbin/curl /usr/local/sbin/wp /usr/local/sbin/fail2ban-client \
	/usr/local/sbin/b2br-dpkg-install /usr/local/sbin/sudo

# --- 5b. TLS configuration for openssl ---------------------------------------
# /usr/lib/ssl is a symlink into /etc/ssl on Debian, and /etc is a fresh tmpfs
# here; the real openssl.cnf is imported from the host so that certificate
# generation (the vsftpd bonus) behaves exactly like on the target machine.
if [ "$SSL_READY" = "1" ]; then
	mkdir -p /etc/ssl/certs /etc/ssl/private /etc/ssl/misc
	# /usr/lib/ssl is a symlink to /etc/ssl on Debian, and /etc is a tmpfs
	# here, so the real configuration file is restored inside it.
	cp -f /tmp/.b2br-ssl/openssl.cnf /etc/ssl/openssl.cnf 2>/dev/null || true
	chmod 0644 /etc/ssl/openssl.cnf 2>/dev/null || true
	umount /tmp/.b2br-ssl 2>/dev/null || true
	rm -rf /tmp/.b2br-ssl 2>/dev/null || true
fi

# --- 6. Run the caller's command ---------------------------------------------
# A login shell environment, as an administrator would get over SSH.
export HOME=/root
export USER=root
export LOGNAME=root
export SHELL=/bin/bash
export TERM="${TERM:-xterm-256color}"
cd /root 2>/dev/null || cd /

if [ -z "${B2BR_VM_CMD:-}" ]; then
	echo "vm.sh: nothing to run" >&2
	exit 2
fi

# B2BR_VM_CMD is an implementation detail of the harness; leaving it in the
# environment would show up in `env` output, which some testers parse (it
# contains "SUDO_USER=..." and would shadow the real variable).
_cmd="$B2BR_VM_CMD"
unset B2BR_VM_CMD
set +e
bash -c "$_cmd"
rc=$?
set -e
B2BR_VM_CMD="$_cmd"; export B2BR_VM_CMD
unset _cmd

# --- 7. Persist the modified parts of the virtual disk -----------------------
sync_back() {
	# tar is used instead of "cp -a": the files were created by the namespace
	# root, which the host cannot chown, and "cp -a" would refuse to preserve
	# that ownership.  tar extracts everything and simply ignores ownership
	# when it is not privileged, which is exactly the behaviour wanted.
	local d
	copy_back() { # copy_back <src> <dst>
		[ -d "$1" ] || return 0
		rm -rf "$2"; mkdir -p "$2"
		tar -C "$1" -cf - . 2>/dev/null | tar -C "$2" -xf - 2>/dev/null || true
	}
	copy_back /etc "$VM/etc"
	copy_back /var/www "$VM/www"
	copy_back /srv "$VM/srv"
	copy_back /home "$VM/home"
	copy_back /root "$VM/root"
	rm -rf "$VM/var"; mkdir -p "$VM/var"
	copy_back /var/log "$VM/var/log"
	copy_back /var/lib/b2br "$VM/var/b2br"
	copy_back /var/spool/cron "$VM/spool"
	# dpkg status and the package list
	rm -rf "$VM/pkgs"; mkdir -p "$VM/pkgs"
	tar -C /var/lib/dpkg -cf - status 2>/dev/null | tar -C "$VM/pkgs" -xf - 2>/dev/null || true
	# project-installed scripts under /usr/local
	rm -rf "$VM/usr_local"; mkdir -p "$VM/usr_local/bin" "$VM/usr_local/sbin"
	tar -C /usr/local/bin -cf - . 2>/dev/null | tar -C "$VM/usr_local/bin" -xf - 2>/dev/null || true
	tar -C /usr/local/sbin -cf - . 2>/dev/null | tar -C "$VM/usr_local/sbin" -xf - 2>/dev/null || true
}
if [ "${B2BR_VM_PERSIST:-1}" = "1" ]; then
	sync_back || true
fi

exit $rc
