#!/bin/bash
# Lance les tests officiels libstdc++ (GCC 4.9.4, ère C++98) contre les headers ft.
# Usage: ./run.sh [dirs...]   (défaut = tous les containers + iterators)
set -u
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"   # ft_containers/
cd "$ROOT" || exit 1
INC="-Iincludes -Itests/official/shim -Itests/official/util"
LIB="tests/official/obj/libtestsuite.a"
CXXFLAGS="-std=c++98 -w $INC"
RES="tests/official/results"
mkdir -p "$RES"

if [ $# -gt 0 ]; then
    DIRS="$*"
else
    DIRS="23_containers/vector 23_containers/map 23_containers/set 23_containers/deque \
23_containers/queue 23_containers/stack 23_containers/multimap 23_containers/multiset \
23_containers/list 23_containers/priority_queue 24_iterators"
fi

PASS=0; FAIL=0; COMPILE_FAIL=0; TIMEOUT=0
FAILED_LIST=""
for d in $DIRS; do
    for f in $(find tests/official/$d -name "*.cc" 2>/dev/null | sort); do
        base=$(basename "$f" .cc)
        rel="${f#tests/official/}"
        bin="/home/user/mres/off_${d//\//_}_${base}"
        # Compile-only tests (no main) only need to compile, not link/run.
        if ! grep -q "main[[:space:]]*(" "$f"; then
            if g++ $CXXFLAGS -c "$f" -o "$bin.o" > "$RES/${d//\//_}_${base}.comp" 2>&1; then
                PASS=$((PASS+1))
            else
                COMPILE_FAIL=$((COMPILE_FAIL+1))
                FAILED_LIST="$FAILED_LIST\nCOMPILE_FAIL  $rel"
            fi
            rm -f "$bin.o"
            continue
        fi
        # compile + link
        if ! g++ $CXXFLAGS "$f" "$LIB" -o "$bin" > "$RES/${d//\//_}_${base}.comp" 2>&1; then
            COMPILE_FAIL=$((COMPILE_FAIL+1))
            FAILED_LIST="$FAILED_LIST\nCOMPILE_FAIL  $rel"
            continue
        fi
        # run
        timeout 10 "$bin" > "$RES/${d//\//_}_${base}.out" 2>&1
        rc=$?
        if [ $rc -eq 0 ]; then
            PASS=$((PASS+1))
        elif [ $rc -eq 124 ]; then
            TIMEOUT=$((TIMEOUT+1))
            FAILED_LIST="$FAILED_LIST\nTIMEOUT      $rel"
        else
            FAIL=$((FAIL+1))
            FAILED_LIST="$FAILED_LIST\nFAIL($rc)    $rel"
        fi
        rm -f "$bin"
    done
done

echo "======================================"
echo "PASS: $PASS   FAIL: $FAIL   COMPILE_FAIL: $COMPILE_FAIL   TIMEOUT: $TIMEOUT"
echo "======================================"
[ -n "$FAILED_LIST" ] && printf "%b\n" "$FAILED_LIST"
