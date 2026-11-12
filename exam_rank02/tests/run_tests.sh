#!/bin/sh
# ──────────────────────────────────────────────────────────────────────
# exam_rank02 test orchestrator
#
# Builds the project and runs all test suites (one per level).
# Each level is compiled with the shared header and libft.
#
# Usage:  ./tests/run_tests.sh
# Exit:   0 = all passed, 1 = some failed
# ──────────────────────────────────────────────────────────────────────
set -e

SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
ROOT_DIR=$(CDPATH= cd -- "$SCRIPT_DIR/.." && pwd)
cd "$ROOT_DIR"

CC=${CC:-cc}
CFLAGS="-Wall -Wextra -Werror -g"
INCLUDES="-I include -I libft"
LDFLAGS=""

# ---- build libft ----
if [ ! -f libft/libft.a ]; then
    echo "→ Building libft…"
    make -C libft
fi

# ---- build test binaries ----
CORE_SRCS="level_1/level_1.c level_2/level_2.c level_3/level_3.c level_4/level_4.c"

echo ""
echo "════════════════════════════════════════════════════════════"
echo "  exam_rank02 — Test Suite"
echo "════════════════════════════════════════════════════════════"

PASS=0
FAIL=0

run_test() {
    name=$1
    src=$2
    echo ""
    echo "→ Compiling & running: $name"
    if $CC $CFLAGS $INCLUDES -I tests $src $CORE_SRCS libft/libft.a $LDFLAGS -o "tests/$name" 2>"tests/$name.err"; then
        if "tests/$name"; then
            PASS=$((PASS + 1))
        else
            FAIL=$((FAIL + 1))
            echo "  ❌ $name FAILED"
        fi
    else
        FAIL=$((FAIL + 1))
        echo "  ❌ $name COMPILE ERROR:"
        cat "tests/$name.err"
    fi
}

run_test test_level_1 tests/test_level_1.c
run_test test_level_2 tests/test_level_2.c
run_test test_level_3 tests/test_level_3.c
run_test test_level_4 tests/test_level_4.c

# ---- summary ----
echo ""
echo "════════════════════════════════════════════════════════════"
echo "  Test Summary: $PASS passed, $FAIL failed"
if [ "$FAIL" -eq 0 ]; then
    echo "  ✅ ALL TESTS PASSED"
else
    echo "  ❌ $FAIL TEST SUITE(S) FAILED"
fi
echo "════════════════════════════════════════════════════════════"

# cleanup
rm -f tests/test_*.err

exit $FAIL
