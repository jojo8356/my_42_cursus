/* Tests complets du jour C07 (malloc). */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char	*ft_strdup(char *src);
int		*ft_range(int min, int max);
int		ft_ultimate_range(int **range, int min, int max);
char	*ft_strjoin(int size, char **strs, char *sep);
char	*ft_convert_base(char *nbr, char *base_from, char *base_to);
char	**ft_split(char *str, char *charset);

static int	g_pass = 0;
static int	g_fail = 0;

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

/* Verifie un tableau [min, max[ croissant. */
static void	check_range(int *tab, int min, int max, const char *name)
{
	int	ok;
	int	i;

	ok = (tab != NULL);
	i = 0;
	while (ok && min + i < max)
	{
		if (tab[i] != min + i)
			ok = 0;
		i++;
	}
	check(ok, name);
}

static void	test_strjoin(void)
{
	char	*strs[4];
	char	*out;

	strs[0] = "Hello";
	strs[1] = "beautiful";
	strs[2] = "world";
	out = ft_strjoin(3, strs, " ");
	check(out != NULL && strcmp(out, "Hello beautiful world") == 0,
		"ex03 strjoin 3 mots + espace");
	free(out);
	out = ft_strjoin(3, strs, "");
	check(out != NULL && strcmp(out, "Hellobeautifulworld") == 0,
		"ex03 strjoin separateur vide");
	free(out);
	out = ft_strjoin(1, strs, "---");
	check(out != NULL && strcmp(out, "Hello") == 0,
		"ex03 strjoin 1 seul mot");
	free(out);
	out = ft_strjoin(0, strs, "x");
	check(out != NULL && strcmp(out, "") == 0, "ex03 strjoin size 0");
	free(out);
	strs[0] = "";
	strs[1] = "";
	out = ft_strjoin(2, strs, "sep");
	check(out != NULL && strcmp(out, "sep") == 0, "ex03 strjoin vides");
	free(out);
}

static void	test_convert_base(void)
{
	char	*out;

	out = ft_convert_base("42", "0123456789", "0123456789");
	check(out != NULL && strcmp(out, "42") == 0, "ex04 convert decimal->decimal");
	free(out);
	out = ft_convert_base("   ---+--+2a", "0123456789abcdef", "01");
	check(out != NULL && strcmp(out, "-101010") == 0,
		"ex04 convert hexa signes -> binaire");
	free(out);
	out = ft_convert_base("101010", "01", "0123456789");
	check(out != NULL && strcmp(out, "42") == 0, "ex04 convert binaire->decimal");
	free(out);
	out = ft_convert_base("0", "0123456789", "0123456789abcdef");
	check(out != NULL && strcmp(out, "0") == 0, "ex04 convert zero");
	free(out);
	out = ft_convert_base("-2147483648", "0123456789", "0123456789");
	check(out != NULL && strcmp(out, "-2147483648") == 0,
		"ex04 convert INT_MIN aller-retour");
	free(out);
	out = ft_convert_base("42", "0", "0123456789");
	check(out == NULL, "ex04 base_from invalide -> NULL");
	out = ft_convert_base("42", "0123456789", "011");
	check(out == NULL, "ex04 base_to avec doublon -> NULL");
	out = ft_convert_base("bc", "0123456789abcdef", "gtaio luSnemf");
	check(out != NULL && strcmp(out, "ttl") == 0, "ex04 hexa -> base 13 custom");
	free(out);
}

static void	test_split(void)
{
	char	**t;

	t = ft_split("  bonjour ,  les gens !! ", " ,!");
	check(t != NULL && t[0] && strcmp(t[0], "bonjour") == 0
		&& t[1] && strcmp(t[1], "les") == 0
		&& t[2] && strcmp(t[2], "gens") == 0
		&& t[3] == NULL, "ex05 split mots separes");
	t = ft_split("abc", "");
	check(t != NULL && t[0] && strcmp(t[0], "abc") == 0 && t[1] == NULL,
		"ex05 split charset vide");
	t = ft_split("", "abc");
	check(t != NULL && t[0] == NULL, "ex05 split chaine vide");
	t = ft_split("aaaa", "a");
	check(t != NULL && t[0] == NULL, "ex05 split que des separateurs");
	t = ft_split("a.b..c...", ".");
	check(t != NULL && t[0] && strcmp(t[0], "a") == 0
		&& t[1] && strcmp(t[1], "b") == 0
		&& t[2] && strcmp(t[2], "c") == 0 && t[3] == NULL,
		"ex05 split separateurs consecutifs");
	t = ft_split("mot", "xyz");
	check(t != NULL && t[0] && strcmp(t[0], "mot") == 0 && t[1] == NULL,
		"ex05 split charset sans effet");
	t = ft_split("ab", "ab");
	check(t != NULL && t[0] == NULL, "ex05 split tout est separateur");
}

int	main(void)
{
	char	*s;
	int		*tab;
	int		*r2;

	printf("== C07 ==\n");
	s = ft_strdup("Hello");
	check(s != NULL && strcmp(s, "Hello") == 0, "ex00 strdup Hello");
	free(s);
	s = ft_strdup("");
	check(s != NULL && strcmp(s, "") == 0, "ex00 strdup vide");
	free(s);
	tab = ft_range(1, 5);
	check_range(tab, 1, 5, "ex01 range(1,5) = [1..4]");
	free(tab);
	tab = ft_range(0, 0);
	check(tab == NULL, "ex01 range(0,0) -> NULL");
	tab = ft_range(5, 1);
	check(tab == NULL, "ex01 range(5,1) -> NULL");
	tab = ft_range(-3, 3);
	check_range(tab, -3, 3, "ex01 range(-3,3) = [-3..2]");
	free(tab);
	tab = ft_range(7, 8);
	check(tab != NULL && tab[0] == 7, "ex01 range(7,8) = [7]");
	free(tab);
	r2 = NULL;
	check(ft_ultimate_range(&r2, 3, 3) == 0 && r2 == NULL,
		"ex02 ultimate_range(3,3) -> 0");
	r2 = NULL;
	check(ft_ultimate_range(&r2, 2, 6) == 4 && r2 != NULL
		&& r2[0] == 2 && r2[3] == 5, "ex02 ultimate_range(2,6) -> 4");
	free(r2);
	r2 = NULL;
	check(ft_ultimate_range(&r2, 6, 2) == 0 && r2 == NULL,
		"ex02 ultimate_range(6,2) -> 0");
	r2 = NULL;
	check(ft_ultimate_range(&r2, -1, 1) == 2 && r2 != NULL
		&& r2[0] == -1 && r2[1] == 0, "ex02 ultimate_range(-1,1) -> 2");
	free(r2);
	test_strjoin();
	test_convert_base();
	test_split();
	printf("C07 : %d reussi(s), %d echec(s)\n", g_pass, g_fail);
	return (g_fail != 0);
}
