#!/usr/bin/env bash
# ****************************************************************************
#   run_tests.sh — orchestrateur des tests de miniRT
# ****************************************************************************
#   Compile et lance les suites de tests headless (shim miniLibX).
set -u

cd "$(dirname "$0")/.."

CC=${CC:-cc}
CFLAGS="-Wall -Wextra -Werror -I include -I libft -I tests -I tests/shim"
SRCS="src/vector.c src/matrix.c src/utils.c src/objects.c src/intersect.c \
src/lighting.c src/camera.c src/render.c src/parser.c src/hooks.c src/mt.c \
src/texture.c"
LDLIBS="libft/libft.a -lm"
SHIM="tests/shim/mlx_shim.c"

# Construire libft si absent
if [ ! -f libft/libft.a ]; then
    echo "Build libft..."
    make -C libft >/dev/null 2>&1
fi

PASS=0
FAIL=0
SUITES="test_vector test_matrix test_objects test_intersect test_lighting \
test_camera test_parser test_render test_integration"

echo "═════════════════════════════════════════════════════════════════════"
echo "  miniRT — Test Suite (headless, shim miniLibX)"
echo "═════════════════════════════════════════════════════════════════════"

for suite in $SUITES; do
    echo ""
    echo "→ Compiling & running: $suite"
    $CC $CFLAGS tests/$suite.c $SRCS $SHIM $LDLIBS -o tests/$suite 2>tests/$suite.err
    if [ $? -ne 0 ]; then
        echo "  ❌ COMPILE ERROR"
        cat tests/$suite.err
        FAIL=$((FAIL + 1))
        continue
    fi
    MLX_SHIM_LOG=/tmp/minrt_${suite}.log MLX_SHIM_FRAMES=5 ./tests/$suite
    rc=$?
    if [ $rc -eq 0 ]; then
        PASS=$((PASS + 1))
    else
        FAIL=$((FAIL + 1))
    fi
done

echo ""
echo "═════════════════════════════════════════════════════════════════════"
if [ $FAIL -eq 0 ]; then
    echo "  Result: $PASS/$PASS suites passed"
    echo "  ✅ ALL TESTS PASSED"
else
    echo "  Result: $PASS suites passed, $FAIL failed"
    echo "  ❌ $FAIL SUITE(S) FAILED"
fi
echo "═════════════════════════════════════════════════════════════════════"
[ $FAIL -eq 0 ]
