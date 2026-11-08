#!/bin/sh
# Driver de tests Shell00 : verifie chaque livrable de l'exercice.
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
PASS=0
FAIL=0

ok() { PASS=$((PASS + 1)); printf '  \033[32mPASS\033[0m %s\n' "$1"; }
ko() { FAIL=$((FAIL + 1)); printf '  \033[31mFAIL\033[0m %s\n' "$1"; }

TMP="$(mktemp -d)"
trap 'rm -rf "$TMP"' EXIT

echo "== Shell00 =="

# --- ex00 : z ---
if [ "$(cat "$ROOT/shell00/ex00/z" 2>/dev/null)" = "Z" ]; then
	ok "ex00 z affiche Z"
else
	ko "ex00 z affiche Z"
fi

# --- ex01 : testShell00.tar ---
mkdir -p "$TMP/e01" && tar -xf "$ROOT/shell00/ex01/testShell00.tar" -C "$TMP/e01" 2>/dev/null
if [ -f "$TMP/e01/testShell00" ]; then
	ok "ex01 tar contient testShell00"
	SZ="$(wc -c < "$TMP/e01/testShell00" | tr -d ' ')"
	[ "$SZ" = "40" ] && ok "ex01 taille 40 octets" || ko "ex01 taille 40 octets ($SZ)"
	PERM="$(stat -c '%a' "$TMP/e01/testShell00")"
	[ "$PERM" = "455" ] && ok "ex01 permissions -r--r-xr-x" || ko "ex01 permissions ($PERM)"
	MON="$(TZ=Europe/Paris date -r "$TMP/e01/testShell00" '+%b %e')"
	echo "$MON" | grep -q 'Jun *1' && ok "ex01 date Jun 1" || ko "ex01 date Jun 1 ($MON)"
else
	ko "ex01 tar contient testShell00"
fi

# --- ex02 : exo2.tar ---
mkdir -p "$TMP/e02" && tar -xf "$ROOT/shell00/ex02/exo2.tar" -C "$TMP/e02" 2>/dev/null
E02="$TMP/e02"
if [ -d "$E02/test0" ] && [ -d "$E02/test2" ] && [ -L "$E02/test6" ]; then
	ok "ex02 arborescence test0..test6"
else
	ko "ex02 arborescence test0..test6"
fi
[ "$(stat -c '%a' "$E02/test0")" = "715" ] && ok "ex02 test0 drwx--xr-x" || ko "ex02 test0 perms"
[ "$(stat -c '%a' "$E02/test1")" = "714" ] && ok "ex02 test1 -rwx--xr--" || ko "ex02 test1 perms"
[ "$(stat -c '%a' "$E02/test2")" = "504" ] && ok "ex02 test2 dr-x---r--" || ko "ex02 test2 perms"
[ "$(stat -c '%a' "$E02/test3")" = "404" ] && ok "ex02 test3 -r-----r--" || ko "ex02 test3 perms"
[ "$(stat -c '%a' "$E02/test4")" = "641" ] && ok "ex02 test4 -rw-r----x" || ko "ex02 test4 perms"
[ "$(wc -c < "$E02/test1" | tr -d ' ')" = "4" ] && ok "ex02 test1 fait 4 octets" || ko "ex02 test1 taille"
[ "$(wc -c < "$E02/test4" | tr -d ' ')" = "2" ] && ok "ex02 test4 fait 2 octets" || ko "ex02 test4 taille"
[ "$(wc -c < "$E02/test3" | tr -d ' ')" = "1" ] && ok "ex02 test3 fait 1 octet" || ko "ex02 test3 taille"
if [ "$E02/test3" -ef "$E02/test5" ]; then
	ok "ex02 test5 est un lien dur vers test3"
else
	ko "ex02 test5 est un lien dur vers test3"
fi
[ "$(readlink "$E02/test6")" = "test0" ] && ok "ex02 test6 -> test0" || ko "ex02 test6 -> test0"

# --- ex03 : id_ed25519_pub ---
if head -c 12 "$ROOT/shell00/ex03/id_ed25519_pub" | grep -q '^ssh-ed25519 '; then
	ok "ex03 cle publique ed25519"
else
	ko "ex03 cle publiche ed25519"
fi

# --- ex04 : midLS ---
mkdir -p "$TMP/e04/d1" && cd "$TMP/e04" || exit 1
touch -t 202601011000 f1 && touch -t 202601011100 d1 && touch -t 202601011200 f2
touch -t 202601011300 .cache
OUT="$(sh "$ROOT/shell00/ex04/midLS")"
if [ "$OUT" = "f2, d1/, f1" ]; then
	ok "ex04 midLS trie par date avec slash et virgules"
else
	ko "ex04 midLS (obtenu: $OUT)"
fi
cd "$ROOT" || exit 1

# --- ex05 : git_commit.sh ---
mkdir -p "$TMP/e05" && cd "$TMP/e05" && git init -q . || exit 1
git config user.email t@t && git config user.name t
I=1
while [ $I -le 6 ]; do
	echo "$I" >> f && git add f
	GIT_AUTHOR_DATE="2026-01-0${I}T12:00:00" GIT_COMMITTER_DATE="2026-01-0${I}T12:00:00" \
		git commit -qm "c$I"
	I=$((I + 1))
done
EXP="$(git log --format=%H -5)"
GOT="$(sh "$ROOT/shell00/ex05/git_commit.sh")"
[ "$GOT" = "$EXP" ] && ok "ex05 5 derniers commits" || ko "ex05 5 derniers commits"
cd "$ROOT" || exit 1

# --- ex06 : git_ignore.sh ---
mkdir -p "$TMP/e06" && cd "$TMP/e06" && git init -q . || exit 1
echo "ig.txt" > .gitignore && touch ig.txt real.txt
GOT="$(sh "$ROOT/shell00/ex06/git_ignore.sh")"
if echo "$GOT" | grep -q '^ig.txt$' && ! echo "$GOT" | grep -q 'real.txt'; then
	ok "ex06 liste les ignores seulement"
else
	ko "ex06 liste les ignores seulement (obtenu: $GOT)"
fi
cd "$ROOT" || exit 1

# --- ex07 : b ---
if [ -s "$ROOT/shell00/ex07/b" ] && [ "$(wc -l < "$ROOT/shell00/ex07/b")" -ge 2 ]; then
	ok "ex07 b existe et contient plusieurs lignes"
else
	ko "ex07 b existe et contient plusieurs lignes"
fi
printf 'alpha\nbeta\ngamma\n' > "$TMP/a07"
printf 'alpha\nBETA\ngamma\n' > "$TMP/b07"
diff "$TMP/a07" "$TMP/b07" > "$TMP/sw.diff"
if patch -s "$TMP/a07" < "$TMP/sw.diff" && grep -q 'BETA' "$TMP/a07"; then
	ok "ex07 cycle diff/patch fonctionnel"
else
	ko "ex07 cycle diff/patch fonctionnel"
fi

# --- ex08 : clean ---
mkdir -p "$TMP/e08/sub" && cd "$TMP/e08" || exit 1
touch 'foo~' '#bar#' '#' 'keep.txt' 'sub/x~'
OUT="$(sh "$ROOT/shell00/ex08/clean")"
if [ ! -e 'foo~' ] && [ ! -e '#bar#' ] && [ ! -e '#' ] && [ ! -e 'sub/x~' ] \
	&& [ -f 'keep.txt' ]; then
	ok "ex08 supprime ~ et #...# (garde le reste)"
else
	ko "ex08 supprime ~ et #...# (garde le reste)"
fi
echo "$OUT" | grep -q 'foo~' && ok "ex08 affiche les fichiers supprimes" \
	|| ko "ex08 affiche les fichiers supprimes"
cd "$ROOT" || exit 1

# --- ex09 : ft_magic ---
MAGIC="$ROOT/shell00/ex09/ft_magic"
if command -v file >/dev/null 2>&1; then
	{ printf '%041d' 0; printf '42XXXX'; } > "$TMP/on"
	{ printf '%040d' 0; printf '42XXXX'; } > "$TMP/off1"
	{ printf '%042d' 0; printf '42XXXX'; } > "$TMP/off2"
	R_ON="$(file -m "$MAGIC" -b "$TMP/on")"
	R1="$(file -m "$MAGIC" -b "$TMP/off1")"
	R2="$(file -m "$MAGIC" -b "$TMP/off2")"
	if [ "$R_ON" = "42 file" ]; then
		ok "ex09 detecte 42 au 42e octet"
	else
		ko "ex09 detecte 42 au 42e octet (obtenu: $R_ON)"
	fi
	if [ "$R_ON" != "$R1" ] && [ "$R_ON" != "$R2" ]; then
		ok "ex09 pas de detection aux mauvais offsets"
	else
		ko "ex09 pas de detection aux mauvais offsets"
	fi
else
	grep -qE '^41[[:space:]]+string[[:space:]]+42[[:space:]]+42 file$' "$MAGIC" \
		&& ok "ex09 regle ft_magic conforme (file non installe)" \
		|| ko "ex09 regle ft_magic invalide"
fi

echo "Shell00 : $PASS reussi(s), $FAIL echec(s)"
[ "$FAIL" -eq 0 ]
