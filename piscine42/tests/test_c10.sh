#!/bin/sh
# Driver C10 : build les 4 programmes avec leur Makefile et compare
# les sorties aux outils systeme (cat, tail) + controle de format.
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
T="${TMPDIR:-/tmp}"
PASS=0
FAIL=0

ok() { PASS=$((PASS + 1)); printf '  \033[32mPASS\033[0m %s\n' "$1"; }
ko() { FAIL=$((FAIL + 1)); printf '  \033[31mFAIL\033[0m %s\n' "$1"; }

echo "== C10 =="
mkdir -p "$T/c10" && cd "$T/c10" || exit 1
printf 'Hello, world!\n' > a.txt
printf 'ABCDEFGHIJ' > b.txt
printf '' > empty.txt
dd if=/dev/urandom of=big.bin bs=1024 count=33 2>/dev/null

for ex in ex00 ex01 ex02 ex03; do
	cp -r "$ROOT/c10/$ex" "$T/c10/$ex"
	(cd "$T/c10/$ex" && make >/dev/null 2>&1) || ko "$ex make"
done

# --- ex00 : ft_display_file ---
B="$T/c10/ex00/ft_display_file"
[ "$("$B" a.txt)" = "$(cat a.txt)" ] && ok "ex00 affiche le contenu" \
	|| ko "ex00 affiche le contenu"
[ "$("$B" empty.txt)" = "" ] && ok "ex00 fichier vide" || ko "ex00 fichier vide"
GOT="$("$B" 2>&1)"
[ "$GOT" = "File name missing." ] && ok "ex00 sans argument -> erreur" \
	|| ko "ex00 sans argument -> erreur"
GOT="$("$B" a.txt b.txt 2>&1)"
[ "$GOT" = "Too many arguments." ] && ok "ex00 deux arguments -> erreur" \
	|| ko "ex00 deux arguments -> erreur"
GOT="$("$B" nofile 2>&1)"
[ "$GOT" = "Cannot read file." ] && ok "ex00 fichier illisible -> erreur" \
	|| ko "ex00 fichier illisible -> erreur"
"$B" nofile >/dev/null 2>&1; [ $? -eq 1 ] && ok "ex00 code sortie 1" \
	|| ko "ex00 code sortie 1"

# --- ex01 : ft_cat ---
B="$T/c10/ex01/ft_cat"
[ "$("$B" a.txt)" = "$(cat a.txt)" ] && ok "ex01 cat un fichier" \
	|| ko "ex01 cat un fichier"
[ "$("$B" a.txt b.txt)" = "$(cat a.txt b.txt)" ] && ok "ex01 cat deux fichiers" \
	|| ko "ex01 cat deux fichiers"
[ "$("$B" empty.txt a.txt)" = "$(cat empty.txt a.txt)" ] && ok "ex01 cat vide+non vide" \
	|| ko "ex01 cat vide+non vide"
"$B" < big.bin | cmp -s - big.bin \
	&& ok "ex01 cat stdin gros fichier" || ko "ex01 cat stdin gros fichier"
GOT="$("$B" nofile 2>&1 >/dev/null)"
[ "$GOT" = "ft_cat: nofile: No such file or directory" ] \
	&& ok "ex01 format erreur" || ko "ex01 format erreur (obtenu: $GOT)"
"$B" nofile a.txt > "$T/c10/out1" 2>/dev/null
[ "$(cat "$T/c10/out1")" = "$(cat a.txt)" ] && ok "ex01 continue apres erreur" \
	|| ko "ex01 continue apres erreur"
"$B" nofile >/dev/null 2>&1; [ $? -eq 1 ] && ok "ex01 code sortie 1" \
	|| ko "ex01 code sortie 1"

# --- ex02 : ft_tail ---
B="$T/c10/ex02/ft_tail"
[ "$("$B" -c 5 a.txt)" = "$(tail -c 5 a.txt)" ] && ok "ex02 tail -c 5" \
	|| ko "ex02 tail -c 5"
[ "$("$B" -c5 a.txt)" = "$(tail -c 5 a.txt)" ] && ok "ex02 -c5 colle" \
	|| ko "ex02 -c5 colle"
[ "$("$B" -c 100 b.txt)" = "$(tail -c 100 b.txt)" ] && ok "ex02 -c > taille" \
	|| ko "ex02 -c > taille"
[ "$("$B" -c 0 a.txt)" = "" ] && ok "ex02 -c 0" || ko "ex02 -c 0"
GOT="$("$B" -c 5 a.txt b.txt)"
EXP="$(tail -c 5 a.txt b.txt)"
[ "$GOT" = "$EXP" ] && ok "ex02 multichemin (en-tetes)" || ko "ex02 multichemin (en-tetes)"
GOT="$("$B" -c 5 < big.bin)"
EXP="$(tail -c 5 < big.bin)"
[ "$GOT" = "$EXP" ] && ok "ex02 stdin" || ko "ex02 stdin"
GOT="$("$B" -c 5000 < big.bin)"
EXP="$(tail -c 5000 < big.bin)"
[ "$GOT" = "$EXP" ] && ok "ex02 stdin gros tampon (5000)" || ko "ex02 stdin gros tampon"
GOT="$("$B" -c 5 nofile a.txt 2>/dev/null)"
[ "$GOT" = "$(tail -c 5 nofile a.txt 2>/dev/null)" ] \
	&& ok "ex02 multichemin avec erreur" || ko "ex02 multichemin avec erreur"
GOT="$("$B" -c 5 nofile 2>&1 >/dev/null)"
[ "$GOT" = "ft_tail: nofile: No such file or directory" ] \
	&& ok "ex02 format erreur" || ko "ex02 format erreur (obtenu: $GOT)"

# --- ex03 : ft_hexdump (controle de format, pas d'hexdump systeme) ---
B="$T/c10/ex03/ft_hexdump"
cat a.txt b.txt > ab.txt
"$B" ab.txt > "$T/c10/hd1" 2>/dev/null
python3 - "$T/c10/hd1" ab.txt > "$T/c10/hd1res" << 'PY'
import sys

def cline(data, off):
	s = '%08x  ' % off
	for i in range(16):
		s += '%02x ' % data[i] if i < len(data) else '   '
		if i == 7:
			s += ' '
	s += ' |'
	for b in data:
		s += chr(b) if 32 <= b <= 126 else '.'
	return s + '|'

data = open(sys.argv[2], 'rb').read()
exp = []
for off in range(0, len(data), 16):
	exp.append(cline(data[off:off + 16], off))
exp.append('%08x' % len(data))
got = open(sys.argv[1]).read().split('\n')
print('OK' if got == exp + [''] else 'BAD')
PY
[ "$(cat "$T/c10/hd1res")" = "OK" ] && ok "ex03 format hexdump -C (fichiers concat.)" \
	|| { ko "ex03 format hexdump -C (fichiers concat.)"; cat "$T/c10/hd1"; }
"$B" -C < big.bin > "$T/c10/hd2" 2>/dev/null
python3 - "$T/c10/hd2" big.bin > "$T/c10/hd2res" << 'PY'
import sys

def cline(data, off):
	s = '%08x  ' % off
	for i in range(16):
		s += '%02x ' % data[i] if i < len(data) else '   '
		if i == 7:
			s += ' '
	s += ' |'
	for b in data:
		s += chr(b) if 32 <= b <= 126 else '.'
	return s + '|'

data = open(sys.argv[2], 'rb').read()
exp = []
for off in range(0, len(data), 16):
	exp.append(cline(data[off:off + 16], off))
exp.append('%08x' % len(data))
got = open(sys.argv[1]).read().split('\n')
print('OK' if got == exp + [''] else 'BAD')
PY
[ "$(cat "$T/c10/hd2res")" = "OK" ] && ok "ex03 gros flux (33 ko, offsets)" \
	|| ko "ex03 gros flux (33 ko, offsets)"
"$B" -C empty.txt | grep -q '^00000000$' && ok "ex03 fichier vide -> 00000000" \
	|| ko "ex03 fichier vide -> 00000000"
GOT="$("$B" -C nofile 2>&1 >/dev/null)"
[ "$GOT" = "ft_hexdump: nofile: No such file or directory" ] \
	&& ok "ex03 format erreur" || ko "ex03 format erreur (obtenu: $GOT)"

cd "$ROOT" || exit 1
rm -rf "$T/c10"
echo "C10 : $PASS reussi(s), $FAIL echec(s)"
[ "$FAIL" -eq 0 ]
