#!/bin/sh
# ──────────────────────────────────────────────────────────────────────
# fract-ol test orchestrator
#
# Builds the project and runs all headless test suites (no X11 required).
# The mlx-dependent parts (window creation, event loop) are tested on the
# user's machine where X11 is available.
#
# Usage:  ./tests/run_tests.sh
# Exit:   0 = all passed, 1 = some failed
# ──────────────────────────────────────────────────────────────────────
set -e

SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
ROOT_DIR=$(CDPATH= cd -- "$SCRIPT_DIR/.." && pwd)
cd "$ROOT_DIR"

CC=${CC:-gcc}
CFLAGS="-Wall -Wextra -O2 -g -fno-omit-frame-pointer"
CFLAGS="$CFLAGS -I includes -I ../libft -I ../mlx"
LDFLAGS="-lm -lpthread"

# mlx is not required for headless tests (only math/color/transform/args/thread).
# We compile the test sources directly, excluding mlx-dependent files.

# ---- build libft ----
if [ ! -f ../libft/libft.a ]; then
    echo "→ Building libft…"
    make -C ../libft
fi

# ---- build test binaries ----
# Each test file is compiled with the core sources (no mlx).
CORE_SRCS="src/fractal_math.c src/color.c src/transform.c src/args.c src/render_core.c"

echo ""
echo "════════════════════════════════════════════════════════════"
echo "  fract-ol — Test Suite"
echo "════════════════════════════════════════════════════════════"

PASS=0
FAIL=0

run_test() {
    name=$1
    src=$2
    echo ""
    echo "→ Compiling & running: $name"
    if $CC $CFLAGS -I tests $src $CORE_SRCS ../libft/libft.a $LDFLAGS -o "tests/$name" 2>"tests/$name.err"; then
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

run_test test_math    tests/test_math.c
run_test test_color   tests/test_color.c
run_test test_transform tests/test_transform.c
run_test test_args    tests/test_args.c
run_test test_render  tests/test_render.c
run_test test_thread  tests/test_thread.c

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
