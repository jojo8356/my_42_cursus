#!/bin/bash
# =============================================================================
#  signature.sh — produce signature.txt, the SHA-256 of the virtual disk
# =============================================================================
#  The subject (chapter V) asks for a signature.txt file at the root of the
#  repository, containing the SHA-256 of the machine's disk.  The evaluator
#  runs "sha256sum disk.vdi" and "diff" against your file: if the machine was
#  modified after the signature was taken, the two differ and the evaluation
#  stops.  The signature must therefore be **regenerated** after every change
#  made to the machine, and the disk must not be running when it is taken.
#
#  Usage:
#      scripts/signature.sh <disk> [--output signature.txt]
#      scripts/signature.sh --check <disk>     # compare with the saved file
#
#  <disk> is the .vdi (VirtualBox) or .qcow2 (UTM) file of the machine.
# =============================================================================

set -euo pipefail

B2BR_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)
OUT="$B2BR_DIR/signature.txt"
MODE="write"
DISK=""

while [ $# -gt 0 ]; do
	case "$1" in
	--output) OUT="$2"; shift ;;
	--check)  MODE="check" ;;
	-h|--help) awk 'NR > 4 && /^# =+$/ { exit } NR > 4 && /^#/ { sub(/^# ?/, ""); print }' "$0"; exit 0 ;;
	-*) printf 'unknown option: %s\n' "$1" >&2; exit 2 ;;
	*)  DISK="$1" ;;
	esac
	shift
done

die() { printf '%s\n' "$*" >&2; exit 1; }

[ -n "$DISK" ] || die "usage: $0 <disk.vdi|disk.qcow2> [--check]"
[ -f "$DISK" ] || die "no such disk: $DISK"

# A disk that is being written to would give a different hash on every run.
case "$DISK" in
*.vdi)
	lock=$(dirname "$DISK")/$(basename "$DISK" .vdi).vbox
	[ -e "$lock" ] || lock=$(dirname "$DISK")/$(basename "$DISK").lock
	if [ -e "$lock" ] || pgrep -f "VirtualBoxVM.*$(basename "$DISK")" >/dev/null 2>&1; then
		die "the machine looks like it is running: shut it down before signing"
	fi ;;
*.qcow2)
	if pgrep -f "qemu.*$(basename "$DISK")" >/dev/null 2>&1; then
		die "the machine looks like it is running: shut it down before signing"
	fi ;;
*) die "unrecognised disk format: $DISK (expected .vdi or .qcow2)" ;;
esac

printf 'Hashing %s … (this takes a moment on a 20 GiB disk)\n' "$DISK"
sum=$(sha256sum "$DISK" | awk '{print $1}')

if [ "$MODE" = "check" ]; then
	[ -f "$OUT" ] || die "$OUT does not exist: nothing to compare against"
	saved=$(tr -d '[:space:]' < "$OUT")
	if [ "$saved" = "$sum" ]; then
		printf 'signature matches %s\n' "$OUT"
		exit 0
	fi
	printf 'signature MISMATCH\n  file:  %s\n  disk:  %s\n' "$saved" "$sum" >&2
	exit 1
fi

printf '%s\n' "$sum" > "$OUT"
chmod 0644 "$OUT"
printf 'written: %s\n  %s\n' "$OUT" "$sum"
printf 'Remember: any change to the machine makes this signature obsolete.\n'
