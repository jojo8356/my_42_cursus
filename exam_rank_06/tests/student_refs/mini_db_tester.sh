#!/bin/bash
# Student-reference tester for mini_db (adapted from cacapon/42_examshell).
# Usage: mini_db_tester.sh <source_dir>
# Compiles <source_dir>/*.cpp and runs the functional command tests.

set +e

SRC_DIR="${1:-../mini_db}"

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

echo -e "${BLUE}🔍 Running tests for mini_db${NC}"
echo "=========================================="
echo ""

WORK="$(mktemp -d)"
trap 'rm -rf "$WORK"; pkill -9 -f "user_mini_db" 2>/dev/null' EXIT

# Test 1: compilation (C++98, strict warnings)
echo -e "${BLUE}📦 Test 1: Checking compilation...${NC}"
g++ -Wall -Wextra -Werror -std=c++98 "$SRC_DIR"/*.cpp -o "$WORK/user_mini_db" 2>&1
if [ $? -ne 0 ]; then
    echo -e "${RED}❌ Compilation failed!${NC}"
    exit 1
fi
echo -e "${GREEN}✅ Compilation successful!${NC}"
echo ""

# Test 2: structural hints
echo -e "${BLUE}📦 Test 2: Checking required components...${NC}"
if grep -q "class\|int[[:space:]]*main" "$SRC_DIR"/*.cpp; then
    echo -e "${GREEN}✅ Found class or main function${NC}"
else
    echo -e "${YELLOW}⚠️  No class or main function detected${NC}"
fi
if grep -q "POST\|GET\|DELETE" "$SRC_DIR"/*.cpp; then
    echo -e "${GREEN}✅ Found database operation hints${NC}"
fi
if grep -q "#include.*socket\|#include.*arpa/inet\|#include.*netinet\|#include.*sys/socket" "$SRC_DIR"/*.cpp "$SRC_DIR"/*.hpp; then
    echo -e "${GREEN}✅ Network includes found${NC}"
fi
if grep -q "ifstream\|ofstream\|fopen\|fclose\|open\|close" "$SRC_DIR"/*.hpp "$SRC_DIR"/*.cpp; then
    echo -e "${GREEN}✅ File I/O functions detected${NC}"
fi
echo ""

# Test 3: functional tests
echo -e "${BLUE}📦 Test 3: Functional tests...${NC}"
rm -f "$WORK/test_db.txt"

PORT=9999
while netstat -tuln 2>/dev/null | grep -q ":$PORT "; do PORT=$((PORT + 1)); done

"$WORK/user_mini_db" "$PORT" "$WORK/test_db.txt" > "$WORK/server_output.txt" 2>&1 &
SERVER_PID=$!
sleep 1

if ! kill -0 $SERVER_PID 2>/dev/null; then
    echo -e "${RED}❌ Server failed to start${NC}"
    cat "$WORK/server_output.txt"
    exit 1
fi

if grep -q "ready" "$WORK/server_output.txt"; then
    echo -e "${GREEN}✅ Server started and printed 'ready'${NC}"
else
    echo -e "${RED}❌ 'ready' message not found${NC}"
fi

FAILED=0
run_check() {
    local label="$1"; local expect="$2"; local input="$3"
    local resp
    resp=$(printf '%s\n' "$input" | nc -w 1 127.0.0.1 "$PORT" 2>/dev/null)
    if echo "$resp" | grep -q "$expect"; then
        echo -e "    ${GREEN}✅ $label${NC}"
    else
        echo -e "    ${RED}❌ $label (response: $resp)${NC}"
        FAILED=1
    fi
}

run_check "POST user john"        "^0"          "POST user john"
run_check "GET user"              "0.*john"     "GET user"
run_check "GET missing returns 1" "^1"          "GET nonexistent"
run_check "DELETE user"           "^0"          "DELETE user"
run_check "GET after DELETE -> 1" "^1"          "GET user"
run_check "unknown returns 2"     "^2"          "INVALID command"

# Persistence
kill -SIGINT $SERVER_PID 2>/dev/null
wait $SERVER_PID 2>/dev/null
if [ -f "$WORK/test_db.txt" ]; then
    echo -e "    ${GREEN}✅ Database file persisted${NC}"
else
    echo -e "    ${RED}❌ Database file not created${NC}"
    FAILED=1
fi

echo ""
echo "=========================================="
if [ "$FAILED" -eq 0 ]; then
    echo -e "${GREEN}PASSED 🎉 All student-reference tests passed${NC}"
    exit 0
else
    echo -e "${RED}FAILED${NC}"
    exit 1
fi
