#!/bin/sh
# Driver Rush01 : valide les solutions avec un verificateur independant
# (car "la premiere solution rencontree" depend du solveur), teste les
# erreurs exactes "Error" et les tailles bonus jusqu'a 9x9.
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
T="${TMPDIR:-/tmp}"
PASS=0
FAIL=0

echo "== Rush01 =="
if gcc -Wall -Wextra -Werror -o "$T/r01" "$ROOT/rush01/ex00/"*.c \
	2>"$T/r01_cc.log"; then
	:
else
	echo "  ERREUR DE COMPILATION :"
	cat "$T/r01_cc.log"
	exit 1
fi

# validateur : verifie que la sortie resout bien la grille donnee
validate()
{
	clues="$1"; out="$2"
	python3 - "$clues" "$out" << 'PYEOF'
import sys
clues_s, out_s = sys.argv[1], sys.argv[2]
nums = clues_s.split()
n = len(nums) // 4
cl = [int(x) for x in nums]
rows = [l.split() for l in out_s.strip().split("\n") if l.strip()]
ok = len(rows) == n
if ok:
    for r in rows:
        if len(r) != n or any(not (1 <= int(x) <= n) for x in r):
            ok = False
if ok:
    g = [[int(x) for x in r] for r in rows]
    for i in range(n):
        if sorted(g[i]) != list(range(1, n + 1)):
            ok = False
        if sorted(g[j][i] for j in range(n)) != list(range(1, n + 1)):
            ok = False
def vis(line):
    m = v = 0
    for x in line:
        if x > m:
            m = x
            v += 1
    return v
if ok:
    for i in range(n):
        if vis(g[i]) != cl[2 * n + i] or vis(g[i][::-1]) != cl[3 * n + i]:
            ok = False
        col = [g[j][i] for j in range(n)]
        if vis(col) != cl[i] or vis(col[::-1]) != cl[n + i]:
            ok = False
sys.exit(0 if ok else 1)
PYEOF
}

# cas valide : sortie non "Error" + solution correcte
vcase()
{
	name="$1"; clues="$2"
	out="$("$T/r01" "$clues")"
	if [ "$out" != "Error" ] && validate "$clues" "$out"; then
		PASS=$((PASS + 1))
	else
		FAIL=$((FAIL + 1))
		echo "  FAIL $name"
		echo "$out" | head -4
	fi
}

# cas erreur : sortie exacte "Error"
ecase()
{
	name="$1"; shift
	out="$("$T/r01" "$@" 2>/dev/null)"
	if [ "$out" = "Error" ]; then
		PASS=$((PASS + 1))
	else
		FAIL=$((FAIL + 1))
		echo "  FAIL $name (obtenu: $out)"
	fi
}

# --- obligatoire 4x4 : l'exemple du sujet + d'autres jeux ---
vcase "exemple du sujet" "4 3 2 1 1 2 2 2 4 3 2 1 1 2 2 2"
vcase "grille A" "2 3 1 2 2 1 3 2 2 1 4 2 2 4 1 2"
vcase "grille B" "2 1 3 2 1 2 2 3 2 4 2 1 2 1 2 4"
vcase "grille deux" "3 3 1 2 1 2 3 3 2 4 2 1 2 1 2 4"

# --- erreurs exactes ---
ecase "nombre d'arguments" 
ecase "arguments en trop" "1 2 3 4" "5 6 7 8"
ecase "chaine vide" ""
ecase "15 nombres" "4 3 2 1 1 2 2 2 4 3 2 1 1 2 2"
ecase "5 nombres en trop" "4 3 2 1 1 2 2 2 4 3 2 1 1 2 2 2 3 4 5 6"
ecase "chiffre trop grand" "4 3 2 5 1 2 2 2 4 3 2 1 1 2 2 2"
ecase "chiffre zero" "4 3 2 0 1 2 2 2 4 3 2 1 1 2 2 2"
ecase "lettres" "4 3 2 a 1 2 2 2 4 3 2 1 1 2 2 2"
ecase "double espace" "4 3 2 1  2 2 2 4 3 2 1 1 2 2 2 1"
ecase "tabulation" "4 3 2 1	1 2 2 2 4 3 2 1 1 2 2 2"
ecase "tiret" "-4 3 2 1 1 2 2 2 4 3 2 1 1 2 2 2"
ecase "insoluble" "4 4 4 4 1 1 1 1 1 1 1 1 1 1 1 1"
ecase "trop peu (8 nombres)" "1 2 1 2 1 2 1 2"

# --- bonus : autres tailles jusqu'a 9x9 ---
vcase "bonus 2x2" "2 1 1 2 2 1 1 2"
vcase "bonus 1x1" "1 1 1 1"
vcase "bonus 5x5 (cyclique)" "5 4 3 2 1 1 2 2 2 2 5 4 3 2 1 1 2 2 2 2"
vcase "bonus 6x6 (cyclique)" \
	"6 5 4 3 2 1 1 2 2 2 2 2 6 5 4 3 2 1 1 2 2 2 2 2"
vcase "bonus 9x9 (cyclique)" \
	"9 8 7 6 5 4 3 2 1 1 2 2 2 2 2 2 2 2 9 8 7 6 5 4 3 2 1 1 2 2 2 2 2 2 2 2"
vcase "bonus 9x9 (dense)" \
	"5 5 4 4 3 3 2 2 1 2 1 3 2 3 2 3 2 3 9 7 5 3 1 8 6 4 2 1 2 2 2 2 2 2 2 2"
ecase "bonus 10x10 refuse (40 nombres)" \
	"1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1"

# --- performance : resolution rapide ---
t0=$(date +%s)
"$T/r01" "4 3 2 1 1 2 2 2 4 3 2 1 1 2 2 2" > /dev/null
t1=$(date +%s)
if [ $((t1 - t0)) -le 2 ]; then
	PASS=$((PASS + 1))
else
	FAIL=$((FAIL + 1))
	echo "  FAIL temps de resolution"
fi

echo "Rush01 : $PASS reussi(s), $FAIL echec(s)"
[ "$FAIL" -eq 0 ]
