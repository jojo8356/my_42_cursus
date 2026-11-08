#!/bin/sh
# Driver BSQ : cas fixes, cartes aleatoires vs reference Python,
# erreurs "map error", multi-fichiers, Makefile sans relink, perf.
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
B="$ROOT/bsq"
T="${TMPDIR:-/tmp}"
PASS=0
FAIL=0

echo "== BSQ =="
if (cd "$B" && make fclean >/dev/null && make >/dev/null 2>"$T/bsq_cc.log"); then
	:
else
	echo "  ERREUR MAKE :"
	cat "$T/bsq_cc.log"
	exit 1
fi

ok()
{
	PASS=$((PASS + 1))
}

ko()
{
	FAIL=$((FAIL + 1))
	echo "  FAIL $1"
}

# cas fige : map dans stdin, sortie attendue dans $2
scase()
{
	name="$1"; map="$2"; want="$3"
	got="$(printf '%s' "$map" | "$B/bsq")"
	if [ "$got" = "$want" ]; then ok; else ko "$name"; echo "$got" | head -3; fi
}

scase "3x3 vide" '3o.x
ooo
ooo
ooo
' 'xxx
xxx
xxx'
scase "exaequo haut/gauche" '4.ox
o..o
....
..o.
....
' 'oxxo
.xx.
..o.
....'
scase "1x1 plein" '1.ox
.
' 'x'
scase "1x1 obstacle" '1.ox
o
' 'o'
scase "que des obstacles" '3.ox
ooo
ooo
ooo
' 'ooo
ooo
ooo'
scase "colonne 5x1" '5.ox
.
o
.
.
.
' 'x
o
.
.
.'
scase "ligne 1x5" '1.ox
..o..
' 'x.o..'
scase "carre en bas (gauche)" '5.ox
..o..
..o..
.....
.....
.....
' '..o..
..o..
xxx..
xxx..
xxx..'
scase "caracteres personnalises" '3+#@
+++
+++
+#+
' '@@+
@@+
+#+'
scase "grande zone libre" '10.ox
..........
..........
..........
..........
..........
..........
..........
..........
..........
..........
' 'xxxxxxxxxx
xxxxxxxxxx
xxxxxxxxxx
xxxxxxxxxx
xxxxxxxxxx
xxxxxxxxxx
xxxxxxxxxx
xxxxxxxxxx
xxxxxxxxxx
xxxxxxxxxx'

# erreurs : "map error"
ecase()
{
	name="$1"; map="$2"
	got="$(printf '%s' "$map" | "$B/bsq")"
	if [ "$got" = "map error" ]; then ok; else ko "$name (obtenu: $got)"; fi
}

ecase "fichier vide" ''
ecase "pas de saut de ligne" '3o.xooo'
ecase "entete trop courte" '3ox
oo
'
ecase "compte non numerique" 'ax.ox
oo
'
ecase "compte zero" '0.ox
'
ecase "caractes dupliques" '3o.o
ooo
ooo
ooo
'
ecase "caractes dupliques 2" '3ooo
ooo
'
ecase "lignes irregulieres" '3.ox
oo
ooo
ooo
'
ecase "trop peu de lignes" '3.ox
oo
oo
'
ecase "trop de lignes" '2.ox
oo
oo
oo
'
ecase "caracte inconnu" '3.ox
oza
ooo
ooo
'
ecase "ligne vide dans la carte" '3.ox
oo

oo
'
ecase "entete sans caracteres" '3

oo
'

# multi-fichiers : separateur ligne vide, erreurs melangees
printf '3.ox\n...\n...\n...\n' > "$T/m1"
printf '1.ox\n.\n' > "$T/m2"
printf 'zzz\n' > "$T/bad"
got="$("$B/bsq" "$T/m1" "$T/m2")"
want='xxx
xxx
xxx

x'
if [ "$got" = "$want" ]; then ok; else ko "multi-fichiers"; fi
got="$("$B/bsq" "$T/m1" "$T/bad" "$T/m2")"
want='xxx
xxx
xxx

map error

x'
if [ "$got" = "$want" ]; then ok; else ko "multi-fichiers avec erreur"; fi
got="$("$B/bsq" "$T/inexistant")"
if [ "$got" = "map error" ]; then ok; else ko "fichier inexistant"; fi

# cartes aleatoires comparees a une reference Python independante
python3 - "$B/bsq" << 'PYEOF'
import random, subprocess, sys
bsq = sys.argv[1]

def ref(path):
    with open(path) as f:
        lines = f.read().split("\n")
    if lines and lines[-1] == "":
        lines.pop()
    head = lines[0]
    rows = int(head[:-3])
    e, o, fl = head[-3], head[-2], head[-1]
    grid = [list(l) for l in lines[1:]]
    n, m = rows, len(grid[0])
    best = (0, -1, -1)
    prev = [0] * (m + 1)
    for i in range(n):
        cur = [0] * (m + 1)
        for j in range(m):
            if grid[i][j] == e:
                if j == 0:
                    cur[j] = 1
                else:
                    cur[j] = 1 + min(prev[j], cur[j-1], prev[j-1])
                if cur[j] > best[0]:
                    best = (cur[j], i, j)
        prev = cur
    s, bi, bj = best
    if s > 0:
        for i in range(bi - s + 1, bi + 1):
            for j in range(bj - s + 1, bj + 1):
                grid[i][j] = fl
    return "\n".join("".join(r) for r in grid)

random.seed(42)
fails = 0
for t in range(6):
    n, m = random.randint(1, 25), random.randint(1, 25)
    path = f"/tmp/bsq_rand_{t}"
    with open(path, "w") as f:
        f.write(f"{n}.ox\n")
        for i in range(n):
            f.write("".join(random.choice(".....o") for _ in range(m)) + "\n")
    out = subprocess.run([bsq, path], capture_output=True, text=True).stdout.strip()
    if out != ref(path):
        fails += 1
        print(f"  FAIL aleatoire {t} ({n}x{m})")
sys.exit(1 if fails else 0)
PYEOF
if [ $? -eq 0 ]; then ok; else ko "cartes aleatoires"; fi

# Makefile : pas de recompilation inutile, recompilation si changement
if (cd "$B" && make 2>&1 | grep -q "cc "); then
	ko "Makefile relink inutile"
else
	ok
fi
touch "$B/solve.c"
if (cd "$B" && make 2>&1 | grep -q "cc ") ; then
	ok
else
	ko "Makefile ne recompile pas apres modification"
fi

# performance : 1000x1000 resolu en moins de 3 s
python3 -c "
import random
random.seed(7)
with open('/tmp/bsq_big', 'w') as f:
    f.write('1000.ox\n')
    for _ in range(1000):
        f.write(''.join(random.choice('..........o') for _ in range(1000)) + '\n')
"
t0=$(date +%s)
"$B/bsq" /tmp/bsq_big > /dev/null
t1=$(date +%s)
if [ $((t1 - t0)) -le 3 ]; then ok; else ko "performance 1000x1000"; fi

echo "BSQ : $PASS reussi(s), $FAIL echec(s)"
[ "$FAIL" -eq 0 ]
