#include "test_framework.h"

static void	check_substr(const char *s, unsigned int start, size_t len, const char *exp)
{
	char	*r = ft_substr(s, start, len);

	T_ASSERT(r != NULL, "ft_substr returned non-NULL");
	if (r)
	{
		T_ASSERT(strcmp(r, exp) == 0, "ft_substr content matches expected");
		T_ASSERT(exact_alloc_matches(r, strlen(exp) + 1), "ft_substr allocates exact needed bytes");
		free(r);
	}
}

static void	test_ft_substr(void)
{
	BEGIN_FN_TEST("ft_substr");
	T_ASSERT(ft_substr(NULL, 0, 5) == NULL, "s=NULL -> NULL");
	check_substr("hello, world", 0, 5, "hello");
	check_substr("hello, world", 7, 5, "world");
	check_substr("hello", 0, 42000, "hello");
	check_substr("hello", 5, 3, "");
	check_substr("hello", 100, 50, "");
	check_substr("hello", UINT_MAX, SIZE_MAX, "");
	check_substr("hello", 0, 0, "");
	check_substr("", 0, 10, "");
	check_substr("0123456789", 9, 10, "9");
	check_substr("BONJOUR LES HARICOTS !", 8, 14, "LES HARICOTS !");
	END_FN_TEST("ft_substr");
}

static void	test_ft_strjoin(void)
{
	char	*r;

	BEGIN_FN_TEST("ft_strjoin");
	T_ASSERT(ft_strjoin(NULL, "abc") == NULL, "s1=NULL -> NULL");
	T_ASSERT(ft_strjoin("abc", NULL) == NULL, "s2=NULL -> NULL");
	T_ASSERT(ft_strjoin(NULL, NULL) == NULL, "both NULL -> NULL");

	r = ft_strjoin("", "");
	T_ASSERT(r && strcmp(r, "") == 0 && exact_alloc_matches(r, 1), "\"\" + \"\"");
	free(r);

	r = ft_strjoin("42", "");
	T_ASSERT(r && strcmp(r, "42") == 0 && exact_alloc_matches(r, 3), "\"42\" + \"\"");
	free(r);

	r = ft_strjoin("", "Nice");
	T_ASSERT(r && strcmp(r, "Nice") == 0 && exact_alloc_matches(r, 5), "\"\" + \"Nice\"");
	free(r);

	r = ft_strjoin("Ecole ", "42!");
	T_ASSERT(r && strcmp(r, "Ecole 42!") == 0 && exact_alloc_matches(r, 10), "\"Ecole \" + \"42!\"");
	free(r);
	END_FN_TEST("ft_strjoin");
}

static void	check_trim(const char *s1, const char *set, const char *exp)
{
	char	*r = ft_strtrim(s1, set);

	T_ASSERT(r != NULL, "ft_strtrim non-NULL");
	if (r)
	{
		T_ASSERT(strcmp(r, exp) == 0, "ft_strtrim content");
		T_ASSERT(exact_alloc_matches(r, strlen(exp) + 1), "ft_strtrim exact allocation size");
		free(r);
	}
}

static void	test_ft_strtrim(void)
{
	BEGIN_FN_TEST("ft_strtrim");
	T_ASSERT(ft_strtrim(NULL, " ") == NULL, "s1=NULL -> NULL");
	T_ASSERT(ft_strtrim("abc", NULL) == NULL, "set=NULL -> NULL");
	T_ASSERT(ft_strtrim(NULL, NULL) == NULL, "both NULL -> NULL");
	check_trim("   hello   ", " ", "hello");
	check_trim("xxxhelloxxx", "x", "hello");
	check_trim("   xxxtripouille   xxx", " x", "tripouille");
	check_trim("   xxx   xxx", " x", "");
	check_trim("", "123", "");
	check_trim("123", "", "123");
	check_trim("", "", "");
	check_trim("abcdba", "acb", "d");
	check_trim("hxelloh", "h", "xello");
	END_FN_TEST("ft_strtrim");
}

static void	check_split(const char *s, char c, const char **exp, size_t count)
{
	char	**arr = ft_split(s, c);

	T_ASSERT(arr != NULL, "ft_split non-NULL");
	if (!arr)
		return ;
	T_ASSERT(exact_alloc_matches(arr, sizeof(char *) * (count + 1)), "ft_split exact array size");
	for (size_t i = 0; i < count; i++)
	{
		T_ASSERT(arr[i] != NULL && strcmp(arr[i], exp[i]) == 0, "ft_split word matches");
		if (arr[i])
			T_ASSERT(exact_alloc_matches(arr[i], strlen(exp[i]) + 1), "ft_split exact word size");
		free(arr[i]);
	}
	T_ASSERT(arr[count] == NULL, "ft_split NULL-terminated");
	free(arr);
}

static void	test_ft_split(void)
{
	const char	*w2[] = {"hello", "world"};
	const char	*w1[] = {"hello"};
	const char	*w3[] = {"lorem", "ipsum", "dolor"};
	const char	*w6[] = {"1", "2", "3", "4", "5", "42"};
	const char	*w_whole[] = {"hello world"};

	BEGIN_FN_TEST("ft_split");
	T_ASSERT(ft_split(NULL, ' ') == NULL, "s=NULL -> NULL");
	check_split("hello world", ' ', w2, 2);
	check_split("   hello    world   ", ' ', w2, 2);
	check_split("", ' ', NULL, 0);
	check_split("      ", ' ', NULL, 0);
	check_split("hello", ' ', w1, 1);
	check_split(" hello ", ' ', w1, 1);
	check_split("--1-2--3---4----5-----42--", '-', w6, 6);
	check_split("hello world", '\0', w_whole, 1);
	check_split("", '\0', NULL, 0);
	check_split("lorem\xffipsum\xff\xff" "dolor\xff", '\xff', w3, 3);
	check_split("|||||||", '|', NULL, 0);
	END_FN_TEST("ft_split");
}

static void	check_itoa_val(int n)
{
	char	expected[32];
	char	*got;

	snprintf(expected, sizeof(expected), "%d", n);
	got = ft_itoa(n);
	T_ASSERT(got != NULL, "ft_itoa non-NULL");
	if (got)
	{
		T_ASSERT(strcmp(got, expected) == 0, expected);
		T_ASSERT(exact_alloc_matches(got, strlen(expected) + 1), "ft_itoa exact alloc size");
		free(got);
	}
}

static void	test_ft_itoa(void)
{
	const int	vals[] = {
		0, 1, -1, 9, -9, 10, -10, 42, -42, 99, -99, 100, -100, 105, -105,
		999, 1000, -1000, 123456, -654321, INT_MAX, INT_MIN, INT_MAX - 1, INT_MIN + 1
	};

	BEGIN_FN_TEST("ft_itoa");
	for (size_t i = 0; i < sizeof(vals) / sizeof(vals[0]); i++)
		check_itoa_val(vals[i]);
	END_FN_TEST("ft_itoa");
}

static char	mapi_fn(unsigned int idx, char c)
{
	return ((char)(c + (idx % 5)));
}

static void	test_ft_strmapi(void)
{
	char	*r;

	BEGIN_FN_TEST("ft_strmapi");
	T_ASSERT(ft_strmapi(NULL, mapi_fn) == NULL, "s=NULL -> NULL");
	T_ASSERT(ft_strmapi("abc", NULL) == NULL, "f=NULL -> NULL");
	T_ASSERT(ft_strmapi(NULL, NULL) == NULL, "both NULL -> NULL");

	r = ft_strmapi("", mapi_fn);
	T_ASSERT(r && strcmp(r, "") == 0 && exact_alloc_matches(r, 1), "empty string");
	free(r);

	r = ft_strmapi("aaaaaa", mapi_fn);
	T_ASSERT(r && strcmp(r, "abcdea") == 0 && exact_alloc_matches(r, 7), "index-aware transform");
	free(r);
	END_FN_TEST("ft_strmapi");
}

static void	iteri_fn(unsigned int idx, char *c)
{
	*c = (char)(*c + (idx % 4));
}

static void	test_ft_striteri(void)
{
	char	buf[] = "aaaaaa";
	char	empty[] = "";

	BEGIN_FN_TEST("ft_striteri");
	ft_striteri(NULL, iteri_fn);
	ft_striteri(buf, NULL);
	T_ASSERT(strcmp(buf, "aaaaaa") == 0, "NULL guards do not touch string");

	ft_striteri(empty, iteri_fn);
	T_ASSERT(strcmp(empty, "") == 0, "empty string untouched");

	ft_striteri(buf, iteri_fn);
	T_ASSERT(strcmp(buf, "abcdab") == 0, "in-place index-aware modification");
	END_FN_TEST("ft_striteri");
}

static void	test_ft_putchar_fd(void)
{
	int				pfd[2];
	unsigned char	out[8];

	BEGIN_FN_TEST("ft_putchar_fd");
	T_ASSERT(pipe(pfd) == 0, "pipe created");
	ft_putchar_fd('A', pfd[1]);
	ft_putchar_fd('\0', pfd[1]);
	ft_putchar_fd('\n', pfd[1]);
	ft_putchar_fd((char)0xFF, pfd[1]);
	close(pfd[1]);
	ssize_t n = read(pfd[0], out, sizeof(out));
	close(pfd[0]);
	T_ASSERT(n == 4, "wrote exact 4 bytes");
	T_ASSERT(out[0] == 'A' && out[1] == '\0' && out[2] == '\n' && out[3] == 0xFF,
		"exact byte stream incl. NUL and 0xFF");
	END_FN_TEST("ft_putchar_fd");
}

static void	test_ft_putstr_fd(void)
{
	int		pfd[2];
	char	out[128];

	BEGIN_FN_TEST("ft_putstr_fd");
	T_ASSERT(pipe(pfd) == 0, "pipe created");
	ft_putstr_fd(NULL, pfd[1]);
	ft_putstr_fd("", pfd[1]);
	ft_putstr_fd("Bonjour ", pfd[1]);
	ft_putstr_fd("42\nNice!", pfd[1]);
	close(pfd[1]);
	ssize_t n = read(pfd[0], out, sizeof(out) - 1);
	close(pfd[0]);
	out[n >= 0 ? n : 0] = '\0';
	T_ASSERT(n == 16 && strcmp(out, "Bonjour 42\nNice!") == 0, "exact string output + NULL/empty safe");
	END_FN_TEST("ft_putstr_fd");
}

static void	test_ft_putendl_fd(void)
{
	int		pfd[2];
	char	out[128];

	BEGIN_FN_TEST("ft_putendl_fd");
	T_ASSERT(pipe(pfd) == 0, "pipe created");
	ft_putendl_fd(NULL, pfd[1]);
	ft_putendl_fd("", pfd[1]);
	ft_putendl_fd("ligne1", pfd[1]);
	ft_putendl_fd("ligne2", pfd[1]);
	close(pfd[1]);
	ssize_t n = read(pfd[0], out, sizeof(out) - 1);
	close(pfd[0]);
	out[n >= 0 ? n : 0] = '\0';
	T_ASSERT(n == 15 && strcmp(out, "\nligne1\nligne2\n") == 0, "writes newline after each string, NULL safe");
	END_FN_TEST("ft_putendl_fd");
}

static void	check_putnbr(int val)
{
	int		pfd[2];
	char	got[64];
	char	exp[64];

	snprintf(exp, sizeof(exp), "%d", val);
	if (pipe(pfd) != 0)
	{
		T_ASSERT(0, "pipe failed");
		return ;
	}
	ft_putnbr_fd(val, pfd[1]);
	close(pfd[1]);
	ssize_t n = read(pfd[0], got, sizeof(got) - 1);
	close(pfd[0]);
	got[n >= 0 ? n : 0] = '\0';
	T_ASSERT(strcmp(got, exp) == 0, exp);
}

static void	test_ft_putnbr_fd(void)
{
	const int	vals[] = {
		0, 1, -1, 9, -9, 10, -10, 42, -42, 100, -100, 105, -105,
		1000, 10000000, INT_MAX, INT_MIN, INT_MAX - 1, INT_MIN + 1
	};

	BEGIN_FN_TEST("ft_putnbr_fd");
	for (size_t i = 0; i < sizeof(vals) / sizeof(vals[0]); i++)
		check_putnbr(vals[i]);
	END_FN_TEST("ft_putnbr_fd");
}

void	run_part2_tests(void)
{
	printf(CLR_BOLD CLR_CYAN "\n--- Partie 2 : Fonctions supplementaires (11 fonctions) ---" CLR_RESET "\n");
	test_ft_substr();
	test_ft_strjoin();
	test_ft_strtrim();
	test_ft_split();
	test_ft_itoa();
	test_ft_strmapi();
	test_ft_striteri();
	test_ft_putchar_fd();
	test_ft_putstr_fd();
	test_ft_putendl_fd();
	test_ft_putnbr_fd();
}
