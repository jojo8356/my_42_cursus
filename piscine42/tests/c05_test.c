/* Tests complets du jour C05 (recursivite, maths, dix dames). */
#include <unistd.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

int	ft_iterative_factorial(int nb);
int	ft_recursive_factorial(int nb);
int	ft_iterative_power(int nb, int power);
int	ft_recursive_power(int nb, int power);
int	ft_fibonacci(int index);
int	ft_sqrt(int nb);
int	ft_is_prime(int nb);
int	ft_find_next_prime(int nb);
int	ft_ten_queens_puzzle(void);

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
	fd = open("/tmp/c05_cap.txt", O_RDWR | O_CREAT | O_TRUNC, 0644);
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
	fd = open("/tmp/c05_cap.txt", O_RDONLY);
	n = read(fd, buf, sizeof(buf) - 1);
	if (n < 0)
		n = 0;
	buf[n] = '\0';
	close(fd);
	return (buf);
}

/* Verifie qu'une solution des 10 dames est valide (pas de conflit). */
static int	sol_valid(char *line)
{
	int	i;
	int	j;

	i = 0;
	while (i < 10)
	{
		if (line[i] < '0' || line[i] > '9')
			return (0);
		j = i + 1;
		while (j < 10)
		{
			if (line[i] == line[j])
				return (0);
			if (line[i] - i == line[j] - j || line[i] + i == line[j] + j)
				return (0);
			j++;
		}
		i++;
	}
	return (1);
}

static void	test_queens(void)
{
	char	*out;
	int		count;
	int		valid;
	int		n;
	int		i;

	start_capture();
	n = ft_ten_queens_puzzle();
	out = end_capture();
	check(n == 724, "ex08 dix dames : 724 solutions (retour)");
	count = 0;
	valid = 1;
	i = 0;
	while (out[i])
	{
		if (out[i + 10] == '\n' && (out[i + 11] == '\0' || out[i + 11]))
		{
			if (!sol_valid(out + i))
				valid = 0;
			count++;
			i += 11;
		}
		else
		{
			valid = 0;
			break ;
		}
	}
	check(count == 724, "ex08 dix dames : 724 lignes affichees");
	check(valid, "ex08 dix dames : toutes les solutions valides");
	check(strstr(out, "0257948136") != NULL
		|| strstr(out, "0") == out, "ex08 dix dames : format 10 chiffres");
}

int	main(void)
{
	printf("== C05 ==\n");
	check(ft_iterative_factorial(0) == 1, "ex00 iter_fact(0) = 1");
	check(ft_iterative_factorial(1) == 1, "ex00 iter_fact(1) = 1");
	check(ft_iterative_factorial(5) == 120, "ex00 iter_fact(5) = 120");
	check(ft_iterative_factorial(12) == 479001600, "ex00 iter_fact(12)");
	check(ft_iterative_factorial(-1) == 0, "ex00 iter_fact(-1) = 0");
	check(ft_recursive_factorial(0) == 1, "ex01 rec_fact(0) = 1");
	check(ft_recursive_factorial(5) == 120, "ex01 rec_fact(5) = 120");
	check(ft_recursive_factorial(12) == 479001600, "ex01 rec_fact(12)");
	check(ft_recursive_factorial(-42) == 0, "ex01 rec_fact(-42) = 0");
	check(ft_iterative_power(2, 10) == 1024, "ex02 iter_pow(2,10)");
	check(ft_iterative_power(42, 0) == 1, "ex02 iter_pow(42,0) = 1");
	check(ft_iterative_power(0, 0) == 1, "ex02 iter_pow(0,0) = 1");
	check(ft_iterative_power(2, -1) == 0, "ex02 iter_pow(2,-1) = 0");
	check(ft_iterative_power(-2, 3) == -8, "ex02 iter_pow(-2,3) = -8");
	check(ft_recursive_power(2, 10) == 1024, "ex03 rec_pow(2,10)");
	check(ft_recursive_power(42, 0) == 1, "ex03 rec_pow(42,0) = 1");
	check(ft_recursive_power(2, -1) == 0, "ex03 rec_pow(2,-1) = 0");
	check(ft_recursive_power(10, 9) == 1000000000, "ex03 rec_pow(10,9)");
	check(ft_fibonacci(0) == 0, "ex04 fibo(0) = 0");
	check(ft_fibonacci(1) == 1, "ex04 fibo(1) = 1");
	check(ft_fibonacci(2) == 1, "ex04 fibo(2) = 1");
	check(ft_fibonacci(10) == 55, "ex04 fibo(10) = 55");
	check(ft_fibonacci(15) == 610, "ex04 fibo(15) = 610");
	check(ft_fibonacci(-1) == -1, "ex04 fibo(-1) = -1");
	check(ft_sqrt(0) == 0, "ex05 sqrt(0) = 0");
	check(ft_sqrt(1) == 1, "ex05 sqrt(1) = 1");
	check(ft_sqrt(4) == 2, "ex05 sqrt(4) = 2");
	check(ft_sqrt(16) == 4, "ex05 sqrt(16) = 4");
	check(ft_sqrt(17) == 0, "ex05 sqrt(17) = 0");
	check(ft_sqrt(2147395600) == 46340, "ex05 sqrt(46340^2)");
	check(ft_sqrt(INT_MAX) == 0, "ex05 sqrt(INT_MAX) = 0");
	check(ft_sqrt(-4) == 0, "ex05 sqrt(-4) = 0");
	check(ft_is_prime(0) == 0 && ft_is_prime(1) == 0, "ex06 0 et 1 non premiers");
	check(ft_is_prime(2) == 1 && ft_is_prime(3) == 1, "ex06 2 et 3 premiers");
	check(ft_is_prime(4) == 0 && ft_is_prime(9) == 0, "ex06 4 et 9 non premiers");
	check(ft_is_prime(2147483647) == 1, "ex06 INT_MAX premier");
	check(ft_is_prime(-7) == 0, "ex06 negatif non premier");
	check(ft_find_next_prime(0) == 2, "ex07 next_prime(0) = 2");
	check(ft_find_next_prime(2) == 2, "ex07 next_prime(2) = 2");
	check(ft_find_next_prime(4) == 5, "ex07 next_prime(4) = 5");
	check(ft_find_next_prime(8) == 11, "ex07 next_prime(8) = 11");
	check(ft_find_next_prime(2147483646) == 2147483647, "ex07 next_prime max");
	test_queens();
	printf("C05 : %d reussi(s), %d echec(s)\n", g_pass, g_fail);
	return (g_fail != 0);
}
