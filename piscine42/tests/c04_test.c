/* Tests complets du jour C04 (conversion, bases). */
#include <unistd.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

int		ft_strlen(char *str);
void	ft_putstr(char *str);
void	ft_putnbr(int n);
int		ft_atoi(char *str);
void	ft_putnbr_base(int nbr, char *base);
int		ft_atoi_base(char *str, char *base);

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
	fd = open("/tmp/c04_cap.txt", O_RDWR | O_CREAT | O_TRUNC, 0644);
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
	fd = open("/tmp/c04_cap.txt", O_RDONLY);
	n = read(fd, buf, sizeof(buf) - 1);
	if (n < 0)
		n = 0;
	buf[n] = '\0';
	close(fd);
	return (buf);
}

int	main(void)
{
	char	*out;

	printf("== C04 ==\n");
	check(ft_strlen("") == 0, "ex00 strlen vide");
	check(ft_strlen("Hello") == 5, "ex00 strlen Hello");
	check(ft_strlen("0123456789") == 10, "ex00 strlen 10");
	start_capture();
	ft_putstr("Hello World");
	out = end_capture();
	check(strcmp(out, "Hello World") == 0, "ex01 putstr");
	start_capture();
	ft_putstr("");
	out = end_capture();
	check(strcmp(out, "") == 0, "ex01 putstr vide");
	start_capture();
	ft_putnbr(0);
	out = end_capture();
	check(strcmp(out, "0") == 0, "ex02 putnbr(0)");
	start_capture();
	ft_putnbr(INT_MAX);
	out = end_capture();
	check(strcmp(out, "2147483647") == 0, "ex02 putnbr(INT_MAX)");
	start_capture();
	ft_putnbr(INT_MIN);
	out = end_capture();
	check(strcmp(out, "-2147483648") == 0, "ex02 putnbr(INT_MIN)");
	start_capture();
	ft_putnbr(-42);
	out = end_capture();
	check(strcmp(out, "-42") == 0, "ex02 putnbr(-42)");
	check(ft_atoi("42") == 42, "ex03 atoi 42");
	check(ft_atoi("   ---+--+1234ab567") == -1234, "ex03 atoi signes melanges");
	check(ft_atoi("\t\n\r\v\f  +12") == 12, "ex03 atoi espaces blancs");
	check(ft_atoi("  -2147483648") == INT_MIN, "ex03 atoi INT_MIN");
	check(ft_atoi("  +2147483647") == INT_MAX, "ex03 atoi INT_MAX");
	check(ft_atoi("abc") == 0, "ex03 atoi pas de chiffre");
	check(ft_atoi("") == 0, "ex03 atoi vide");
	check(ft_atoi("  4 2") == 4, "ex03 atoi s'arrete a l'espace");
	check(ft_atoi("--42") == 42, "ex03 atoi double moins -> +");
	start_capture();
	ft_putnbr_base(42, "0123456789");
	out = end_capture();
	check(strcmp(out, "42") == 0, "ex04 putnbr_base decimal");
	start_capture();
	ft_putnbr_base(-42, "0123456789");
	out = end_capture();
	check(strcmp(out, "-42") == 0, "ex04 putnbr_base negatif");
	start_capture();
	ft_putnbr_base(255, "0123456789ABCDEF");
	out = end_capture();
	check(strcmp(out, "FF") == 0, "ex04 putnbr_base hexa 255");
	start_capture();
	ft_putnbr_base(-255, "01");
	out = end_capture();
	check(strcmp(out, "-11111111") == 0, "ex04 putnbr_base binaire -255");
	start_capture();
	ft_putnbr_base(0, "0123456789");
	out = end_capture();
	check(strcmp(out, "0") == 0, "ex04 putnbr_base zero");
	start_capture();
	ft_putnbr_base(INT_MIN, "0123456789");
	out = end_capture();
	check(strcmp(out, "-2147483648") == 0, "ex04 putnbr_base INT_MIN");
	start_capture();
	ft_putnbr_base(42, "0");
	out = end_capture();
	check(strcmp(out, "") == 0, "ex04 base trop courte -> rien");
	start_capture();
	ft_putnbr_base(42, "012+3");
	out = end_capture();
	check(strcmp(out, "") == 0, "ex04 base avec + -> rien");
	start_capture();
	ft_putnbr_base(42, "01123");
	out = end_capture();
	check(strcmp(out, "") == 0, "ex04 base avec doublon -> rien");
	start_capture();
	ft_putnbr_base(42, "");
	out = end_capture();
	check(strcmp(out, "") == 0, "ex04 base vide -> rien");
	check(ft_atoi_base("42", "0123456789") == 42, "ex05 atoi_base decimal");
	check(ft_atoi_base("101010", "01") == 42, "ex05 atoi_base binaire");
	check(ft_atoi_base("   ---+--+2A", "0123456789ABCDEF") == -42,
		"ex05 atoi_base hexa signes");
	check(ft_atoi_base("abc", "0123456789abcdef") == 2748, "ex05 atoi_base abc hexa");
	check(ft_atoi_base("zzz", "0123456789abcdef") == 0, "ex05 digit hors base -> 0");
	check(ft_atoi_base("abc", "0") == 0, "ex05 base invalide -> 0");
	check(ft_atoi_base("abc", "011") == 0, "ex05 doublon -> 0");
	check(ft_atoi_base("", "01") == 0, "ex05 chaine vide -> 0");
	check(ft_atoi_base("--1", "01") == 1, "ex05 double negatif");
	check(ft_atoi_base("-2147483648", "0123456789") == INT_MIN,
		"ex05 atoi_base INT_MIN");
	printf("C04 : %d reussi(s), %d echec(s)\n", g_pass, g_fail);
	return (g_fail != 0);
}
