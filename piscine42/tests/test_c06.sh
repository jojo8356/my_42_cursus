#!/bin/sh
# Driver C06 : compile chaque programme, l'execute et compare les sorties.
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
T="${TMPDIR:-/tmp}"
PASS=0
FAIL=0

ok() { PASS=$((PASS + 1)); printf '  \033[32mPASS\033[0m %s\n' "$1"; }
ko() { FAIL=$((FAIL + 1)); printf '  \033[31mFAIL\033[0m %s\n' "$1"; }

echo "== C06 =="

gcc -Wall -Wextra -Werror -o "$T/c06_ex00" "$ROOT/c06/ex00/ft_print_program_name.c" \
	|| { ko "ex00 compilation"; echo "C06 : $PASS reussi(s), $FAIL echec(s)"; exit 1; }
GOT="$("$T/c06_ex00" arg1 arg2)"
[ "$GOT" = "$T/c06_ex00" ] && ok "ex00 affiche argv[0]" || ko "ex00 affiche argv[0]"

gcc -Wall -Wextra -Werror -o "$T/c06_ex01" "$ROOT/c06/ex01/ft_print_params.c" || ko "ex01 compilation"
GOT="$("$T/c06_ex01" salut les gens)"
if [ "$GOT" = "salut
les
gens" ]; then
	ok "ex01 parametres dans l'ordre"
else
	ko "ex01 parametres dans l'ordre"
fi
GOT="$("$T/c06_ex01")"
[ -z "$GOT" ] && ok "ex01 sans parametre -> rien" || ko "ex01 sans parametre -> rien"

gcc -Wall -Wextra -Werror -o "$T/c06_ex02" "$ROOT/c06/ex02/ft_rev_params.c" || ko "ex02 compilation"
GOT="$("$T/c06_ex02" salut les gens)"
if [ "$GOT" = "gens
les
salut" ]; then
	ok "ex02 parametres inverses"
else
	ko "ex02 parametres inverses"
fi

gcc -Wall -Wextra -Werror -o "$T/c06_ex03" "$ROOT/c06/ex03/ft_sort_params.c" || ko "ex03 compilation"
GOT="$("$T/c06_ex03" bb aa cc Aa)"
if [ "$GOT" = "Aa
aa
bb
cc" ]; then
	ok "ex03 parametres tries (ASCII)"
else
	ko "ex03 parametres tries (ASCII)"
fi
GOT="$("$T/c06_ex03" one)"
[ "$GOT" = "one" ] && ok "ex03 un seul parametre" || ko "ex03 un seul parametre"
GOT="$("$T/c06_ex03" "" "a")"
if [ "$GOT" = "
a" ]; then
	ok "ex03 chaine vide en premier"
else
	ko "ex03 chaine vide en premier"
fi

rm -f "$T/c06_ex00" "$T/c06_ex01" "$T/c06_ex02" "$T/c06_ex03"
echo "C06 : $PASS reussi(s), $FAIL echec(s)"
[ "$FAIL" -eq 0 ]
