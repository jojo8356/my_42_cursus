#!/bin/sh
# Driver de tests Shell01 : verifie chaque livrable de l'exercice.
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
PASS=0
FAIL=0

ok() { PASS=$((PASS + 1)); printf '  \033[32mPASS\033[0m %s\n' "$1"; }
ko() { FAIL=$((FAIL + 1)); printf '  \033[31mFAIL\033[0m %s\n' "$1"; }

TMP="$(mktemp -d)"
trap 'rm -rf "$TMP"' EXIT

echo "== Shell01 =="

# --- ex01 : print_groups.sh ---
FT_USER="$(id -un)"
export FT_USER
GOT="$(sh "$ROOT/shell01/ex01/print_groups.sh")"
EXP="$(id -Gn "$FT_USER" | tr ' ' ',')"
if [ "$GOT" = "$EXP" ] && [ -n "$GOT" ] && ! echo "$GOT" | grep -q ' '; then
	ok "ex01 groupes separes par virgules"
else
	ko "ex01 groupes (obtenu: $GOT attendu: $EXP)"
fi
FT_USER="root"
GOT="$(sh "$ROOT/shell01/ex01/print_groups.sh")"
[ "$GOT" = "root" ] && ok "ex01 FT_USER=root -> root" || ko "ex01 FT_USER=root ($GOT)"

# --- ex02 : find_sh.sh ---
mkdir -p "$TMP/e02/sub/deep"
touch "$TMP/e02/a.sh" "$TMP/e02/sub/b.sh" "$TMP/e02/sub/deep/c.sh" \
	"$TMP/e02/d.txt" "$TMP/e02/noext"
GOT="$(cd "$TMP/e02" && sh "$ROOT/shell01/ex02/find_sh.sh" | sort)"
if [ "$GOT" = "a
b
c" ]; then
	ok "ex02 noms sans extension, recursif"
else
	ko "ex02 noms (obtenu: $GOT)"
fi

# --- ex03 : count_files.sh ---
EXP="$(cd "$TMP/e02" && find . \( -type f -o -type d \) | wc -l | tr -d ' ')"
GOT="$(cd "$TMP/e02" && sh "$ROOT/shell01/ex03/count_files.sh" | tr -d ' ')"
[ "$GOT" = "$EXP" ] && ok "ex03 compte fichiers + repertoires" \
	|| ko "ex03 compte (obtenu: $GOT attendu: $EXP)"

# --- ex04 : MAC.sh ---
if command -v ifconfig >/dev/null 2>&1 && [ -n "$(ifconfig 2>/dev/null | grep ether)" ]; then
	GOT="$(sh "$ROOT/shell01/ex04/MAC.sh")"
	BAD="$(echo "$GOT" | grep -v -E '^([0-9a-fA-F]{2}:){5}[0-9a-fA-F]{2}$')"
	if [ -n "$GOT" ] && [ -z "$BAD" ]; then
		ok "ex04 adresses MAC une par ligne"
	else
		ko "ex04 format MAC (lignes: $BAD)"
	fi
else
	echo "  \033[33mSKIP\033[0m ex04 (ifconfig indisponible ici)"
fi

# --- ex05 : fichier exotique ---
F="$(find "$ROOT/shell01/ex05" -type f | head -1)"
NAME="$(basename "$F")"
WANT="$(printf '\042\134\077\044\052\047MaRViN\047\052\044\077\134\042')"
if [ "$NAME" = "$WANT" ]; then
	ok "ex05 nom de fichier exotique exact"
else
	ko "ex05 nom (obtenu: $NAME)"
fi
[ "$(cat "$F")" = "42" ] && [ "$(wc -c < "$F" | tr -d ' ')" = "2" ] \
	&& ok "ex05 contenu exact 42" || ko "ex05 contenu exact 42"

# --- ex06 : skip.sh ---
mkdir -p "$TMP/e06" && cd "$TMP/e06" || exit 1
touch n1 n2 n3 n4 n5
GOT="$(sh "$ROOT/shell01/ex06/skip.sh")"
echo "$GOT" | sed -n 2p | grep -q 'n2$' && ok "ex06 garde la ligne 3 (n2)" \
	|| ko "ex06 garde la ligne 3 (n2)"
echo "$GOT" | sed -n 3p | grep -q 'n4$' && ok "ex06 garde la ligne 5 (n4)" \
	|| ko "ex06 garde la ligne 5 (n4)"
[ "$(echo "$GOT" | wc -l | tr -d ' ')" = "3" ] && ok "ex06 exactement 3 lignes" \
	|| ko "ex06 exactement 3 lignes (obtenu: $GOT)"
cd "$ROOT" || exit 1

# --- ex07 : r_dwssap.sh ---
export FT_LINE1=1 FT_LINE2=3
EXP="$(python3 - <<'PY'
lines = [l for l in open('/etc/passwd') if not l.startswith('#')]
logins = [l.split(':')[0] for l in lines[1::2]]
rev = sorted((l[::-1] for l in logins), reverse=True)[0:3]
print(', '.join(rev) + '.')
PY
)"
GOT="$(sh "$ROOT/shell01/ex07/r_dwssap.sh")"
if [ "$GOT" = "$EXP" ] && [ "${GOT%.}" != "$GOT" ]; then
	ok "ex07 pipeline complet (FT_LINE1=1 FT_LINE2=3)"
else
	ko "ex07 pipeline (obtenu: $GOT attendu: $EXP)"
fi

# --- ex08 : add_chelou.sh ---
export FT_NBR1 FT_NBR2
# Exemple 1 du sujet : FT_NBR1='\?\"'"\ et FT_NBR2=rcrdmddd -> "Salut"
FT_NBR1="$(printf '\047\134\077\042\047\042\134\047')"
FT_NBR2="rcrdmddd"
GOT="$(sh "$ROOT/shell01/ex08/add_chelou.sh")"
if [ "$GOT" = "Salut" ]; then
	ok "ex08 exemple 1 -> Salut"
else
	ko "ex08 exemple 1 (obtenu: $GOT attendu: Salut)"
fi
# petits nombres : "!" (4) + "m" (0) = 4 -> "o" en base gtaio
FT_NBR1="!"
FT_NBR2="m"
GOT="$(sh "$ROOT/shell01/ex08/add_chelou.sh")"
[ "$GOT" = "o" ] && ok "ex08 petits nombres (4 + 0 -> o)" \
	|| ko "ex08 petits nombres (obtenu: $GOT attendu: o)"
# "!!!" (124) + "mmm" (0) = 124 -> "nu"
FT_NBR1="!!!"
FT_NBR2="mmm"
GOT="$(sh "$ROOT/shell01/ex08/add_chelou.sh")"
[ "$GOT" = "nu" ] && ok "ex08 124 -> nu" || ko "ex08 124 -> nu (obtenu: $GOT)"
# grand nombre : ne doit pas crasher et doit produire une sortie non vide
FT_NBR1="\\"
FT_NBR2="rrrrrrrrrrrrrrrrrrrrrrrrrrrrr"
GOT="$(sh "$ROOT/shell01/ex08/add_chelou.sh")"
[ -n "$GOT" ] && ok "ex08 grands nombres sans crash" \
	|| ko "ex08 grands nombres sans crash"

echo "Shell01 : $PASS reussi(s), $FAIL echec(s)"
[ "$FAIL" -eq 0 ]
