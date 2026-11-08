#!/bin/bash
# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    run_tests.sh                                        :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2026/11/08 22:30:00 by jojo8356          #+#    #+#              #
#    Updated: 2026/11/08 22:30:00 by jojo8356         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

# ---------------------------------------------------------------------------
#  Test suite for so_long -- one entry point.
#
#      bash run_tests.sh              build, then run every layer
#      bash run_tests.sh --no-build   reuse the binaries of a previous run
#      bash run_tests.sh <name>       run one layer only (see LAYERS below)
#
#  The project draws with the minilibX, which needs an X server. This machine
#  has none, so every layer runs the game headlessly: tests/build.sh links the
#  real sources against tests/shim/mlx_shim.c, an implementation of mlx.h that
#  records every call instead of drawing. The game code under test is unchanged
#  and compiled with the same -Wall -Wextra -Werror as the Makefile.
#
#  What that buys, and what it costs:
#
#    + parsing, validation, exit status, move counting, win/lose conditions,
#      hook registration, image loading and even sprite animation are all
#      observable, because the shim logs them;
#    - actual pixels are not verified. Nothing here proves the window looks
#      right, only that the right image went to the right tile.
#
#  On a machine with X11, `make` and `make bonus` build the real thing; this
#  suite then exercises the same code with a notebook instead of a screen.
# ---------------------------------------------------------------------------
set -uo pipefail

ROOT=$(cd "$(dirname "$0")" && pwd)
cd "$ROOT" || exit 1

STATE=${SOLONG_STATE:-/tmp/solong_ci}
export SOLONG_STATE="$STATE"

BUILD=1
ONLY=""
for arg in "$@"; do
	case "$arg" in
		--no-build) BUILD=0 ;;
		-h|--help) sed -n '15,40p' "$0" | sed 's/^# \{0,1\}//'; exit 0 ;;
		*) ONLY="$arg" ;;
	esac
done

# name|script|what it covers
LAYERS="
parse|tests/cases/10-map-validation.sh|the ten map rules of chapter IV, plus 41 corpus maps
game|tests/cases/20-gameplay.sh|movements, walls, exit, win, ESC, rendering
bonus|tests/cases/30-bonus.sh|enemies, animation, on-screen counter
memory|tests/cases/40-memory.sh|ASan+UBSan over every map, no leaks, no crash
compliance|tests/cases/50-compliance.sh|Norm rules, Makefile, assets, docs, READMEs
community|tests/cases/60-community.sh|augustobecker/so_long_tester, verbatim
"

bold=$(printf '\033[1m')
dim=$(printf '\033[2m')
red=$(printf '\033[31m')
green=$(printf '\033[32m')
blue=$(printf '\033[34m')
reset=$(printf '\033[0m')

printf '%s\n' "${bold}so_long test suite${reset}"
printf '%s\n\n' "${dim}root: $ROOT${reset}"

if [ "$BUILD" -eq 1 ]; then
	printf '%s\n' "${blue}== build ==${reset}"
	if ! bash tests/build.sh "$STATE"; then
		printf '%s\n' "${red}build failed, stopping${reset}"
		exit 1
	fi
	printf '\n'
fi

[ -x "$STATE/so_long" ] || { printf '%s\n' \
	"${red}no binary: run without --no-build${reset}"; exit 1; }

PASSED=0
FAILED=0
SKIPPED=0
RESULTS=""

while IFS='|' read -r name script what; do
	[ -n "$name" ] || continue
	if [ -n "$ONLY" ] && [ "$ONLY" != "$name" ]; then
		continue
	fi
	printf '%s\n' "${bold}-- $name: $what${reset}"
	if [ ! -f "$script" ]; then
		printf '%s\n\n' "${red}missing script: $script${reset}"
		FAILED=$((FAILED + 1))
		RESULTS="$RESULTS$name|missing|$what\n"
		continue
	fi
	start=$(date +%s)
	if bash "$script" > "$STATE/$name.log" 2>&1; then
		end=$(date +%s)
		printf '%s\n\n' "${green}   passed${reset} ${dim}($((end - start))s)${reset}"
		PASSED=$((PASSED + 1))
		RESULTS="$RESULTS$name|pass|$what\n"
	else
		end=$(date +%s)
		printf '%s\n' "${red}   FAILED${reset} ${dim}($(($end - $start))s)${reset}"
		# show the failing lines, then where the full log is
		grep -E '\[KO\]|KO\]|ERROR|FATAL' "$STATE/$name.log" | head -12 \
			| sed 's/^/     /'
		printf '%s\n\n' "${dim}   full log: $STATE/$name.log${reset}"
		FAILED=$((FAILED + 1))
		RESULTS="$RESULTS$name|FAIL|$what\n"
	fi
done <<EOF
$(printf '%s' "$LAYERS" | grep -v '^$')
EOF

printf '%s\n' "${bold}== summary ==${reset}"
printf '%b' "$RESULTS" | while IFS='|' read -r name status what; do
	[ -n "$name" ] || continue
	case "$status" in
		pass) mark="${green}PASS${reset}" ;;
		*) mark="${red}FAIL${reset}" ;;
	esac
	printf '  %s  %-12s %s\n' "$mark" "$name" "$what"
done
printf '\n  %d passed, %d failed\n' "$PASSED" "$FAILED"

[ "$FAILED" -eq 0 ] || exit 1
printf '%s\n' "${green}all layers passed${reset}"
exit 0
