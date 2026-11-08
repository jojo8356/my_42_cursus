/* Tests complets du jour C02 (chaines). */
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>

char			*ft_strcpy(char *dest, char *src);
char			*ft_strncpy(char *dest, char *src, unsigned int n);
int				ft_str_is_alpha(char *str);
int				ft_str_is_numeric(char *str);
int				ft_str_is_lowercase(char *str);
int				ft_str_is_uppercase(char *str);
int				ft_str_is_printable(char *str);
char			*ft_strupcase(char *str);
char			*ft_strlowcase(char *str);
char			*ft_strcapitalize(char *str);
unsigned int	ft_strlcpy(char *dest, char *src, unsigned int size);
void			ft_putstr_non_printable(char *str);
void			*ft_print_memory(void *addr, unsigned int size);

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
	fd = open("/tmp/c02_cap.txt", O_RDWR | O_CREAT | O_TRUNC, 0644);
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
	fd = open("/tmp/c02_cap.txt", O_RDONLY);
	n = read(fd, buf, sizeof(buf) - 1);
	if (n < 0)
		n = 0;
	buf[n] = '\0';
	close(fd);
	return (buf);
}

static void	test_print_memory(void)
{
	char	*out;
	char	data[40];

	memset(data, 'A', 40);
	{
		void	*ret;

		start_capture();
		ret = ft_print_memory(data, 0);
		out = end_capture();
		check(ret == data, "ex12 retourne addr");
	}
	check(strcmp(out, "") == 0, "ex12 size 0 -> rien");
	start_capture();
	ft_print_memory(data, 3);
	out = end_capture();
	check(strncmp(out, "00000000: 4141 41", 17) == 0
		&& strstr(out, "AAA") != NULL, "ex12 3 octets ligne unique");
	check(strchr(out, '\n') != NULL
		&& *(strchr(out, '\n') + 1) == '\0', "ex12 une seule ligne");
	data[0] = 'B';
	data[1] = 'o';
	data[2] = 'n';
	data[3] = 'j';
	data[4] = 27;
	data[5] = '.';
	start_capture();
	ft_print_memory(data, 6);
	out = end_capture();
	check(strstr(out, "426f 6e6a 1b2e") != NULL, "ex12 hex des 6 octets");
	check(strstr(out, "Bonj") != NULL && strstr(out, "..") != NULL,
		"ex12 ASCII avec point pour non-imprimable");
	start_capture();
	ft_print_memory(data, 20);
	out = end_capture();
	check(strstr(out, "00000010:") != NULL, "ex12 deux lignes pour 20 octets");
}

int	main(void)
{
	char	d[128];
	char	*r;
	char	*out;
	int		ret;

	printf("== C02 ==\n");
	r = ft_strcpy(d, "Hello");
	check(r == d && strcmp(d, "Hello") == 0, "ex00 strcpy copie Hello");
	ft_strcpy(d, "");
	check(strcmp(d, "") == 0, "ex00 strcpy chaine vide");
	ft_strcpy(d, "abcd");
	check(strcmp(d, "abcd") == 0, "ex00 strcpy ecrase la precedente");
	memset(d, 'X', 8);
	ft_strncpy(d, "abc", 2);
	check(d[0] == 'a' && d[1] == 'b' && d[2] == 'X', "ex01 strncpy n=2");
	memset(d, 'X', 8);
	ft_strncpy(d, "ab", 5);
	check(d[2] == '\0' && d[3] == '\0' && d[4] == '\0',
		"ex01 strncpy complete par \\0");
	memset(d, 'X', 8);
	ft_strncpy(d, "abc", 0);
	check(d[0] == 'X', "ex01 strncpy n=0 ne touche pas");
	check(ft_str_is_alpha("AbCdEfZ") == 1, "ex02 lettres -> 1");
	check(ft_str_is_alpha("abc1") == 0, "ex02 chiffre -> 0");
	check(ft_str_is_alpha("") == 1, "ex02 vide -> 1");
	check(ft_str_is_alpha("abc def") == 0, "ex02 espace -> 0");
	check(ft_str_is_numeric("0123456789") == 1, "ex03 chiffres -> 1");
	check(ft_str_is_numeric("42a") == 0, "ex03 lettre -> 0");
	check(ft_str_is_numeric("") == 1, "ex03 vide -> 1");
	check(ft_str_is_lowercase("abcxyz") == 1, "ex04 minuscules -> 1");
	check(ft_str_is_lowercase("abcX") == 0, "ex04 majuscule -> 0");
	check(ft_str_is_lowercase("") == 1, "ex04 vide -> 1");
	check(ft_str_is_uppercase("ABCXYZ") == 1, "ex05 majuscules -> 1");
	check(ft_str_is_uppercase("ABCx") == 0, "ex05 minuscule -> 0");
	check(ft_str_is_uppercase("") == 1, "ex05 vide -> 1");
	check(ft_str_is_printable("Hello World! 42") == 1, "ex06 imprimable");
	check(ft_str_is_printable("a\nb") == 0, "ex06 saut de ligne -> 0");
	check(ft_str_is_printable("") == 1, "ex06 vide -> 1");
	strcpy(d, "Hello World 42");
	check(ft_strupcase(d) == d && strcmp(d, "HELLO WORLD 42") == 0,
		"ex07 strupcase");
	strcpy(d, "abc");
	ft_strupcase(d);
	check(strcmp(d, "ABC") == 0, "ex07 strupcase simple");
	strcpy(d, "HELLO World 42");
	check(ft_strlowcase(d) == d && strcmp(d, "hello world 42") == 0,
		"ex08 strlowcase");
	strcpy(d, "salut, comment tu vas ? 42mots quarante-deux; "
		"cinquante+et+un");
	ft_strcapitalize(d);
	check(strcmp(d, "Salut, Comment Tu Vas ? 42mots Quarante-Deux; "
			"Cinquante+Et+Un") == 0, "ex09 strcapitalize phrase complete");
	strcpy(d, "a");
	ft_strcapitalize(d);
	check(strcmp(d, "A") == 0, "ex09 strcapitalize mot unique");
	strcpy(d, "");
	ft_strcapitalize(d);
	check(strcmp(d, "") == 0, "ex09 strcapitalize vide");
	memset(d, 'X', 8);
	ret = ft_strlcpy(d, "abc", 8);
	check(ret == 3 && strcmp(d, "abc") == 0, "ex10 strlcpy normal");
	memset(d, 'X', 8);
	ret = ft_strlcpy(d, "abcdef", 3);
	check(ret == 6 && d[2] == '\0' && d[3] == 'X', "ex10 strlcpy tronque");
	memset(d, 'X', 8);
	ret = ft_strlcpy(d, "abc", 0);
	check(ret == 3 && d[0] == 'X', "ex10 strlcpy size 0");
	start_capture();
	ft_putstr_non_printable("Coucou\ntu vas bien ?");
	out = end_capture();
	check(strcmp(out, "Coucou\\0atu vas bien ?") == 0,
		"ex11 non imprimable -> \\0a");
	start_capture();
	ft_putstr_non_printable("");
	out = end_capture();
	check(strcmp(out, "") == 0, "ex11 chaine vide");
	start_capture();
	ft_putstr_non_printable("\x7f\x01");
	out = end_capture();
	check(strcmp(out, "\\7f\\01") == 0, "ex11 \x7f et \x01");
	test_print_memory();
	printf("C02 : %d reussi(s), %d echec(s)\n", g_pass, g_fail);
	return (g_fail != 0);
}
