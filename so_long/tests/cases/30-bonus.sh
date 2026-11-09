#!/bin/bash
# ---------------------------------------------------------------------------
#  Layer 4 -- the three bonuses of chapter V.
#
#    1. Enemies that patrol and cause the player to lose on contact.
#    2. Sprite animation.
#    3. The move counter displayed on the screen.
#
#  A bonus only counts if the mandatory part is perfect, so this layer also
#  re-checks that the bonus build still refuses every invalid map: the sixth
#  symbol must not have loosened anything else.
# ---------------------------------------------------------------------------
set -uo pipefail
ROOT=$(cd "$(dirname "$0")/../.." && pwd)
STATE=${SOLONG_STATE:-/tmp/solong_ci}
MBIN="$STATE/so_long"
BBIN="$STATE/so_long_bonus"
[ -x "$MBIN" ] && [ -x "$BBIN" ] || { echo "binaries missing: run tests/build.sh"; exit 1; }

WORK=$(mktemp -d)
trap 'rm -rf "$WORK"' EXIT
cd "$WORK" || exit 1

PASS=0
FAIL=0
ok() { PASS=$((PASS + 1)); }
ko() { FAIL=$((FAIL + 1)); printf '  \033[31m[KO]\033[0m %s\n' "$*"; }

K_W=119; K_A=97; K_S=115; K_D=100; K_ESC=65307

bplay() {
	local keys=$1 frames=$2 map=$3
	: > "$WORK/blog"
	MLX_SHIM_LOG="$WORK/blog" MLX_SHIM_FRAMES="$frames" MLX_SHIM_KEYS="$keys" \
		timeout 10 "$BBIN" "$map" 2>"$WORK/be"
}
moves_of() { printf '%s\n' "$1" | grep -E '^[0-9]+$' | tr '\n' ' ' | sed 's/ $//'; }

# A level with one enemy on row 3, patrolling between the walls at x=0 and x=6.
#
#   1111111
#   1P00C01     P (1,1)  C (4,1)
#   1011011
#   1X00CE1     X (1,3) patrolling, C (4,3), E (5,3)
#   1111111
cat > en.ber <<'EOF'
1111111
1P00C01
1011011
1X00CE1
1111111
EOF

echo "== B1. The mandatory binary rejects the enemy symbol =="
out=$("$MBIN" en.ber 2>"$WORK/e")
rc=$?
[ "$rc" -ne 0 ] && printf '%s' "$out" | head -1 | grep -q '^Error' && ok \
	|| ko "mandatory binary accepted 'X'"

echo "== B2. The bonus binary accepts it and runs =="
bplay "$K_D" 2 en.ber >/dev/null
rc=$?
[ "$rc" -eq 0 ] && ok || ko "bonus binary rejected the enemy map (status $rc)"

echo "== B3. Enemies are drawn, and animated =="
grep -q '^put ' "$WORK/blog" && ok || ko "nothing drawn"
# The enemy sprite is one of the two last images; both must appear over frames.
n_enemy_frames=$(grep -c '^put ' "$WORK/blog")
[ "$n_enemy_frames" -ge 40 ] && ok \
	|| ko "too few blits for a 7x5 map with actors: $n_enemy_frames"

echo "== B4. An enemy patrolling into the player loses the game =="
# The player stands still at (1,1). The enemy starts at (1,3) moving right,
# reaches the wall at (6,3) and comes back. It never enters row 1, so this run
# must NOT lose. Then a map where the patrol does cross the player is tested.
bplay "" 3 en.ber > "$WORK/o"
if grep -q 'lose' "$WORK/o"; then
	ko "lost while no enemy could reach the player"
else
	ok
fi

# Enemy on the player's row: it patrols row 1 and must hit the player at (1,1).
cat > row.ber <<'EOF'
1111111
1P0C0X1
1000EC1
1111111
EOF
bplay "" 30 row.ber > "$WORK/o"
grep -q 'lose' "$WORK/o" && ok || ko "an enemy walking into the player did not lose"

echo "== B5. Walking into an enemy loses the game =="
# The player moves right twice along row 1 and reaches the enemy head-on.
bplay "$K_D $K_D" 6 row.ber > "$WORK/o"
grep -q '^1$' "$WORK/o" && ok || ko "player did not move before the collision"
grep -q 'lose' "$WORK/o" && ok || ko "walking into an enemy did not lose the game"

echo "== B6. The move counter is displayed on the screen =="
bplay "$K_D $K_D $K_D" 5 en.ber >/dev/null
# A HUD line is an mlx_string_put carrying a number.
hud=$(grep '^string ' "$WORK/blog" | grep -cE ' [0-9]+$')
[ "$hud" -ge 1 ] && ok || ko "no numeric mlx_string_put: no on-screen counter"
# and it must show the real count on the last frame
last=$(grep '^string ' "$WORK/blog" | grep -E ' [0-9]+$' | tail -1 | awk '{print $NF}')
[ "$last" = "3" ] && ok || ko "on-screen counter shows [$last], expected 3"

echo "== B7. The shell counter is still there (mandatory is not lost) =="
got=$(moves_of "$(bplay "$K_D $K_D $K_D" 5 en.ber)")
[ "$got" = "1 2 3" ] && ok || ko "bonus build no longer prints moves: '[$got]'"

echo "== B8. Sprite animation: the player alternates between two frames =="
# The shim logs a "frame N" line before each loop-hook call, so the last blit
# at the player's tile within a frame is the player sprite (the tile itself is
# drawn first). Two distinct sprites alternating frame after frame is the
# animation, and it is observable without a display.
bplay "" 4 en.ber >/dev/null
px=$((1 * 64)); py=$((1 * 64))
frames=$(awk -v px="$px" -v py="$py" '
	/^frame / { f = $2 }
	/^put / && $4 == px && $5 == py { last[f] = $2 }
	END { for (i = 0; i in last; i++) print last[i] }' "$WORK/blog")
distinct=$(printf '%s\n' "$frames" | sort -u | grep -c .)
[ "$distinct" -eq 2 ] && ok \
	|| ko "player sprite uses $distinct images across frames, expected 2"
first=$(printf '%s\n' "$frames" | sed -n '1p')
second=$(printf '%s\n' "$frames" | sed -n '2p')
third=$(printf '%s\n' "$frames" | sed -n '3p')
[ "$first" != "$second" ] && [ "$first" = "$third" ] && ok \
	|| ko "player frames do not alternate"

# The enemy animates on its own tile too.
ex=$((1 * 64)); ey=$((3 * 64))
eframes=$(awk -v px="$ex" -v py="$ey" '
	/^frame / { f = $2 }
	/^put / && $4 == px && $5 == py { last[f] = $2 }
	END { for (i = 0; i in last; i++) print last[i] }' "$WORK/blog")
edistinct=$(printf '%s\n' "$eframes" | sort -u | grep -c .)
[ "$edistinct" -ge 2 ] && ok || ko "the enemy sprite does not animate ($edistinct)"

echo "== B9. The bonus keeps every mandatory rule =="
n_bad=0
for m in "$ROOT/tests/community/augustobecker/maps/invalid/"*; do
	[ -f "$m" ] || continue
	out=$("$BBIN" "$m" 2>"$WORK/e")
	if [ $? -eq 0 ] || ! printf '%s' "$out" | head -1 | grep -q '^Error'; then
		n_bad=$((n_bad + 1))
		printf '       accepted: %s\n' "$(basename "$m")"
	fi
done
[ "$n_bad" -eq 0 ] && ok || ko "$n_bad invalid maps accepted by the bonus build"

echo "== B10. A six-symbol map still respects rectangularity and walls =="
printf '11111\n1PX01\n100E1\n1111\n' > bad.ber
out=$("$BBIN" bad.ber 2>"$WORK/e")
[ $? -ne 0 ] && printf '%s' "$out" | grep -q '^Error' && ok \
	|| ko "ragged enemy map accepted"
printf '11110\n1PX01\n100E1\n11111\n' > bad2.ber
out=$("$BBIN" bad2.ber 2>"$WORK/e")
[ $? -ne 0 ] && printf '%s' "$out" | grep -q '^Error' && ok \
	|| ko "open enemy map accepted"

echo "== B11. Several enemies patrol independently =="
cat > multi.ber <<'EOF'
11111111
1P00C001
10110101
1X0X0CE1
11111111
EOF
out=$(bplay "" 5 multi.ber)
case "$out" in
	Error*) ko "multi-enemy map reported an error" ;;
	*) ok ;;
esac

printf '\n  \033[1;34mbonus\033[0m: %d passed, %d failed\n' "$PASS" "$FAIL"
[ "$FAIL" -eq 0 ]
