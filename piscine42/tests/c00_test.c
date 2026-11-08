/* Tests complets du jour C00. Compile avec les sources de c00/exNN.
   Les impressions sont capturees via redirection de stdout. */
#include <unistd.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void	ft_putchar(char c);
void	ft_print_alphabet(void);
void	ft_print_reverse_alphabet(void);
void	ft_print_numbers(void);
void	ft_is_negative(int n);
void	ft_print_comb(void);
void	ft_print_comb2(void);
void	ft_putnbr(int n);
void	ft_print_combn(int n);

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
	fd = open("/tmp/c00_cap.txt", O_RDWR | O_CREAT | O_TRUNC, 0644);
	dup2(fd, 1);
	close(fd);
}

static char	*end_capture(void)
{
	static char	buf[1048576];
	int			fd;
	int			n;

	fflush(stdout);
	dup2(g_saved, 1);
	close(g_saved);
	fd = open("/tmp/c00_cap.txt", O_RDONLY);
	n = read(fd, buf, sizeof(buf) - 1);
	if (n < 0)
		n = 0;
	buf[n] = '\0';
	close(fd);
	return (buf);
}

/* Construit la sortie attendue de ft_print_combn dans un buffer. */
static void	build_combn(char *out, int n, int *d, int depth, int *first)
{
	int	i;

	if (depth == n)
	{
		if (!*first)
			strcat(out, ", ");
		*first = 0;
		i = 0;
		while (i < n)
		{
			char s[2];

			s[0] = d[i] + '0';
			s[1] = '\0';
			strcat(out, s);
			i++;
		}
		return ;
	}
	i = 0;
	if (depth > 0)
		i = d[depth - 1] + 1;
	while (i <= 9 - n + 1 + depth)
	{
		d[depth] = i;
		build_combn(out, n, d, depth + 1, first);
		i++;
	}
}

int	main(void)
{
	char	*out;
	char	want[1048576];
	int		d[9];
	int		first;
	int		n;

	printf("== C00 ==\n");
	start_capture();
	ft_putchar('A');
	out = end_capture();
	check(strcmp(out, "A") == 0, "ex00 ft_putchar('A')");
	start_capture();
	ft_putchar('\n');
	out = end_capture();
	check(strcmp(out, "\n") == 0, "ex00 ft_putchar('\\n')");
	start_capture();
	ft_putchar('0');
	ft_putchar('z');
	out = end_capture();
	check(strcmp(out, "0z") == 0, "ex00 deux putchar consecutifs");
	start_capture();
	ft_print_alphabet();
	out = end_capture();
	check(strcmp(out, "abcdefghijklmnopqrstuvwxyz") == 0,
		"ex01 alphabet complet");
	start_capture();
	ft_print_reverse_alphabet();
	out = end_capture();
	check(strcmp(out, "zyxwvutsrqponmlkjihgfedcba") == 0,
		"ex02 alphabet inverse");
	start_capture();
	ft_print_numbers();
	out = end_capture();
	check(strcmp(out, "0123456789") == 0, "ex03 chiffres 0-9");
	start_capture();
	ft_is_negative(-42);
	out = end_capture();
	check(strcmp(out, "N\n") == 0, "ex04 negatif -> N");
	start_capture();
	ft_is_negative(0);
	out = end_capture();
	check(strcmp(out, "Z\n") == 0, "ex04 zero -> Z");
	start_capture();
	ft_is_negative(42);
	out = end_capture();
	check(strcmp(out, "P\n") == 0, "ex04 positif -> P");
	start_capture();
	ft_is_negative(INT_MIN);
	out = end_capture();
	check(strcmp(out, "N\n") == 0, "ex04 INT_MIN -> N");
	start_capture();
	ft_print_comb();
	out = end_capture();
	check(strlen(out) == 598 && strncmp(out, "012, 013", 8) == 0
		&& strcmp(out + strlen(out) - 3, "789") == 0
		&& out[strlen(out) - 1] != '\n', "ex05 comb (longueur 598, fin 789)");
	start_capture();
	ft_print_comb2();
	out = end_capture();
	check(strncmp(out, "00 01, 00 02", 12) == 0
		&& strcmp(out + strlen(out) - 5, "98 99") == 0
		&& strstr(out, "98 99,") == NULL, "ex06 comb2 (debut 00 01, fin 98 99)");
	start_capture();
	ft_putnbr(0);
	out = end_capture();
	check(strcmp(out, "0") == 0, "ex07 putnbr(0)");
	start_capture();
	ft_putnbr(42);
	out = end_capture();
	check(strcmp(out, "42") == 0, "ex07 putnbr(42)");
	start_capture();
	ft_putnbr(-42);
	out = end_capture();
	check(strcmp(out, "-42") == 0, "ex07 putnbr(-42)");
	start_capture();
	ft_putnbr(INT_MAX);
	out = end_capture();
	check(strcmp(out, "2147483647") == 0, "ex07 putnbr(INT_MAX)");
	start_capture();
	ft_putnbr(INT_MIN);
	out = end_capture();
	check(strcmp(out, "-2147483648") == 0, "ex07 putnbr(INT_MIN)");
	n = 1;
	while (n <= 9)
	{
		first = 1;
		want[0] = '\0';
		build_combn(want, n, d, 0, &first);
		start_capture();
		ft_print_combn(n);
		out = end_capture();
		if (strcmp(out, want) == 0)
			check(1, "ex08 combn valide");
		else
		{
			printf("        n=%d obtenu=%.40s... attendu=%.40s...\n",
				n, out, want);
			check(0, "ex08 combn valide");
		}
		n++;
	}
	start_capture();
	ft_print_combn(0);
	out = end_capture();
	check(strcmp(out, "") == 0, "ex08 combn(0) ne fait rien");
	start_capture();
	ft_print_combn(10);
	out = end_capture();
	check(strcmp(out, "") == 0, "ex08 combn(10) ne fait rien");
	printf("C00 : %d reussi(s), %d echec(s)\n", g_pass, g_fail);
	return (g_fail != 0);
}
