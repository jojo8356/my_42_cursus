#!/bin/sh
# ──────────────────────────────────────────────────────────────────────
# fdf test orchestrator
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
LDFLAGS="-lm"

# ---- build libft ----
if [ ! -f ../libft/libft.a ]; then
    echo "→ Building libft…"
    make -C ../libft
fi

# ---- build test binaries ----
CORE_SRCS="src/parse.c src/math.c src/color.c src/draw.c"

echo ""
echo "════════════════════════════════════════════════════════════"
echo "  fdf — Test Suite"
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

run_test test_parse  tests/test_parse.c
run_test test_math   tests/test_math.c
run_test test_draw   tests/test_draw.c
run_test test_color  tests/test_color.c

# test_hooks needs hooks.c + mlx stubs (hooks.c references mlx functions).
echo ""
echo "→ Compiling & running: test_hooks"
if $CC $CFLAGS -I tests tests/test_hooks.c $CORE_SRCS src/hooks.c tests/test_mlx_stub.c ../libft/libft.a $LDFLAGS -o "tests/test_hooks" 2>"tests/test_hooks.err"; then
    if "tests/test_hooks"; then
        PASS=$((PASS + 1))
    else
        FAIL=$((FAIL + 1))
        echo "  ❌ test_hooks FAILED"
    fi
else
    FAIL=$((FAIL + 1))
    echo "  ❌ test_hooks COMPILE ERROR:"
    cat "tests/test_hooks.err"
fi

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
