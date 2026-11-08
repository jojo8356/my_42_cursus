#!/bin/sh
# Driver Rush02 : conversions, erreurs, formats de dico, bonus (-w, FR, stdin).
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
D="$ROOT/rush02/ex00"
T="${TMPDIR:-/tmp}"
PASS=0
FAIL=0

echo "== Rush02 =="
if (cd "$D" && make fclean >/dev/null && make >/dev/null 2>"$T/r02_cc.log"); then
	:
else
	echo "  ERREUR MAKE :"
	cat "$T/r02_cc.log"
	exit 1
fi

tcase()
{
	name="$1"; want="$2"; shift 2
	got="$(cd "$D" && ./rush-02 "$@" 2>&1)"
	if [ "$got" = "$want" ]; then
		PASS=$((PASS + 1))
	else
		FAIL=$((FAIL + 1))
		echo "  FAIL $name (obtenu: $got)"
	fi
}

# --- obligatoire ---
tcase "42" "forty two" 42
tcase "0" "zero" 0
tcase "20" "twenty" 20
tcase "17" "seventeen" 17
tcase "101" "one hundred one" 101
tcase "1000" "one thousand" 1000
tcase "100000" "one hundred thousand" 100000
tcase "2 milliards" "two billion" 2000000000
tcase "au-dela uint" "four billion two hundred ninety four million nine hundred sixty seven thousand two hundred ninety six" 4294967296
tcase "quintillion" "one quintillion" 1000000000000000000
tcase "sextillion (10^21)" "one sextillion" 1000000000000000000000
tcase "undecillion (10^36)" "one undecillion" 1000000000000000000000000000000000000
tcase "42 undecillion + 7" "forty two undecillion seven" 42000000000000000000000000000000000007
tcase "10.4 -> Error" "Error" 10.4
tcase "negatif -> Error" "Error" -3
tcase "lettre -> Error" "Error" abc
tcase "vide -> Error" "Error" ""
tcase "sans argument -> Error" "Error"
tcase "trop d'arguments -> Error" "Error" 1 2 3

# --- dictionnaire : valeurs modifiees / erreurs ---
printf '100:   cent   \n\n0: zero\n20 : hey everybody ! \n1: one\n' > "$T/tdict"
tcase "dico modifie 20" "hey everybody !" "$T/tdict" 20
tcase "dico modifie + espaces" "one cent" "$T/tdict" 100
tcase "dico inexistant -> Dict Error" "Dict Error" "$T/none.dict" 5
printf 'pas de deux points ici\n' > "$T/bad.dict"
tcase "dico malforme -> Dict Error" "Dict Error" "$T/bad.dict" 5
printf '0: zero\n1: one\n' > "$T/small.dict"
tcase "cle manquante -> Dict Error" "Dict Error" "$T/small.dict" 42

# --- bonus -w : style ecrit ---
tcase "-w 42" "forty-two" 42 -w
tcase "-w 101" "one hundred and one" 101 -w
tcase "-w 2345" "two thousand, three hundred and forty-five" 2345 -w
tcase "-w conserve valeur perso" "hey everybody !" "$T/tdict" 20 -w

# --- bonus : autre langue (fr) ---
tcase "fr 42" "quarante-deux" numbers_fr.dict 42
tcase "fr 71" "soixante et onze" numbers_fr.dict 71
tcase "fr 80" "quatre-vingts" numbers_fr.dict 80
tcase "fr 81" "quatre-vingt-un" numbers_fr.dict 81
tcase "fr 200" "deux cents" numbers_fr.dict 200
tcase "fr 201" "deux cent un" numbers_fr.dict 201
tcase "fr 1000" "mille" numbers_fr.dict 1000
tcase "fr 2024" "deux mille vingt-quatre" numbers_fr.dict 2024
tcase "fr 1M" "un million" numbers_fr.dict 1000000
tcase "fr 2M" "deux millions" numbers_fr.dict 2000000

# --- bonus : lecture stdin avec "-" et sans argument ---
got="$(cd "$D" && printf '42\n0\nbad\n999\n' | ./rush-02 -)"
want="forty two
zero
Error
nine hundred ninety nine"
if [ "$got" = "$want" ]; then
	PASS=$((PASS + 1))
else
	FAIL=$((FAIL + 1))
	echo "  FAIL stdin - (obtenu: $got)"
fi

got2="$(cd "$D" && printf '42\n1000000000000000000000000000000000000\n' | ./rush-02)"
want2="forty two
one undecillion"
if [ "$got2" = "$want2" ]; then
	PASS=$((PASS + 1))
else
	FAIL=$((FAIL + 1))
	echo "  FAIL stdin sans arg (obtenu: $got2)"
fi

# --- fuites memoire ---
if command -v valgrind >/dev/null 2>&1; then
	(cd "$D" && valgrind --error-exitcode=42 --leak-check=full ./rush-02 42 2>"$T/r02_vg.log" >/dev/null) \
		|| { FAIL=$((FAIL + 1)); echo "  FAIL valgrind"; tail -5 "$T/r02_vg.log"; }
	PASS=$((PASS + 1))
fi

echo "Rush02 : $PASS reussi(s), $FAIL echec(s)"
[ "$FAIL" -eq 0 ]
