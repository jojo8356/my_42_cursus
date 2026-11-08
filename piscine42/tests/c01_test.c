/* Tests complets du jour C01 (pointeurs). */
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>

void	ft_ft(int *nbr);
void	ft_ultimate_ft(int *********nbr);
void	ft_swap(int *a, int *b);
void	ft_div_mod(int a, int b, int *div, int *mod);
void	ft_ultimate_div_mod(int *a, int *b);
void	ft_putstr(char *str);
int		ft_strlen(char *str);
void	ft_rev_int_tab(int *tab, int size);
void	ft_sort_int_tab(int *tab, int size);

static int	g_pass = 0;
static int	g_fail = 0;
static int	g_saved = -1;

static void	check(int cond, const char *name)
{
	if (cond)
	{
		g_pass++;
		printf("  PASS %s\n", name);
	}
	else
	{
		g_fail++;
		printf("  FAIL %s\n", name);
	}
}

static void	start_capture(void)
{
	int	fd;

	fflush(stdout);
	g_saved = dup(1);
	fd = open("/tmp/c01_cap.txt", O_RDWR | O_CREAT | O_TRUNC, 0644);
	dup2(fd, 1);
	close(fd);
}

static char	*end_capture(void)
{
	static char	buf[65536];
	int			fd;
	int			n;

	fflush(stdout);
	dup2(g_saved, 1);
	close(g_saved);
	fd = open("/tmp/c01_cap.txt", O_RDONLY);
	n = read(fd, buf, sizeof(buf) - 1);
	if (n < 0)
		n = 0;
	buf[n] = '\0';
	close(fd);
	return (buf);
}

int	main(void)
{
	int		n;
	int		a;
	int		b;
	int		div;
	int		mod;
	int		t1[5];
	char	*out;

	printf("== C01 ==\n");
	n = 0;
	ft_ft(&n);
	check(n == 42, "ex00 ft_ft met 42");
	{
		int		l9 = 0;
		int		*p1 = &l9;
		int		**p2 = &p1;
		int		***p3 = &p2;
		int		****p4 = &p3;
		int		*****p5 = &p4;
		int		******p6 = &p5;
		int		*******p7 = &p6;
		int		********p8 = &p7;

		ft_ultimate_ft(&p8);
		check(l9 == 42, "ex01 ultimate_ft met 42 (9 niveaux)");
	}
	a = 1;
	b = 2;
	ft_swap(&a, &b);
	check(a == 2 && b == 1, "ex02 swap (1,2) -> (2,1)");
	a = -1;
	b = -2;
	ft_swap(&a, &b);
	check(a == -2 && b == -1, "ex02 swap negatifs");
	ft_div_mod(42, 5, &div, &mod);
	check(div == 8 && mod == 2, "ex03 div_mod(42,5) -> (8,2)");
	ft_div_mod(-42, 5, &div, &mod);
	check(div == -8 && mod == -2, "ex03 div_mod(-42,5) -> (-8,-2)");
	ft_div_mod(5, 10, &div, &mod);
	check(div == 0 && mod == 5, "ex03 div_mod(5,10) -> (0,5)");
	a = 42;
	b = 5;
	ft_ultimate_div_mod(&a, &b);
	check(a == 8 && b == 2, "ex04 ultimate_div_mod(42,5) -> (8,2)");
	a = -42;
	b = 5;
	ft_ultimate_div_mod(&a, &b);
	check(a == -8 && b == -2, "ex04 ultimate_div_mod(-42,5)");
	start_capture();
	ft_putstr("Hello");
	out = end_capture();
	check(strcmp(out, "Hello") == 0, "ex05 putstr(\"Hello\")");
	start_capture();
	ft_putstr("");
	out = end_capture();
	check(strcmp(out, "") == 0, "ex05 putstr(\"\")");
	check(ft_strlen("") == 0, "ex06 strlen(\"\")");
	check(ft_strlen("Hello") == 5, "ex06 strlen(\"Hello\")");
	check(ft_strlen("0123456789abcdef") == 16, "ex06 strlen 16");
	t1[0] = 1;
	t1[1] = 2;
	t1[2] = 3;
	t1[3] = 4;
	t1[4] = 5;
	ft_rev_int_tab(t1, 5);
	check(t1[0] == 5 && t1[1] == 4 && t1[2] == 3 && t1[3] == 2
		&& t1[4] == 1, "ex07 rev_int_tab {1..5} -> {5..1}");
	ft_rev_int_tab(t1, 1);
	check(t1[0] == 5, "ex07 rev_int_tab taille 1 inchange");
	ft_rev_int_tab(t1, 0);
	check(t1[0] == 5, "ex07 rev_int_tab taille 0 inchange");
	t1[0] = 3;
	t1[1] = 1;
	t1[2] = 2;
	t1[3] = 5;
	t1[4] = 4;
	ft_sort_int_tab(t1, 5);
	check(t1[0] == 1 && t1[1] == 2 && t1[2] == 3 && t1[3] == 4
		&& t1[4] == 5, "ex08 sort_int_tab {3,1,2,5,4}");
	ft_sort_int_tab(t1, 5);
	check(t1[0] == 1 && t1[4] == 5, "ex08 sort_int_tab deja trie");
	t1[0] = 5;
	t1[1] = 4;
	t1[2] = 3;
	t1[3] = 2;
	t1[4] = 1;
	ft_sort_int_tab(t1, 5);
	check(t1[0] == 1 && t1[4] == 5, "ex08 sort_int_tab trie inverse");
	ft_sort_int_tab(t1, 0);
	check(t1[0] == 1, "ex08 sort_int_tab taille 0");
	printf("C01 : %d reussi(s), %d echec(s)\n", g_pass, g_fail);
	return (g_fail != 0);
}
