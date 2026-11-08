#!/usr/bin/env bash
# Run the full verification suite for ft_printf.
#   ./run_tests.sh          -> norm + differential + valgrind + sanitizers
#   ./run_tests.sh --full   -> also run the Tripouille reference tester (m + b)
set -euo pipefail
cd "$(dirname "$0")"

FULL=0
[ "${1:-}" = "--full" ] && FULL=1

echo "============================================================"
echo " 1. Norminette"
echo "============================================================"
if command -v norminette >/dev/null 2>&1; then
    norminette ./*.c ./*.h
else
    echo "norminette not installed -> skipped (pip install norminette)"
fi

echo
echo "============================================================"
echo " 2. Build (mandatory then bonus), strict flags"
echo "============================================================"
make fclean
make
make bonus

echo
echo "============================================================"
echo " 3. Differential tests (~27 000 cases)"
echo "============================================================"
make -C tests run

echo
echo "============================================================"
echo " 4. valgrind (leaks + memory errors)"
echo "============================================================"
if command -v valgrind >/dev/null 2>&1; then
    make -C tests valgrind
else
    echo "valgrind not installed -> skipped"
fi

echo
echo "============================================================"
echo " 5. AddressSanitizer + UBSan"
echo "============================================================"
make -C tests sanitize

if [ "$FULL" = "1" ]; then
    echo
    echo "============================================================"
    echo " 6. Tripouille reference tester (mandatory + bonus)"
    echo "============================================================"
    if [ -d printfTester ]; then
        export TERM="${TERM:-xterm}"
        make -C printfTester m
        make -C printfTester b
    else
        echo "printfTester/ not present -> skipped"
    fi
fi

echo
echo "============================================================"
echo " ALL CHECKS COMPLETED"
echo "============================================================"
