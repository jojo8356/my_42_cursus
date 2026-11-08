#!/bin/sh
# /etc/passwd -> on retire les commentaires, on garde une ligne sur deux
# (en commencant par la 2e), on inverse chaque login, on trie par ordre
# inverse, on garde FT_LINE1..FT_LINE2, on colle avec ", " et un final ".".
grep -v '^#' /etc/passwd | awk -F: 'NR % 2 == 0 { print $1 }' | rev \
	| sort -r | sed -n "$FT_LINE1,$FT_LINE2 p" | paste -sd ',' - \
	| sed 's/,/, /g;s/$/./'
