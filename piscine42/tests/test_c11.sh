#!/bin/sh
# Driver C11 : teste les fonctions + le programme do-op.
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
T="${TMPDIR:-/tmp}"
PASS=0
FAIL=0

ok() { PASS=$((PASS + 1)); printf '  \033[32mPASS\033[0m %s\n' "$1"; }
ko() { FAIL=$((FAIL + 1)); printf '  \033[31mFAIL\033[0m %s\n' "$1"; }

echo "== C11 =="

# --- fonctions ex00-ex04, ex06, ex07 ---
cat > "$T/c11_fn.c" << 'EOF'
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
void	ft_foreach(int *tab, int length, void (*f)(int));
int		*ft_map(int *tab, int length, int (*f)(int));
int		ft_any(char **tab, int (*f)(char *));
int		ft_count_if(char **tab, int (*f)(char *));
int		ft_is_sort(int *tab, int length, int (*f)(int, int));
void	ft_sort_string_tab(char **tab);
void	ft_advanced_sort_string_tab(char **tab, int (*cmp)(char *, char *));

static int	g_sum;
static void	add_to_sum(int n) { g_sum += n; }
static int	times2(int n) { return (n * 2); }
static int	is_a(char *s) { return (*s == 'a'); }
static int	cmp_int(int a, int b) { return (a - b); }
static int	cmp_len(char *a, char *b)
{
	int	la = 0, lb = 0;
	while (a[la]) la++;
	while (b[lb]) lb++;
	return (la - lb);
}
static int	cmp_str(char *a, char *b)
{
	while (*a && *a == *b) { a++; b++; }
	return ((unsigned char)*a - (unsigned char)*b);
}
static int	bad(int cond, const char *name)
{
	printf("  %s %s\n", cond ? "PASS" : "FAIL", name);
	return (cond ? 0 : 1);
}
int	main(void)
{
	int		err = 0;
	int		tab[4] = {1, 2, 3, 4};
	int		*m;
	char	*strs[5];
	int		sorted_asc[3] = {1, 2, 3};
	int		sorted_desc[3] = {3, 2, 1};
	int		unsorted[3] = {1, 3, 2};
	int		same[3] = {2, 2, 2};

	g_sum = 0;
	ft_foreach(tab, 4, add_to_sum);
	err += bad(g_sum == 10, "ex00 foreach applique f");
	m = ft_map(tab, 4, times2);
	err += bad(m && m[0] == 2 && m[3] == 8, "ex01 map double");
	free(m);
	strs[0] = "banana"; strs[1] = "apple"; strs[2] = "avocado"; strs[3] = NULL;
	err += bad(ft_any(strs, is_a) == 1, "ex02 any -> 1");
	err += bad(ft_any(strs, is_a) == 1, "ex02 any stable");
	strs[0] = "zzz"; strs[1] = "yyy"; strs[2] = NULL;
	err += bad(ft_any(strs, is_a) == 0, "ex02 any -> 0");
	strs[0] = "abc"; strs[1] = "a"; strs[2] = "b"; strs[3] = "axy"; strs[4] = NULL;
	err += bad(ft_count_if(strs, is_a) == 3, "ex03 count_if == 3");
	err += bad(ft_is_sort(sorted_asc, 3, cmp_int) == 1, "ex04 is_sort croissant");
	err += bad(ft_is_sort(sorted_desc, 3, cmp_int) == 1, "ex04 is_sort decroissant");
	err += bad(ft_is_sort(unsorted, 3, cmp_int) == 0, "ex04 is_sort non trie");
	err += bad(ft_is_sort(same, 3, cmp_int) == 1, "ex04 is_sort elements egaux");
	err += bad(ft_is_sort(tab, 1, cmp_int) == 1, "ex04 is_sort taille 1");
	strs[0] = "bb"; strs[1] = "a"; strs[2] = "ccc"; strs[3] = "ab"; strs[4] = NULL;
	ft_sort_string_tab(strs);
	err += bad(!strcmp(strs[0], "a") && !strcmp(strs[1], "ab")
		&& !strcmp(strs[2], "bb") && !strcmp(strs[3], "ccc"),
		"ex06 sort_string_tab ASCII");
	strs[0] = "bb"; strs[1] = "a"; strs[2] = "ccc"; strs[3] = "ab"; strs[4] = NULL;
	ft_advanced_sort_string_tab(strs, cmp_len);
	err += bad(!strcmp(strs[0], "a") && !strcmp(strs[1], "bb")
		&& !strcmp(strs[2], "ab") && !strcmp(strs[3], "ccc"),
		"ex07 advanced_sort par longueur");
	strs[0] = "bb"; strs[1] = "a"; strs[2] = "ccc"; strs[3] = "ab"; strs[4] = NULL;
	ft_advanced_sort_string_tab(strs, cmp_str);
	err += bad(!strcmp(strs[0], "a") && !strcmp(strs[1], "ab")
		&& !strcmp(strs[2], "bb") && !strcmp(strs[3], "ccc"),
		"ex07 advanced_sort ASCII");
	return (err);
}
EOF
if gcc -Wall -Wextra -Werror -o "$T/c11_fn" "$T/c11_fn.c" \
	"$ROOT"/c11/ex00/ft_foreach.c "$ROOT"/c11/ex01/ft_map.c \
	"$ROOT"/c11/ex02/ft_any.c "$ROOT"/c11/ex03/ft_count_if.c \
	"$ROOT"/c11/ex04/ft_is_sort.c "$ROOT"/c11/ex06/ft_sort_string_tab.c \
	"$ROOT"/c11/ex07/ft_advanced_sort_string_tab.c 2>"$T/c11_cc.log"; then
	"$T/c11_fn"
	[ $? -eq 0 ] && ok "ex00-ex07 toutes les fonctions" || ko "ex00-ex07 toutes les fonctions"
else
	ko "ex00-ex07 compilation"
	cat "$T/c11_cc.log"
fi

# --- ex05 : do-op ---
cp -r "$ROOT/c11/ex05" "$T/c11_ex05"
if (cd "$T/c11_ex05" && make >/dev/null 2>&1); then
	B="$T/c11_ex05/do-op"
	[ "$("$B" 42 + 21)" = "63" ] && ok "ex05 42 + 21 = 63" || ko "ex05 42 + 21"
	[ "$("$B" 42amis - --+-20toto12)" = "62" ] && ok "ex05 parsing atoi style" \
		|| ko "ex05 parsing atoi style (obtenu: $("$B" 42amis - --+-20toto12))"
	[ "$("$B" 1 + toto3)" = "1" ] && ok "ex05 toto3 -> 0" || ko "ex05 toto3 -> 0"
	[ "$("$B" toto3 + 4)" = "4" ] && ok "ex05 gauche non numerique" || ko "ex05 gauche"
	[ "$("$B" 1 p 1)" = "0" ] && ok "ex05 operateur invalide -> 0" || ko "ex05 op invalide"
	[ "$("$B" foo plus bar)" = "0" ] && ok "ex05 tout invalide -> 0" || ko "ex05 tout invalide"
	[ "$("$B" 25 / 0)" = "Stop : division by zero" ] && ok "ex05 div par zero" \
		|| ko "ex05 div par zero (obtenu: $("$B" 25 / 0))"
	[ "$("$B" 25 % 0)" = "Stop : modulo by zero" ] && ok "ex05 mod par zero" \
		|| ko "ex05 mod par zero"
	[ -z "$("$B")" ] && ok "ex05 sans arguments -> rien" || ko "ex05 sans arguments"
	[ -z "$("$B" 1 + 1 + 1)" ] && ok "ex05 trop d'arguments -> rien" || ko "ex05 trop d'args"
	[ "$("$B" 7 "*" 6)" = "42" ] && ok "ex05 7 * 6 = 42" || ko "ex05 7 * 6"
	[ "$("$B" 42 / 21)" = "2" ] && ok "ex05 42 / 21 = 2" || ko "ex05 42 / 21"
	[ "$("$B" 42 % 21)" = "0" ] && ok "ex05 42 % 21 = 0" || ko "ex05 42 % 21"
	[ "$("$B" -5 + -3)" = "-8" ] && ok "ex05 negatifs" || ko "ex05 negatifs"
else
	ko "ex05 make"
fi

rm -rf "$T/c11_fn.c" "$T/c11_fn" "$T/c11_ex05"
echo "C11 : $PASS reussi(s), $FAIL echec(s)"
[ "$FAIL" -eq 0 ]
