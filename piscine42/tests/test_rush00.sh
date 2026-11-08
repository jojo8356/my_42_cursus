#!/bin/sh
# Driver Rush00 : les 5 variantes + le binaire unique, compares a un
# generateur d'attendus (les 5 exemples du sujet + cas limites).
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
T="${TMPDIR:-/tmp}"
PASS=0
FAIL=0

# expect <TL> <TR> <BL> <BR> <T> <M> <x> <y>  -> rectangle attendu
expect()
{
	tl="$1"; tr="$2"; bl="$3"; br="$4"; t="$5"; m="$6"; x="$7"; y="$8"
	{ [ "$x" -gt 0 ] && [ "$y" -gt 0 ]; } || return
	r=1
	while [ "$r" -le "$y" ]; do
		c=1
		line=""
		while [ "$c" -le "$x" ]; do
			if [ "$r" -eq 1 ] && [ "$c" -eq 1 ]; then ch="$tl"
			elif [ "$r" -eq 1 ] && [ "$c" -eq "$x" ]; then ch="$tr"
			elif [ "$r" -eq "$y" ] && [ "$c" -eq 1 ]; then ch="$bl"
			elif [ "$r" -eq "$y" ] && [ "$c" -eq "$x" ]; then ch="$br"
			elif [ "$r" -eq 1 ] || [ "$r" -eq "$y" ]; then ch="$t"
			elif [ "$c" -eq 1 ] || [ "$c" -eq "$x" ]; then ch="$m"
			else ch=" "
			fi
			line="$line$ch"
			c=$((c + 1))
		done
		printf '%s\n' "$line"
		r=$((r + 1))
	done
}

# compare <nom> <binaire> <args-binaires...> puis 6 chars + x + y
compare()
{
	name="$1"; bin="$2"; shift 2
	nargs="$1"; shift
	binargs=""
	i=0
	while [ "$i" -lt "$nargs" ]; do
		binargs="$binargs '$1'"; shift
		i=$((i + 1))
	done
	tl="$1"; tr="$2"; bl="$3"; br="$4"; t="$5"; m="$6"; x="$7"; y="$8"
	expect "$tl" "$tr" "$bl" "$br" "$t" "$m" "$x" "$y" > "$T/r00_exp"
	eval "\"$bin\" $binargs '$x' '$y'" > "$T/r00_got" 2>/dev/null
	if cmp -s "$T/r00_exp" "$T/r00_got"; then
		PASS=$((PASS + 1))
	else
		FAIL=$((FAIL + 1))
		echo "  FAIL $name ($x,$y)"
	fi
}

echo "== Rush00 =="

check_variant()
{
	dir="$1"; label="$2"; tl="$3"; tr="$4"; bl="$5"; br="$6"; t="$7"; m="$8"
	if gcc -Wall -Wextra -Werror -o "$T/r00_v" "$ROOT/tests/rush00_main.c" \
		"$ROOT/rush00/$dir/ft_putchar.c" "$ROOT/rush00/$dir/"rush*.c \
		2>"$T/r00_cc.log"; then
		:
	else
		FAIL=$((FAIL + 1))
		echo "  FAIL $label compilation"
		cat "$T/r00_cc.log"
		return
	fi
	for size in "5 3" "5 1" "1 1" "1 5" "4 4" "2 2" "3 2" "1 2" "2 1"; do
		set -- $size
		compare "$label" "$T/r00_v" 0 "$tl" "$tr" "$bl" "$br" "$t" "$m" "$1" "$2"
	done
	for size in "0 5" "5 0" "-2 3" "4 -1" "0 0"; do
		set -- $size
		expect "$tl" "$tr" "$bl" "$br" "$t" "$m" "$1" "$2" > "$T/r00_exp"
		"$T/r00_v" "$1" "$2" > "$T/r00_got" 2>/dev/null
		if [ ! -s "$T/r00_exp" ] && [ ! -s "$T/r00_got" ]; then
			PASS=$((PASS + 1))
		else
			FAIL=$((FAIL + 1))
			echo "  FAIL $label cas limite ($1,$2)"
		fi
	done
}

check_variant ex00 "rush00" o o o o - '|'
check_variant rush01 "rush01" / '\' '\' / '*' '*'
check_variant rush02 "rush02" A A C C B B
check_variant rush03 "rush03" A C A C B B
check_variant rush04 "rush04" A C C A B B

# bonus binaire unique : les 5 versions doivent correspondre aux variantes
if gcc -Wall -Wextra -Werror -o "$T/r00_all" "$ROOT/rush00/bonus_binaire/"*.c \
	2>"$T/r00_cc.log"; then
	compare "bonus v0" "$T/r00_all" 1 0 o o o o - '|' 5 3
	compare "bonus v0" "$T/r00_all" 1 0 o o o o - '|' 1 1
	compare "bonus v1" "$T/r00_all" 1 1 / '\' '\' / '*' '*' 5 3
	compare "bonus v1" "$T/r00_all" 1 1 / '\' '\' / '*' '*' 1 5
	compare "bonus v2" "$T/r00_all" 1 2 A A C C B B 4 4
	compare "bonus v3" "$T/r00_all" 1 3 A C A C B B 5 3
	compare "bonus v4" "$T/r00_all" 1 4 A C C A B B 5 3
	if "$T/r00_all" 9 > "$T/r00_out" 2>"$T/r00_err" || [ -s "$T/r00_out" ]; then
		FAIL=$((FAIL + 1)); echo "  FAIL bonus version invalide"
	else
		PASS=$((PASS + 1))
	fi
else
	FAIL=$((FAIL + 1))
	echo "  FAIL bonus binaire compilation"
	cat "$T/r00_cc.log"
fi

echo "Rush00 : $PASS reussi(s), $FAIL echec(s)"
[ "$FAIL" -eq 0 ]
