#!/usr/bin/env bash

set -u

bin=${1:-./life}
root=$(cd "$(dirname "$0")" && pwd)
temp=$(mktemp -d)
pass=0
fail=0

trap 'rm -rf "$temp"' EXIT

if [ ! -x "$bin" ]; then
	echo "❌ Executable not found: $bin"
	echo "Usage: $0 ./life"
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
	local file=$1
	local name
	local args
	local code

	name=$(basename "$file" .in)
	args=$(cat "$root/args/$name.args")

	timeout 3 "$bin" $args \
		< "$file" \
		> "$temp/$name.out" \
		2> "$temp/$name.err"
	code=$?

	if [ "$code" -eq 124 ]; then
		echo "❌ $name — timeout"
		fail=$((fail + 1))
		return
	fi

	if [ "$name" != "${name#random-}" ]; then
		if cmp -s "$temp/$name.out" "$root/expected/$name.out" \
			&& [ ! -s "$temp/$name.err" ]; then
			echo "✅ $name"
			pass=$((pass + 1))
		else
			echo
			echo "========================================"
			echo "❌ TEST: $name"
			echo "========================================"
			echo "Args: $args"
			echo
			echo "Input:"
			showfile "$file"
			echo
			echo "Expected stdout:"
			showfile "$root/expected/$name.out"
			echo
			echo "Actual stdout:"
			showfile "$temp/$name.out"
			echo
			echo "Actual stderr:"
			showfile "$temp/$name.err"
			echo
			diff -u "$root/expected/$name.out" "$temp/$name.out" || true
			fail=$((fail + 1))
		fi
		return
	fi

	echo
	echo "========================================"
	echo "🧪 TEST: $name"
	echo "========================================"
	echo "Args: $args"
	echo
	echo "Input:"
	showfile "$file"

	echo
	echo "Actual stdout:"
	showfile "$temp/$name.out"

	echo
	echo "Actual stderr:"
	showfile "$temp/$name.err"

	echo
	echo "Exit code: $code"

	if cmp -s "$temp/$name.out" "$root/expected/$name.out" \
		&& [ ! -s "$temp/$name.err" ]; then
		echo
		echo "✅ RESULT: PASS"
		pass=$((pass + 1))
	else
		echo
		echo "❌ RESULT: FAIL"
		echo
		echo "Expected stdout:"
		showfile "$root/expected/$name.out"
		echo
		echo "stdout diff:"
		diff -u "$root/expected/$name.out" "$temp/$name.out" || true
		fail=$((fail + 1))
	fi
}

runpolicy()
{
	local file=$1
	local name
	local args

	name=$(basename "$file" .in)
	args=$(cat "$root/policy/$name.args")

	timeout 3 "$bin" $args \
		< "$file" \
		> "$temp/$name.out" \
		2> "$temp/$name.err"

	echo
	echo "========================================"
	echo "🟡 POLICY CHECK: $name"
	echo "========================================"
	echo "Args: $args"
	echo
	echo "Input:"
	showfile "$file"
	echo
	echo "Actual stdout:"
	showfile "$temp/$name.out"
	echo
	echo "Expected if pen may leave board:"
	showfile "$root/policy/$name.free.out"
	echo
	echo "Expected if movement is clamped:"
	showfile "$root/policy/$name.clamp.out"

	if cmp -s "$temp/$name.out" "$root/policy/$name.free.out"; then
		echo
		echo "🟡 POLICY RESULT: matches free-moving pen"
	elif cmp -s "$temp/$name.out" "$root/policy/$name.clamp.out"; then
		echo
		echo "🟡 POLICY RESULT: matches clamped pen"
	else
		echo
		echo "❌ POLICY RESULT: matches neither policy"
		fail=$((fail + 1))
	fi
}

echo "========================================"
echo "              LIFE TESTS"
echo "========================================"

for file in "$root"/input/*.in
do
	runone "$file"
done

echo
echo "========================================"
echo "        BORDER POLICY CHECKS"
echo "========================================"

for file in "$root"/policy/*.in
do
	runpolicy "$file"
done

echo
echo "========================================"
echo "FINAL RESULT"
echo "========================================"
echo "✅ Passed: $pass"
echo "❌ Failed: $fail"
echo "📦 Strict total: $((pass + fail))"
echo "========================================"

if [ "$fail" -ne 0 ]; then
	exit 1
fi
