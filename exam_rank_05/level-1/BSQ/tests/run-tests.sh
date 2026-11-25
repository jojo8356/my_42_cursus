#!/usr/bin/env bash

set -u

bin=${1:-./bsq}
root=$(cd "$(dirname "$0")" && pwd)
temp=$(mktemp -d)
passed=0
failed=0

trap 'rm -rf "$temp"' EXIT

if [ ! -x "$bin" ]; then
	echo "❌ Executable not found: $bin"
	echo "Usage: $0 ./bsq"
	exit 1
fi

showfile()
{
	local file=$1

	if [ ! -s "$file" ]; then
		echo "<empty>"
	else
		sed -n 'l' "$file"
	fi
}

runone()
{
	local input=$1
	local name
	local code

	name=$(basename "$input" .map)

	echo
	echo "========================================"
	echo "🧪 TEST: $name"
	echo "========================================"

	echo
	echo "📥 Input:"
	showfile "$input"

	timeout 3 "$bin" "$input" \
		> "$temp/$name.out" \
		2> "$temp/$name.err"
	code=$?

	echo
	echo "📤 Actual stdout:"
	showfile "$temp/$name.out"

	echo
	echo "⚠️  Actual stderr:"
	showfile "$temp/$name.err"

	echo
	echo "🔢 Exit code: $code"

	if [ "$code" -eq 124 ]; then
		echo
		echo "❌ RESULT: TIMEOUT"
		failed=$((failed + 1))
		return
	fi

	if cmp -s "$temp/$name.out" "$root/expected-out/$name.out" \
		&& cmp -s "$temp/$name.err" "$root/expected-err/$name.err"; then
		echo
		echo "✅ RESULT: PASS"
		passed=$((passed + 1))
		return
	fi

	echo
	echo "❌ RESULT: FAIL"

	echo
	echo "📋 Expected stdout:"
	showfile "$root/expected-out/$name.out"

	echo
	echo "📋 Expected stderr:"
	showfile "$root/expected-err/$name.err"

	echo
	echo "🔍 stdout diff:"
	diff -u "$root/expected-out/$name.out" "$temp/$name.out" || true

	echo
	echo "🔍 stderr diff:"
	diff -u "$root/expected-err/$name.err" "$temp/$name.err" || true

	failed=$((failed + 1))
}

echo "========================================"
echo "              BSQ TESTS"
echo "========================================"

for input in "$root"/input/*.map
do
	runone "$input"
done

echo
echo "========================================"
echo "📊 FINAL RESULT"
echo "========================================"
echo "✅ Passed: $passed"
echo "❌ Failed: $failed"
echo "📦 Total:  $((passed + failed))"
echo "========================================"

if [ "$failed" -ne 0 ]; then
	exit 1
fi
