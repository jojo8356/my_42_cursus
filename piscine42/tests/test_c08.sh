#!/bin/sh
# Driver C08 : compile les headers avec les mains du style sujet et
# teste ft_strs_to_tab / ft_show_tab.
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
T="${TMPDIR:-/tmp}"
PASS=0
FAIL=0

ok() { PASS=$((PASS + 1)); printf '  \033[32mPASS\033[0m %s\n' "$1"; }
ko() { FAIL=$((FAIL + 1)); printf '  \033[31mFAIL\033[0m %s\n' "$1"; }

echo "== C08 =="

# --- ex00 : ft.h compile avec un main utilisant ses prototypes ---
cat > "$T/c08_main0.c" << 'EOF'
#include "ft.h"
#include <stdio.h>
int	main(void)
{
	int	a;
	int	b;

	a = 1;
	b = 2;
	ft_putchar('a');
	ft_putstr("b");
	printf("%d", ft_strlen("cd"));
	printf("%d", ft_strcmp("a", "b"));
	ft_swap(&a, &b);
	printf("%d%d", a, b);
	return (0);
}
EOF
if gcc -Wall -Wextra -Werror -I "$ROOT/c08/ex00" -o "$T/c08_ex00" \
	"$T/c08_main0.c" "$ROOT"/c00/ex00/ft_putchar.c "$ROOT"/c01/ex05/ft_putstr.c \
	"$ROOT"/c01/ex06/ft_strlen.c "$ROOT"/c03/ex00/ft_strcmp.c \
	"$ROOT"/c01/ex02/ft_swap.c 2>"$T/c08_cc0.log"; then
	ok "ex00 ft.h compile avec les 5 prototypes"
else
	ko "ex00 ft.h compile avec les 5 prototypes"
	cat "$T/c08_cc0.log"
fi

# --- ex01 : ft_boolean.h avec le main du sujet ---
cat > "$T/c08_main1.c" << 'EOF'
#include "ft_boolean.h"
int	main(int argc, char **argv)
{
	(void)argv;
	if (EVEN(argc))
		ft_putstr(EVEN_MSG);
	else
		ft_putstr(ODD_MSG);
	return (SUCCESS);
}
EOF
if gcc -Wall -Wextra -Werror -I "$ROOT/c08/ex01" -o "$T/c08_ex01" \
	"$T/c08_main1.c" "$ROOT"/c01/ex05/ft_putstr.c 2>"$T/c08_cc1.log"; then
	GOT="$("$T/c08_ex01")"
	[ "$GOT" = "I have an odd number of arguments." ] \
		&& ok "ex01 ft_boolean.h (1 arg -> odd)" \
		|| ko "ex01 ft_boolean.h (obtenu: $GOT)"
	GOT="$("$T/c08_ex01" a)"
	[ "$GOT" = "I have an even number of arguments." ] \
		&& ok "ex01 ft_boolean.h (2 args -> even)" \
		|| ko "ex01 ft_boolean.h (obtenu: $GOT)"
else
	ko "ex01 ft_boolean.h compile"
	cat "$T/c08_cc1.log"
fi

# --- ex02 : ft_abs.h ---
cat > "$T/c08_main2.c" << 'EOF'
#include "ft_abs.h"
int	main(void) { return (ABS(-42) == 42 && ABS(42) == 42 && ABS(0) == 0 ? 0 : 1); }
EOF
if gcc -Wall -Wextra -Werror -I "$ROOT/c08/ex02" -o "$T/c08_ex02" \
	"$T/c08_main2.c" 2>"$T/c08_cc2.log"; then
	"$T/c08_ex02" && ok "ex02 ft_abs.h ABS(-42)=42" || ko "ex02 ft_abs.h valeurs"
else
	ko "ex02 ft_abs.h compile"
	cat "$T/c08_cc2.log"
fi

# --- ex03 : ft_point.h ---
cat > "$T/c08_main3.c" << 'EOF'
#include "ft_point.h"
void	set_point(t_point *p) { p->x = 42; p->y = 21; }
int	main(void) { t_point p; set_point(&p); return (p.x == 42 && p.y == 21 ? 0 : 1); }
EOF
if gcc -Wall -Wextra -Werror -I "$ROOT/c08/ex03" -o "$T/c08_ex03" \
	"$T/c08_main3.c" 2>"$T/c08_cc3.log"; then
	"$T/c08_ex03" && ok "ex03 ft_point.h structure t_point" || ko "ex03 valeurs"
else
	ko "ex03 ft_point.h compile"
	cat "$T/c08_cc3.log"
fi

# --- ex04/ex05 : ft_strs_to_tab + ft_show_tab (cross-testes) ---
cat > "$T/c08_main45.c" << 'EOF'
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include "ft_stock_str.h"
struct s_stock_str	*ft_strs_to_tab(int ac, char **av);
void				ft_show_tab(struct s_stock_str *par);
int	main(void)
{
	char	*av[] = {"Hello", "", "42 school"};
	struct s_stock_str *t = ft_strs_to_tab(3, av);
	int	fd, n; static char buf[4096];
	if (!t) { printf("NULL\n"); return (1); }
	if (strcmp(t[1].str, "") != 0 || t[0].size != 5 || t[2].size != 9) { printf("BAD FIELDS\n"); return (1); }
	if (strcmp(t[0].copy, "Hello") != 0 || t[0].copy == t[0].str) { printf("BAD COPY\n"); return (1); }
	if (t[3].str != 0) { printf("NO TERMINATOR\n"); return (1); }
	t[0].copy[0] = 'h';
	fflush(stdout); fd = open("/tmp/c08_cap.txt", O_RDWR | O_CREAT | O_TRUNC, 0644);
	dup2(fd, 1); close(fd);
	ft_show_tab(t);
	fflush(stdout); dup2(2, 1);
	fd = open("/tmp/c08_cap.txt", O_RDONLY);
	n = read(fd, buf, sizeof(buf) - 1); buf[n] = 0; close(fd);
	if (strcmp(buf, "Hello\n5\nhello\n\n0\n\n42 school\n9\n42 school\n") != 0) {
		dprintf(2, "SHOW=[%s]\n", buf); return (1); }
	return (0);
}
EOF
if gcc -Wall -Wextra -Werror -I "$ROOT/c08/ex04" -o "$T/c08_ex45" \
	"$T/c08_main45.c" "$ROOT"/c08/ex04/ft_strs_to_tab.c \
	"$ROOT"/c08/ex05/ft_show_tab.c 2>"$T/c08_cc45.log"; then
	"$T/c08_ex45" 2>"$T/c08_run45.log" && ok "ex04/ex05 strs_to_tab + show_tab" \
		|| { ko "ex04/ex05 strs_to_tab + show_tab"; cat "$T/c08_run45.log"; }
else
	ko "ex04/ex05 compilation"
	cat "$T/c08_cc45.log"
fi

rm -f "$T"/c08_ex* "$T"/c08_main*.c
echo "C08 : $PASS reussi(s), $FAIL echec(s)"
[ "$FAIL" -eq 0 ]
