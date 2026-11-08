#!/bin/bash
# ---------------------------------------------------------------------------
#  Layer 6 -- compliance with the subject and the Norm.
#
#  norminette is not installable here, so the Norm's mechanical rules are
#  checked directly: 80-column lines, tabs for indentation, no space after a
#  keyword, a header on every file, five functions per file at most, and the
#  bonus in _bonus.c files as chapter III demands.
#
#  The structural checks matter as much: a reviewer opens the repository, not a
#  binary, so the Makefile rules, the asset set and the subject transcription
#  are part of the deliverable.
# ---------------------------------------------------------------------------
set -uo pipefail
ROOT=$(cd "$(dirname "$0")/../.." && pwd)
cd "$ROOT" || exit 1

PASS=0
FAIL=0
ok() { PASS=$((PASS + 1)); }
ko() { FAIL=$((FAIL + 1)); printf '  \033[31m[KO]\033[0m %s\n' "$*"; }

SOURCES=$(find src include -name '*.c' -o -name '*.h' 2>/dev/null | sort)
[ -n "$SOURCES" ] || { echo "no sources found"; exit 1; }

echo "== 1. Every source carries a 42 header =="
bad=0
for f in $SOURCES; do
	head -1 "$f" | grep -q '^/\* \*\*\*' || { bad=$((bad + 1)); \
		printf '       %s\n' "$f"; }
done
[ "$bad" -eq 0 ] && ok || ko "$bad file(s) without header"

echo "== 2. No line exceeds 80 columns =="
bad=0
for f in $SOURCES; do
	n=$(awk 'length($0) > 80' "$f" | wc -l)
	[ "$n" -eq 0 ] || { bad=$((bad + 1)); \
		printf '       %s: %d line(s)\n' "$f" "$n"; }
done
[ "$bad" -eq 0 ] && ok || ko "$bad file(s) over 80 columns"

echo "== 3. Indentation uses tabs, not runs of spaces =="
bad=0
for f in $SOURCES; do
	# four leading spaces where a tab belongs is the classic Norm mistake
	n=$(grep -cE '^    [^ *]' "$f")
	[ "$n" -eq 0 ] || { bad=$((bad + 1)); printf '       %s: %d\n' "$f" "$n"; }
done
[ "$bad" -eq 0 ] && ok || ko "$bad file(s) indented with spaces"

echo "== 4. At most five functions per source file =="
bad=0
for f in $(find src -name '*.c' | sort); do
	# a definition is a line starting at column 0, with a return type, a name
	# and an opening parenthesis, followed by a brace block
	n=$(awk '/^[a-zA-Z_].*\(/ && !/;[[:space:]]*$/ {c++} END {print c+0}' "$f")
	[ "$n" -le 5 ] || { bad=$((bad + 1)); printf '       %s: %d\n' "$f" "$n"; }
done
[ "$bad" -eq 0 ] && ok || ko "$bad file(s) with more than five functions"

echo "== 5. No forbidden function is used =="
ALLOWED='open|close|read|write|printf|malloc|free|perror|strerror|exit|mlx_'
bad=0
for f in $SOURCES; do
	# calls that look like libc but are not on the allowed list
	hits=$(grep -oE '\b(fopen|fclose|fread|fwrite|strdup|calloc|realloc|snprintf|sprintf|fprintf|puts|putendl|strcat|strcpy|strncpy|memmove|qsort|getline|stat|access|opendir|readdir|system|fork|exec[a-z]*)\s*\(' "$f" \
		| sed 's/\s*($//' | sort -u | tr '\n' ' ')
	if [ -n "$hits" ]; then
		# ft_ helpers of the vendored libft are allowed
		case "$hits" in
			*ft_*) : ;;
		esac
		bad=$((bad + 1))
		printf '       %s: %s\n' "$f" "$hits"
	fi
done
[ "$bad" -eq 0 ] && ok || ko "$bad file(s) use a non-allowed function"

echo "== 6. The bonus lives in _bonus files =="
# chapter III: "the bonus part must be in files ending with _bonus.c/_bonus.h"
bad=0
for f in $(find src/bonus -name '*.c' 2>/dev/null); do
	case "$f" in
		*_bonus.c) : ;;
		*) bad=$((bad + 1)); printf '       %s does not end in _bonus.c\n' "$f" ;;
	esac
done
[ -d src/bonus ] && [ "$bad" -eq 0 ] && ok || ko "bonus files not named _bonus.c"

echo "== 7. Mandatory sources contain no bonus symbol =="
# the mandatory binary must be linkable without a single bonus file
bad=0
for f in $(make -s print-sources | sed -n '/^MANDATORY /s///p' | tr ' ' '\n' \
		| grep -v bonus_stub); do
	if grep -qE '\b(bonus_tick|bonus_spawn_enemies|bonus_draw_hud|t_enemy)\b' \
		"$f"; then
		bad=$((bad + 1))
		printf '       %s references a bonus symbol\n' "$f"
	fi
done
[ "$bad" -eq 0 ] && ok || ko "$bad mandatory file(s) reference the bonus"

echo "== 8. The Makefile has the five required rules =="
for rule in all clean fclean re bonus; do
	make -n "$rule" >/dev/null 2>&1 && ok || ko "make $rule does not work"
done

echo "== 9. No object file or binary is tracked by git =="
if git -C "$ROOT" rev-parse --git-dir >/dev/null 2>&1; then
	# only look at paths under so_long
	tracked=$(git -C "$ROOT" ls-files 'so_long/*' \
		| grep -E '\.(o|a)$|^so_long/(so_long|so_long_bonus)$' || true)
	[ -z "$tracked" ] && ok || ko "build artefacts are tracked: $tracked"
else
	ok
fi

echo "== 10. The nine sprite images exist and are valid XPM =="
bad=0
for name in floor wall collect exit exit_open player1 player2 enemy1 enemy2; do
	f="assets/$name.xpm"
	if [ ! -f "$f" ]; then
		bad=$((bad + 1)); printf '       missing %s\n' "$f"; continue
	fi
	head -2 "$f" | grep -q 'XPM' || { bad=$((bad + 1)); \
		printf '       %s is not XPM\n' "$f"; }
	grep -qE '"[0-9]+ [0-9]+ [0-9]+ [0-9]+"' "$f" || { bad=$((bad + 1)); \
		printf '       %s has no dimensions line\n' "$f"; }
done
[ "$bad" -eq 0 ] && ok || ko "$bad problem(s) with the sprite set"

echo "== 11. The map assets referenced by the code all exist =="
bad=0
for f in $(grep -ohE 'assets/[a-z0-9_]+\.xpm' src/render.c | sort -u); do
	[ -f "$f" ] || { bad=$((bad + 1)); printf '       missing %s\n' "$f"; }
done
[ "$bad" -eq 0 ] && ok || ko "the code references $bad missing image(s)"

echo "== 12. The subject is shipped in docs/ =="
[ -f docs/en.subject.pdf ] && ok || ko "docs/en.subject.pdf missing"
if head -c 5 docs/en.subject.pdf 2>/dev/null | grep -q '%PDF'; then
	ok
else
	ko "docs/en.subject.pdf is not a PDF"
fi
[ -s docs/en.subject.md ] && ok || ko "docs/en.subject.md missing"

echo "== 13. Both READMEs exist and follow the house convention =="
for f in README.md README.fr.md; do
	[ -s "$f" ] || { ko "$f missing"; continue; }
	head -1 "$f" | grep -q 'part of the 42 curriculum' && ok \
		|| ko "$f does not open with the curriculum line"
done

echo "== 14. The third-party tester is preserved verbatim =="
t="$ROOT/tests/community/augustobecker"
n_scripts=$(find "$t/tests" -name 'check-*.sh' 2>/dev/null | wc -l)
[ "$n_scripts" -ge 13 ] && ok \
	|| ko "only $n_scripts of the tester's 13 check scripts are present"
n=$(find "$t/maps/invalid" -type f 2>/dev/null | wc -l)
[ "$n" -ge 30 ] && ok || ko "only $n invalid maps preserved"

printf '\n  \033[1;34mcompliance\033[0m: %d passed, %d failed\n' "$PASS" "$FAIL"
[ "$FAIL" -eq 0 ]
