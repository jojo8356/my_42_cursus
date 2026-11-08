/* Tests complets du jour C03 (comparaison/concatenation de chaines). */
#include <stdio.h>
#include <string.h>

int				ft_strcmp(char *s1, char *s2);
int				ft_strncmp(char *s1, char *s2, unsigned int n);
char			*ft_strcat(char *dest, char *src);
char			*ft_strncat(char *dest, char *src, unsigned int nb);
char			*ft_strstr(char *str, char *to_find);
unsigned int	ft_strlcat(char *dest, char *src, unsigned int size);

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

int	main(void)
{
	char	d[128];
	int		r;

	printf("== C03 ==\n");
	check(ft_strcmp("abc", "abc") == 0, "ex00 strcmp egales");
	check(ft_strcmp("abc", "abd") < 0, "ex00 strcmp abc < abd");
	check(ft_strcmp("abd", "abc") > 0, "ex00 strcmp abd > abc");
	check(ft_strcmp("ab", "abc") < 0, "ex00 strcmp prefixe < longue");
	check(ft_strcmp("", "") == 0, "ex00 strcmp vides");
	check(ft_strcmp("a", "") > 0, "ex00 strcmp non vide > vide");
	check(ft_strcmp("A", "a") < 0, "ex00 strcmp majuscule < minuscule");
	check(ft_strcmp("\x80", "\x7f") > 0, "ex00 strcmp unsigned char");
	check(ft_strncmp("abc", "abd", 2) == 0, "ex01 strncmp n=2 egales");
	check(ft_strncmp("abc", "abd", 3) < 0, "ex01 strncmp n=3 differentes");
	check(ft_strncmp("abc", "abd", 0) == 0, "ex01 strncmp n=0");
	check(ft_strncmp("ab", "abc", 5) < 0, "ex01 strncmp prefixe");
	check(ft_strncmp("", "", 5) == 0, "ex01 strncmp vides");
	strcpy(d, "Hello");
	r = (ft_strcat(d, " World") == d && strcmp(d, "Hello World") == 0);
	check(r, "ex02 strcat Hello + World");
	strcpy(d, "42");
	ft_strcat(d, "");
	check(strcmp(d, "42") == 0, "ex02 strcat + vide");
	strcpy(d, "");
	ft_strcat(d, "abc");
	check(strcmp(d, "abc") == 0, "ex02 strcat vide + abc");
	strcpy(d, "Hello");
	r = (ft_strncat(d, " World", 3) == d && strcmp(d, "Hello Wo") == 0);
	check(r, "ex03 strncat 3 caracteres");
	strcpy(d, "Hello");
	ft_strncat(d, " World", 20);
	check(strcmp(d, "Hello World") == 0, "ex03 strncat nb > strlen(src)");
	strcpy(d, "Hello");
	ft_strncat(d, " World", 0);
	check(strcmp(d, "Hello") == 0, "ex03 strncat nb=0");
	check(ft_strstr("Hello World", "World") != NULL
		&& strcmp(ft_strstr("Hello World", "World"), "World") == 0,
		"ex04 strstr trouve World");
	check(ft_strstr("Hello", "") == (char *)"Hello"
		|| ft_strstr("Hello", "") != NULL,
		"ex04 strstr to_find vide -> str");
	check(ft_strstr("Hello", "xyz") == NULL, "ex04 strstr absent -> NULL");
	check(ft_strstr("", "a") == NULL, "ex04 strstr dans vide -> NULL");
	check(ft_strstr("aaaaab", "aab") != NULL
		&& strcmp(ft_strstr("aaaaab", "aab"), "aab") == 0,
		"ex04 strstr occurrence avec chevauchement");
	strcpy(d, "Hello");
	r = ft_strlcat(d, " World", 32);
	check(r == 11 && strcmp(d, "Hello World") == 0,
		"ex05 strlcat normal -> 11");
	strcpy(d, "Hello");
	r = ft_strlcat(d, " World", 8);
	check(r == 11 && strcmp(d, "Hello W") == 0, "ex05 strlcat tronque -> 11");
	strcpy(d, "Hello");
	r = ft_strlcat(d, " World", 4);
	check(r == 10 && strcmp(d, "Hello") == 0, "ex05 strlcat size=4 -> 10");
	strcpy(d, "Hello");
	r = ft_strlcat(d, " World", 5);
	check(r == 11 && strcmp(d, "Hello") == 0, "ex05 strlcat size=5 -> 11");
	strcpy(d, "Hello");
	r = ft_strlcat(d, " World", 0);
	check(r == 6 && strcmp(d, "Hello") == 0, "ex05 strlcat size=0 -> 6");
	strcpy(d, "");
	r = ft_strlcat(d, "abc", 2);
	check(r == 3 && strcmp(d, "a") == 0, "ex05 strlcat dest vide tronque");
	printf("C03 : %d reussi(s), %d echec(s)\n", g_pass, g_fail);
	return (g_fail != 0);
}
