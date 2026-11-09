#!/bin/bash
# ---------------------------------------------------------------------------
#  Builds both flavours headlessly.
#
#  The project uses the minilibX, which needs an X server. This machine has
#  none -- and a CI machine usually has none either -- so the suite links
#  against tests/shim/mlx_shim.c, a drop-in implementation of mlx.h that logs
#  every call instead of drawing. The game code is compiled unchanged, with the
#  same -Wall -Wextra -Werror: what is tested is the real program, only its
#  window is a notebook.
#
#  This script writes the two binaries to $1 (default /tmp/solong_ci) and prints
#  the build log. Any warning is fatal, as in the Makefile.
# ---------------------------------------------------------------------------
set -uo pipefail
ROOT=$(cd "$(dirname "$0")/.." && pwd)
OUT=${1:-/tmp/solong_ci}
cd "$ROOT" || exit 1

CFLAGS="-Wall -Wextra -Werror"
INC="-I include -I libft -I tests/shim"
OBJ="$OUT/obj"

mkdir -p "$OUT" "$OBJ" || exit 1

# libft first: the project's Makefile builds it through its own Makefile
make -C libft >/dev/null 2>&1 || { echo "libft build failed"; exit 1; }

# The source lists come from the Makefile, so the suite can never compile a
# different set of files than the real build. If the two ever drift, that is
# exactly the kind of bug the tests are supposed to catch, not hide.
SOURCES=$(make -s -f Makefile print-sources)
MANDATORY=$(printf '%s\n' "$SOURCES" | sed -n '/^MANDATORY /s///p')
BONUS=$(printf '%s\n' "$SOURCES" | sed -n '/^BONUS /s///p')
[ -n "$MANDATORY" ] && [ -n "$BONUS" ] || { echo "cannot read sources"; exit 1; }

rm -f "$OBJ"/*.o

build() {
	local out=$1
	shift
	local log=""
	for f in "$@"; do
		log=$(cc $CFLAGS $INC -c "$f" -o "$OBJ/$(basename "${f%.c}").o" 2>&1)
		if [ -n "$log" ]; then
			printf '%s\n' "$log"
			return 1
		fi
	done
	log=$(cc $CFLAGS -I tests/shim -c tests/shim/mlx_shim.c \
		-o "$OBJ/mlx_shim.o" 2>&1)
	[ -n "$log" ] && { printf '%s\n' "$log"; return 1; }
	log=$(cc "$OBJ"/*.o libft/libft.a -o "$out" 2>&1)
	[ -n "$log" ] && { printf '%s\n' "$log"; return 1; }
	return 0
}

if ! build "$OUT/so_long" $MANDATORY; then
	echo "FATAL: mandatory build failed"
	exit 1
fi
rm -f "$OBJ"/*.o
if ! build "$OUT/so_long_bonus" $BONUS; then
	echo "FATAL: bonus build failed"
	exit 1
fi

printf '%s\n%s\n' "$OUT/so_long" "$OUT/so_long_bonus"
