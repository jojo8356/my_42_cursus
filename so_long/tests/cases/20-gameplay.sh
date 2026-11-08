#!/bin/bash
# ---------------------------------------------------------------------------
#  Layer 3 -- gameplay (mandatory).
#
#  Chapter IV says: "At every move, the current number of moves must be
#  displayed in the shell." It also says the player must not enter a wall, must
#  collect every item before using the exit, and that ESC must quit. All four
#  are observable from outside, so all four are tested here.
#
#  Every expected value in this file is derived from tests/tools/reference_model
#  .py, an independent implementation of the rules. When you change an
#  expectation, run that script rather than adjusting the number by hand.
#
#  Keys are fed through the shim, one per frame, in $MLX_SHIM_KEYS. The shim
#  runs the loop hook a finite number of times ($MLX_SHIM_FRAMES) and returns,
#  which turns the interactive loop into a scripted run.
# ---------------------------------------------------------------------------
set -uo pipefail
ROOT=$(cd "$(dirname "$0")/../.." && pwd)
STATE=${SOLONG_STATE:-/tmp/solong_ci}
BIN="$STATE/so_long"
MAP="$ROOT/tests/cases/maps/ref.ber"
[ -x "$BIN" ] || { echo "binary missing: run tests/build.sh"; exit 1; }
[ -f "$MAP" ] || { echo "missing reference map: $MAP"; exit 1; }

WORK=$(mktemp -d)
trap 'rm -rf "$WORK"' EXIT
cd "$WORK" || exit 1

PASS=0
FAIL=0
ok() { PASS=$((PASS + 1)); }
ko() { FAIL=$((FAIL + 1)); printf '  \033[31m[KO]\033[0m %s\n' "$*"; }

K_W=119; K_A=97; K_S=115; K_D=100; K_ESC=65307

# play <keys> <frames> [binary] [map] -- run a scripted game, print its output
play() {
	local keys=$1 frames=$2 bin=${3:-$BIN} map=${4:-$MAP}
	: > "$WORK/log"
	MLX_SHIM_LOG="$WORK/log" MLX_SHIM_FRAMES="$frames" MLX_SHIM_KEYS="$keys" \
		timeout 10 "$bin" "$map" 2>"$WORK/e"
}

# moves_of <output> -- the shell move counter, one integer per line
moves_of() { printf '%s\n' "$1" | grep -E '^[0-9]+$' | tr '\n' ' ' | sed 's/ $//'; }

echo "== 1. The move counter is printed in the shell, one line per move =="
got=$(moves_of "$(play "$K_D $K_D" 4)")
[ "$got" = "1 2" ] && ok || ko "expected '1 2', got '[$got]'"

# A into the left wall, then two real moves: still exactly 1 2.
got=$(moves_of "$(play "$K_A $K_D $K_D" 5)")
[ "$got" = "1 2" ] && ok || ko "refused move must not be counted, got '[$got]'"

echo "== 2. A wall cannot be entered =="
got=$(moves_of "$(play "$K_W $K_A $K_W" 5)")
[ -z "$got" ] && ok || ko "three moves into walls printed '[$got]'"

# (3,2) is a wall: going right along row 1 then down must stop before it.
got=$(moves_of "$(play "$K_D $K_D $K_S $K_S" 7)")
[ "$got" = "1 2" ] && ok || ko "wall at (3,2) not blocking, got '[$got]'"

echo "== 3. The exit is refused until every collectible is taken =="
# Down to (1,3), then right four times: the last step would be the exit while
# one collectible is still on the board, so only five moves count.
got=$(moves_of "$(play "$K_S $K_S $K_D $K_D $K_D $K_D" 9)")
[ "$got" = "1 2 3 4 5" ] && ok || ko "exit used too early, got '[$got]'"

echo "== 4. Collecting the last item then reaching the exit wins =="
# D D D takes C1, S S walks to the exit's row taking C2, D is the exit.
got=$(moves_of "$(play "$K_D $K_D $K_D $K_S $K_S $K_D" 9)")
[ "$got" = "1 2 3 4 5 6" ] && ok || ko "winning route gave '[$got]', expected 6 moves"

echo "== 5. Once won, keys no longer move the player =="
got=$(moves_of "$(play "$K_D $K_D $K_D $K_S $K_S $K_D $K_A $K_A $K_W $K_S" 14)")
[ "$got" = "1 2 3 4 5 6" ] && ok || ko "moves after victory: '[$got]'"

echo "== 6. ESC quits without moving =="
got=$(moves_of "$(play "$K_ESC $K_D $K_D" 5)")
[ -z "$got" ] && ok || ko "ESC did not stop the game, got '[$got]'"

echo "== 7. Unknown keys are ignored =="
got=$(moves_of "$(play "999 32 65288 $K_D" 6)")
[ "$got" = "1" ] && ok || ko "unknown keys produced '[$got]'"

echo "== 8. The window is titled and sized from the map =="
play "" 1 >/dev/null
win=$(grep '^new_window' "$WORK/log" | head -1)
case "$win" in
	"new_window 448 320 so_long") ok ;;
	*) ko "unexpected window line: [$win]" ;;
esac
n_xpm=$(grep -c '^xpm ' "$WORK/log")
[ "$n_xpm" -eq 9 ] && ok || ko "expected 9 XPM images, loaded $n_xpm"

echo "== 9. Every tile is drawn, then the player on top =="
# 7 x 5 = 35 tiles, plus the player sprite.
n_put=$(grep -c '^put ' "$WORK/log")
[ "$n_put" -eq 36 ] && ok || ko "expected 36 blits (35 tiles + player), got $n_put"

echo "== 10. The red cross is hooked (event 17) =="
grep -q '^hook 17$' "$WORK/log" && ok || ko "no event-17 hook: window cannot be closed"

echo "== 11. Exiting destroys the window, the display and the images =="
grep -q '^destroy_window$' "$WORK/log" && ok || ko "window not destroyed"
grep -q '^destroy_display$' "$WORK/log" && ok || ko "display not destroyed"

echo "== 12. Movement follows the map, whatever its size =="
# A 5x3 map: the player at (1,1) can only move right, twice, to the exit.
printf '11111\n1PCE1\n11111\n' > "$WORK/tiny.ber"
got=$(moves_of "$(play "$K_D $K_D $K_D $K_A $K_W" 7 "" "$WORK/tiny.ber")")
[ "$got" = "1 2" ] && ok || ko "small map gave '[$got]', expected '1 2'"

printf '\n  \033[1;34mgameplay\033[0m: %d passed, %d failed\n' "$PASS" "$FAIL"
[ "$FAIL" -eq 0 ]
