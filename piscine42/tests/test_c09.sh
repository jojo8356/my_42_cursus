#!/bin/sh
# Driver C09 : teste libft_creator.sh, le Makefile et ft_split.
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
T="${TMPDIR:-/tmp}"
PASS=0
FAIL=0

ok() { PASS=$((PASS + 1)); printf '  \033[32mPASS\033[0m %s\n' "$1"; }
ko() { FAIL=$((FAIL + 1)); printf '  \033[31mFAIL\033[0m %s\n' "$1"; }

echo "== C09 =="

# --- ex00 : libft_creator.sh ---
mkdir -p "$T/c09_ex00" && cp "$ROOT"/c09/ex00/* "$T/c09_ex00/"
if (cd "$T/c09_ex00" && sh libft_creator.sh) 2>"$T/c09_cc0.log"; then
	[ -f "$T/c09_ex00/libft.a" ] && ok "ex00 libft.a cree" || ko "ex00 libft.a cree"
	cat > "$T/c09_main0.c" << 'EOF'
#include <unistd.h>
void	ft_putchar(char c);
void	ft_swap(int *a, int *b);
void	ft_putstr(char *str);
int		ft_strlen(char *str);
int		ft_strcmp(char *s1, char *s2);
int	main(void)
{
	int	a;
	int	b;

	a = 1;
	b = 2;
	ft_putchar('A');
	ft_putstr("B");
	ft_swap(&a, &b);
	if (ft_strlen("abc") != 3 || ft_strcmp("a", "a") != 0 || a != 2 || b != 1)
		return (1);
	return (0);
}
EOF
	if cc -Wall -Wextra -Werror -o "$T/c09_bin0" "$T/c09_main0.c" \
		-L "$T/c09_ex00" -lft 2>"$T/c09_cc0.log" \
		&& [ "$("$T/c09_bin0")" = "AB" ]; then
		ok "ex00 libft.a se lie et fonctionne"
	else
		ko "ex00 libft.a se lie et fonctionne"
		cat "$T/c09_cc0.log"
	fi
	SYMS="$(nm "$T/c09_ex00/libft.a" 2>/dev/null | grep -c ' T ft_')"
	[ "$SYMS" -eq 5 ] && ok "ex00 les 5 symboles sont presents" \
		|| ko "ex00 symboles ($SYMS au lieu de 5)"
else
	ko "ex00 sh libft_creator.sh"
	cat "$T/c09_cc0.log"
fi

# --- ex01 : Makefile (srcs/ + includes/) ---
mkdir -p "$T/c09_ex01/srcs" "$T/c09_ex01/includes"
cp "$ROOT"/c09/ex00/*.c "$T/c09_ex01/srcs/"
printf '#ifndef FT_H\n# define FT_H\nvoid ft_putchar(char c);\n#endif\n' \
	> "$T/c09_ex01/includes/ft.h"
cp "$ROOT/c09/ex01/Makefile" "$T/c09_ex01/"
if (cd "$T/c09_ex01" && make) 2>"$T/c09_mk.log"; then
	[ -f "$T/c09_ex01/libft.a" ] && ok "ex01 make -> libft.a" || ko "ex01 make -> libft.a"
	[ -f "$T/c09_ex01/srcs/ft_putchar.o" ] && ok "ex01 .o a cote des .c" \
		|| ko "ex01 .o a cote des .c"
	OUT2="$(cd "$T/c09_ex01" && make 2>&1 | grep -c 'cc -Wall')"
	[ "$OUT2" = "0" ] && ok "ex01 pas de recompilation inutile" \
		|| ko "ex01 pas de recompilation inutile"
	(cd "$T/c09_ex01" && make clean >/dev/null 2>&1)
	[ ! -f "$T/c09_ex01/srcs/ft_putchar.o" ] && ok "ex01 clean supprime les .o" \
		|| ko "ex01 clean supprime les .o"
	[ -f "$T/c09_ex01/libft.a" ] && ok "ex01 clean garde la lib" \
		|| ko "ex01 clean garde la lib"
	(cd "$T/c09_ex01" && make fclean >/dev/null 2>&1)
	[ ! -f "$T/c09_ex01/libft.a" ] && ok "ex01 fclean supprime la lib" \
		|| ko "ex01 fclean supprime la lib"
	(cd "$T/c09_ex01" && make re >/dev/null 2>&1)
	[ -f "$T/c09_ex01/libft.a" ] && ok "ex01 re reconstruit tout" \
		|| ko "ex01 re reconstruit tout"
else
	ko "ex01 make"
	cat "$T/c09_mk.log"
fi

# --- ex02 : ft_split (charset) ---
cat > "$T/c09_main2.c" << 'EOF'
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
char	**ft_split(char *str, char *charset);
static void	free_tab(char **t) { int i = 0; while (t[i]) free(t[i++]); free(t); }
int	main(void)
{
	char	**t;
	t = ft_split("a1b2c", "12");
	if (!t || !t[0] || strcmp(t[0], "a") || !t[1] || strcmp(t[1], "b")
		|| !t[2] || strcmp(t[2], "c") || t[3]) { printf("K1\n"); return (1); }
	free_tab(t);
	t = ft_split("", "abc");
	if (!t || t[0]) { printf("K2\n"); return (1); }
	free_tab(t);
	t = ft_split("...", ".");
	if (!t || t[0]) { printf("K3\n"); return (1); }
	free_tab(t);
	t = ft_split("word", "");
	if (!t || !t[0] || strcmp(t[0], "word") || t[1]) { printf("K4\n"); return (1); }
	free_tab(t);
	t = ft_split("  a  b  ", " ");
	if (!t || !t[0] || strcmp(t[0], "a") || !t[1] || strcmp(t[1], "b") || t[2])
	{ printf("K5\n"); return (1); }
	free_tab(t);
	return (0);
}
EOF
if cc -Wall -Wextra -Werror -o "$T/c09_bin2" "$T/c09_main2.c" \
	"$ROOT/c09/ex02/ft_split.c" 2>"$T/c09_cc2.log"; then
	"$T/c09_bin2" && ok "ex02 ft_split tous les cas" || ko "ex02 ft_split tous les cas"
else
	ko "ex02 ft_split compilation"
	cat "$T/c09_cc2.log"
fi

rm -rf "$T/c09_ex00" "$T/c09_ex01" "$T"/c09_bin* "$T"/c09_main*.c
echo "C09 : $PASS reussi(s), $FAIL echec(s)"
[ "$FAIL" -eq 0 ]
