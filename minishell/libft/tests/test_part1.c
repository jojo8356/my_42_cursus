#include "test_framework.h"

/* Reference BSD implementations for exact differential testing */
static size_t	ref_strlcpy(char *dst, const char *src, size_t size)
{
	size_t	len = strlen(src);
	if (size > 0)
	{
		size_t copy = (len >= size) ? size - 1 : len;
		memcpy(dst, src, copy);
		dst[copy] = '\0';
	}
	return (len);
}

static size_t	ref_strlcat(char *dst, const char *src, size_t size)
{
	size_t	src_len = strlen(src);
	size_t	dst_len = 0;

	while (dst_len < size && dst[dst_len] != '\0')
		dst_len++;
	if (dst_len == size)
		return (size + src_len);
	size_t room = size - dst_len - 1;
	size_t copy = (src_len > room) ? room : src_len;
	memcpy(dst + dst_len, src, copy);
	dst[dst_len + copy] = '\0';
	return (dst_len + src_len);
}

static char	*ref_strnstr(const char *big, const char *little, size_t len)
{
	size_t	l_len;

	if (*little == '\0')
		return ((char *)big);
	if (len == 0)
		return (NULL);
	l_len = strlen(little);
	while (*big && len >= l_len)
	{
		if (memcmp(big, little, l_len) == 0)
			return ((char *)big);
		big++;
		len--;
	}
	return (NULL);
}

static void	test_ft_isalpha(void)
{
	BEGIN_FN_TEST("ft_isalpha");
	T_ASSERT(ft_isalpha(EOF) == 0, "EOF -> 0");
	for (int c = -512; c <= 512; c++)
	{
		int got = ft_isalpha(c);
		int exp = ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')) ? 1 : 0;
		T_ASSERT(got == exp, "sweep -512..512 exact 1/0 (v19.3)");
		if (c >= 0 && c <= 255)
			T_ASSERT(got == (isalpha(c) ? 1 : 0), "matches <ctype.h> isalpha");
	}
	T_ASSERT(ft_isalpha(INT_MIN) == 0, "INT_MIN -> 0");
	T_ASSERT(ft_isalpha(INT_MAX) == 0, "INT_MAX -> 0");
	END_FN_TEST("ft_isalpha");
}

static void	test_ft_isdigit(void)
{
	BEGIN_FN_TEST("ft_isdigit");
	T_ASSERT(ft_isdigit(EOF) == 0, "EOF -> 0");
	for (int c = -512; c <= 512; c++)
	{
		int got = ft_isdigit(c);
		int exp = (c >= '0' && c <= '9') ? 1 : 0;
		T_ASSERT(got == exp, "sweep -512..512 exact 1/0 (v19.3)");
		if (c >= 0 && c <= 255)
			T_ASSERT(got == (isdigit(c) ? 1 : 0), "matches <ctype.h> isdigit");
	}
	T_ASSERT(ft_isdigit(INT_MIN) == 0, "INT_MIN -> 0");
	T_ASSERT(ft_isdigit(INT_MAX) == 0, "INT_MAX -> 0");
	END_FN_TEST("ft_isdigit");
}

static void	test_ft_isalnum(void)
{
	BEGIN_FN_TEST("ft_isalnum");
	T_ASSERT(ft_isalnum(EOF) == 0, "EOF -> 0");
	for (int c = -512; c <= 512; c++)
	{
		int got = ft_isalnum(c);
		int exp = ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')
			|| (c >= '0' && c <= '9')) ? 1 : 0;
		T_ASSERT(got == exp, "sweep -512..512 exact 1/0 (v19.3)");
		if (c >= 0 && c <= 255)
			T_ASSERT(got == (isalnum(c) ? 1 : 0), "matches <ctype.h> isalnum");
	}
	T_ASSERT(ft_isalnum(INT_MIN) == 0, "INT_MIN -> 0");
	T_ASSERT(ft_isalnum(INT_MAX) == 0, "INT_MAX -> 0");
	END_FN_TEST("ft_isalnum");
}

static void	test_ft_isascii(void)
{
	BEGIN_FN_TEST("ft_isascii");
	T_ASSERT(ft_isascii(EOF) == 0, "EOF (-1) -> 0");
	for (int c = -512; c <= 512; c++)
	{
		int got = ft_isascii(c);
		int exp = (c >= 0 && c <= 127) ? 1 : 0;
		T_ASSERT(got == exp, "sweep -512..512 exact 1/0 (v19.3)");
		T_ASSERT(got == (isascii(c) ? 1 : 0), "matches <ctype.h> isascii");
	}
	T_ASSERT(ft_isascii(INT_MIN) == 0, "INT_MIN -> 0");
	T_ASSERT(ft_isascii(INT_MAX) == 0, "INT_MAX -> 0");
	END_FN_TEST("ft_isascii");
}

static void	test_ft_isprint(void)
{
	BEGIN_FN_TEST("ft_isprint");
	T_ASSERT(ft_isprint(EOF) == 0, "EOF -> 0");
	for (int c = -512; c <= 512; c++)
	{
		int got = ft_isprint(c);
		int exp = (c >= 32 && c <= 126) ? 1 : 0;
		T_ASSERT(got == exp, "sweep -512..512 exact 1/0 (v19.3)");
		if (c >= 0 && c <= 255)
			T_ASSERT(got == (isprint(c) ? 1 : 0), "matches <ctype.h> isprint");
	}
	T_ASSERT(ft_isprint(31) == 0 && ft_isprint(32) == 1, "boundary 31/32");
	T_ASSERT(ft_isprint(126) == 1 && ft_isprint(127) == 0, "boundary 126/127");
	END_FN_TEST("ft_isprint");
}

static void	test_ft_strlen(void)
{
	char	big[4097];

	BEGIN_FN_TEST("ft_strlen");
	T_ASSERT(ft_strlen("") == 0, "empty string");
	T_ASSERT(ft_strlen("a") == 1, "1 char");
	T_ASSERT(ft_strlen("42 Nice") == 7, "normal string");
	T_ASSERT(ft_strlen("abc\0def") == 3, "stops at embedded NUL");
	T_ASSERT(ft_strlen("\xff\xfe\x80") == 3, "extended unsigned bytes");
	memset(big, 'x', 4096);
	big[4096] = '\0';
	T_ASSERT(ft_strlen(big) == 4096, "4096-byte string");
	END_FN_TEST("ft_strlen");
}

static void	test_ft_memset(void)
{
	unsigned char	buf1[64];
	unsigned char	buf2[64];

	BEGIN_FN_TEST("ft_memset");
	memset(buf1, 0xA5, sizeof(buf1));
	memset(buf2, 0xA5, sizeof(buf2));
	T_ASSERT(ft_memset(buf1 + 8, 'Z', 32) == buf1 + 8, "returns s pointer");
	memset(buf2 + 8, 'Z', 32);
	T_ASSERT(memcmp(buf1, buf2, sizeof(buf1)) == 0, "exact fill + canary boundaries intact");

	T_ASSERT(ft_memset(buf1 + 10, 0, 0) == buf1 + 10, "n=0 returns s");
	T_ASSERT(memcmp(buf1, buf2, sizeof(buf1)) == 0, "n=0 leaves buffer untouched");

	ft_memset(buf1 + 4, 256 + 0x7F, 16);
	memset(buf2 + 4, 256 + 0x7F, 16);
	T_ASSERT(memcmp(buf1, buf2, sizeof(buf1)) == 0, "c > 255 truncated to unsigned char");

	ft_memset(buf1 + 4, -42, 16);
	memset(buf2 + 4, -42, 16);
	T_ASSERT(memcmp(buf1, buf2, sizeof(buf1)) == 0, "negative c converted to unsigned char");
	END_FN_TEST("ft_memset");
}

static void	test_ft_bzero(void)
{
	unsigned char	buf1[64];
	unsigned char	buf2[64];

	BEGIN_FN_TEST("ft_bzero");
	memset(buf1, 0xFF, sizeof(buf1));
	memset(buf2, 0xFF, sizeof(buf2));
	ft_bzero(buf1 + 10, 0);
	T_ASSERT(memcmp(buf1, buf2, sizeof(buf1)) == 0, "n=0 leaves all bytes 0xFF");

	ft_bzero(buf1 + 10, 25);
	memset(buf2 + 10, 0, 25);
	T_ASSERT(memcmp(buf1, buf2, sizeof(buf1)) == 0, "zeros exact 25 bytes, canaries intact");
	T_ASSERT(buf1[9] == 0xFF && buf1[35] == 0xFF, "exact boundary guard bytes");
	END_FN_TEST("ft_bzero");
}

static void	test_ft_memcpy(void)
{
	unsigned char	src[64];
	unsigned char	d1[80];
	unsigned char	d2[80];

	BEGIN_FN_TEST("ft_memcpy");
	for (size_t i = 0; i < sizeof(src); i++)
		src[i] = (unsigned char)(i * 7 + 3);
	memset(d1, 0x5A, sizeof(d1));
	memset(d2, 0x5A, sizeof(d2));

	T_ASSERT(ft_memcpy(d1 + 8, src, 64) == d1 + 8, "returns dest");
	memcpy(d2 + 8, src, 64);
	T_ASSERT(memcmp(d1, d2, sizeof(d1)) == 0, "64 binary bytes incl. 0x00 & >=0x80 + canaries");

	T_ASSERT(ft_memcpy(d1, "42\0hidden", 9) == d1, "embedded NUL return");
	T_ASSERT(memcmp(d1, "42\0hidden", 9) == 0, "copies past embedded NUL");

	memset(d1, 'A', 16);
	T_ASSERT(ft_memcpy(d1, NULL, 0) == d1, "n=0 with src=NULL returns dest safely");
	T_ASSERT(d1[0] == 'A', "n=0 does not modify dest");
	T_ASSERT(ft_memcpy(NULL, "abc", 0) == NULL, "n=0 with dest=NULL returns NULL safely");
	T_ASSERT(ft_memcpy(NULL, NULL, 0) == NULL, "NULL, NULL, 0 -> NULL");
	T_ASSERT(ft_memcpy(NULL, NULL, 5) == NULL, "NULL, NULL, 5 -> NULL (dest == src)");
	END_FN_TEST("ft_memcpy");
}

static void	test_ft_memmove(void)
{
	char	b1[64];
	char	b2[64];

	BEGIN_FN_TEST("ft_memmove");
	/* Overlap dst > src (backward copy required) */
	strcpy(b1, "0123456789abcdefXXXXXXXX");
	strcpy(b2, "0123456789abcdefXXXXXXXX");
	T_ASSERT(ft_memmove(b1 + 4, b1, 16) == b1 + 4, "overlap dst > src returns dst");
	memmove(b2 + 4, b2, 16);
	T_ASSERT(memcmp(b1, b2, 25) == 0, "overlap dst > src matches memmove");

	/* Overlap dst < src (forward copy required) */
	strcpy(b1, "0123456789abcdefXXXXXXXX");
	strcpy(b2, "0123456789abcdefXXXXXXXX");
	T_ASSERT(ft_memmove(b1, b1 + 4, 16) == b1, "overlap dst < src returns dst");
	memmove(b2, b2 + 4, 16);
	T_ASSERT(memcmp(b1, b2, 25) == 0, "overlap dst < src matches memmove");

	/* Overlap by 1 byte */
	strcpy(b1, "abcdefghij");
	strcpy(b2, "abcdefghij");
	ft_memmove(b1 + 1, b1, 8);
	memmove(b2 + 1, b2, 8);
	T_ASSERT(memcmp(b1, b2, 11) == 0, "1-byte shift right overlap");

	/* Identical pointers & n=0 & NULL/NULL */
	T_ASSERT(ft_memmove(b1, b1, 10) == b1, "dst == src returns dst");
	T_ASSERT(memcmp(b1, b2, 11) == 0, "dst == src preserves content");
	T_ASSERT(ft_memmove(b1, NULL, 0) == b1, "n=0 with src=NULL safe");
	T_ASSERT(ft_memmove(NULL, b1, 0) == NULL, "n=0 with dst=NULL safe");
	T_ASSERT(ft_memmove(NULL, NULL, 5) == NULL, "NULL, NULL, 5 -> NULL");
	END_FN_TEST("ft_memmove");
}

static void	test_ft_strlcpy(void)
{
	char		d1[32];
	char		d2[32];
	const char	*srcs[] = {"", "a", "hello", "0123456789abcdef"};

	BEGIN_FN_TEST("ft_strlcpy");
	T_ASSERT(ft_strlcpy(NULL, "hello", 0) == 5, "size=0 with dst=NULL returns strlen(src)");
	T_ASSERT(ft_strlcpy(NULL, "", 0) == 0, "size=0 with empty src and dst=NULL");

	for (size_t s = 0; s < 4; s++)
	{
		for (size_t sz = 0; sz <= 20; sz++)
		{
			memset(d1, 0xAA, sizeof(d1));
			memset(d2, 0xAA, sizeof(d2));
			size_t r1 = ft_strlcpy(d1 + 2, srcs[s], sz);
			size_t r2 = ref_strlcpy(d2 + 2, srcs[s], sz);
			T_ASSERT(r1 == r2, "return value matches BSD strlcpy");
			T_ASSERT(memcmp(d1, d2, sizeof(d1)) == 0, "buffer + canaries match BSD strlcpy");
		}
	}
	END_FN_TEST("ft_strlcpy");
}

static void	test_ft_strlcat(void)
{
	char		d1[48];
	char		d2[48];
	const char	*inits[] = {"", "A", "Hello", "0123456789"};
	const char	*srcs[] = {"", "!", " World", "abcdefghijkl"};

	BEGIN_FN_TEST("ft_strlcat");
	T_ASSERT(ft_strlcat(NULL, "hello", 0) == 5, "size=0 with dst=NULL does not dereference dst");
	T_ASSERT(ft_strlcat(NULL, "", 0) == 0, "size=0 with dst=NULL and empty src");

	/* Unterminated dst within first `size` bytes: must stop at `size` without reading further */
	memset(d1, 'X', sizeof(d1));
	T_ASSERT(ft_strlcat(d1, "abc", 5) == 8, "unterminated dst of size=5 returns 5 + 3");
	T_ASSERT(d1[5] == 'X', "unterminated dst is not modified");

	for (size_t i = 0; i < 4; i++)
	{
		for (size_t s = 0; s < 4; s++)
		{
			for (size_t sz = 0; sz <= 28; sz++)
			{
				memset(d1, 0xCC, sizeof(d1));
				memset(d2, 0xCC, sizeof(d2));
				strcpy(d1 + 2, inits[i]);
				strcpy(d2 + 2, inits[i]);
				size_t r1 = ft_strlcat(d1 + 2, srcs[s], sz);
				size_t r2 = ref_strlcat(d2 + 2, srcs[s], sz);
				T_ASSERT(r1 == r2, "return matches BSD strlcat");
				T_ASSERT(memcmp(d1, d2, sizeof(d1)) == 0, "buffer + canaries match BSD strlcat");
			}
		}
	}
	END_FN_TEST("ft_strlcat");
}

static void	test_ft_toupper(void)
{
	BEGIN_FN_TEST("ft_toupper");
	T_ASSERT(ft_toupper(EOF) == EOF, "EOF unchanged");
	for (int c = -512; c <= 512; c++)
	{
		int exp = (c >= 'a' && c <= 'z') ? (c - 32) : c;
		T_ASSERT(ft_toupper(c) == exp, "sweep -512..512");
		if (c >= 0 && c <= 255)
			T_ASSERT(ft_toupper(c) == toupper(c), "matches <ctype.h> toupper");
	}
	END_FN_TEST("ft_toupper");
}

static void	test_ft_tolower(void)
{
	BEGIN_FN_TEST("ft_tolower");
	T_ASSERT(ft_tolower(EOF) == EOF, "EOF unchanged");
	for (int c = -512; c <= 512; c++)
	{
		int exp = (c >= 'A' && c <= 'Z') ? (c + 32) : c;
		T_ASSERT(ft_tolower(c) == exp, "sweep -512..512");
		if (c >= 0 && c <= 255)
			T_ASSERT(ft_tolower(c) == tolower(c), "matches <ctype.h> tolower");
	}
	END_FN_TEST("ft_tolower");
}

static void	test_ft_strchr(void)
{
	const char	*s = "tripouille\xff" "42";
	const char	*empty = "";

	BEGIN_FN_TEST("ft_strchr");
	T_ASSERT(ft_strchr(s, 't') == strchr(s, 't'), "first char");
	T_ASSERT(ft_strchr(s, 'i') == strchr(s, 'i'), "first of duplicate 'i'");
	T_ASSERT(ft_strchr(s, '2') == strchr(s, '2'), "last char before NUL");
	T_ASSERT(ft_strchr(s, '\0') == strchr(s, '\0'), "terminating NUL found");
	T_ASSERT(ft_strchr(s, 'z') == NULL, "missing char -> NULL");
	T_ASSERT(ft_strchr(s, 't' + 256) == strchr(s, 't' + 256), "c + 256 cast to char");
	T_ASSERT(ft_strchr(s, 0xFF) == strchr(s, 0xFF), "extended byte 0xFF");
	T_ASSERT(ft_strchr(s, -1) == strchr(s, -1), "signed -1 (0xFF)");
	T_ASSERT(ft_strchr(empty, '\0') == empty, "empty string NUL");
	T_ASSERT(ft_strchr(empty, 'a') == NULL, "empty string non-NUL");
	END_FN_TEST("ft_strchr");
}

static void	test_ft_strrchr(void)
{
	const char	*s = "abracadabra\xfez";
	const char	*empty = "";

	BEGIN_FN_TEST("ft_strrchr");
	T_ASSERT(ft_strrchr(s, 'a') == strrchr(s, 'a'), "last occurrence of 'a'");
	T_ASSERT(ft_strrchr(s, 'b') == strrchr(s, 'b'), "last occurrence of 'b'");
	T_ASSERT(ft_strrchr(s, 'z') == strrchr(s, 'z'), "final character");
	T_ASSERT(ft_strrchr("xhello", 'x') == strrchr("xhello", 'x'), "only at index 0");
	T_ASSERT(ft_strrchr(s, '\0') == strrchr(s, '\0'), "terminating NUL");
	T_ASSERT(ft_strrchr(s, 'Q') == NULL, "missing char -> NULL");
	T_ASSERT(ft_strrchr(s, 'a' + 256) == strrchr(s, 'a' + 256), "c + 256 cast to char");
	T_ASSERT(ft_strrchr(s, 0xFE) == strrchr(s, 0xFE), "extended byte 0xFE");
	T_ASSERT(ft_strrchr(empty, '\0') == empty, "empty string NUL");
	T_ASSERT(ft_strrchr(empty, 'a') == NULL, "empty string non-NUL");
	END_FN_TEST("ft_strrchr");
}

static int	sign_of(int x)
{
	return ((x > 0) - (x < 0));
}

static void	test_ft_strncmp(void)
{
	BEGIN_FN_TEST("ft_strncmp");
	T_ASSERT(ft_strncmp("abc", "xyz", 0) == 0, "n=0 returns 0");
	T_ASSERT(ft_strncmp(NULL, NULL, 0) == 0, "n=0 does not dereference pointers");
	T_ASSERT(sign_of(ft_strncmp("", "", 5)) == sign_of(strncmp("", "", 5)), "both empty");
	T_ASSERT(sign_of(ft_strncmp("abc", "abc", 3)) == 0, "exact match n=3");
	T_ASSERT(sign_of(ft_strncmp("abc", "abd", 2)) == 0, "diff after n");
	T_ASSERT(sign_of(ft_strncmp("abc", "abd", 3)) == sign_of(strncmp("abc", "abd", 3)), "diff at n-1");
	T_ASSERT(sign_of(ft_strncmp("abc\0X", "abc\0Y", 5)) == 0, "stops at NUL");
	T_ASSERT(ft_strncmp("test\200", "test\0", 6) > 0, "unsigned char comparison (\\200 > \\0)");
	T_ASSERT(ft_strncmp("test\0", "test\200", 6) < 0, "unsigned char comparison (\\0 < \\200)");
	T_ASSERT(sign_of(ft_strncmp("\xff", "\x7f", 1)) == sign_of(strncmp("\xff", "\x7f", 1)), "0xFF > 0x7F");
	T_ASSERT(sign_of(ft_strncmp("42", "42", SIZE_MAX)) == 0, "n = SIZE_MAX");
	END_FN_TEST("ft_strncmp");
}

static void	test_ft_memchr(void)
{
	const unsigned char	buf[] = {0, 1, 2, 0x80, 0xFF, 42, 2, 99};

	BEGIN_FN_TEST("ft_memchr");
	T_ASSERT(ft_memchr(NULL, 'a', 0) == NULL, "n=0 with s=NULL safe");
	T_ASSERT(ft_memchr(buf, 0, sizeof(buf)) == memchr(buf, 0, sizeof(buf)), "finds NUL at index 0");
	T_ASSERT(ft_memchr(buf, 2, sizeof(buf)) == memchr(buf, 2, sizeof(buf)), "finds first 2");
	T_ASSERT(ft_memchr(buf, 0xFF, sizeof(buf)) == memchr(buf, 0xFF, sizeof(buf)), "finds 0xFF");
	T_ASSERT(ft_memchr(buf, -1, sizeof(buf)) == memchr(buf, -1, sizeof(buf)), "-1 matches 0xFF");
	T_ASSERT(ft_memchr(buf, 42 + 512, sizeof(buf)) == memchr(buf, 42 + 512, sizeof(buf)), "c + 512");
	T_ASSERT(ft_memchr(buf, 99, 7) == NULL, "byte at index 7 excluded when n=7");
	T_ASSERT(ft_memchr(buf, 99, 8) == buf + 7, "byte at index 7 found when n=8");
	END_FN_TEST("ft_memchr");
}

static void	test_ft_memcmp(void)
{
	const unsigned char	a[] = {'a', '\0', 0x80, 10};
	const unsigned char	b[] = {'a', '\0', 0x7F, 99};
	const unsigned char	c[] = {'a', '\0', 0x80, 10};

	BEGIN_FN_TEST("ft_memcmp");
	T_ASSERT(ft_memcmp(NULL, NULL, 0) == 0, "n=0 safe and returns 0");
	T_ASSERT(ft_memcmp(a, b, 2) == 0, "equal up to n=2 (incl. NUL)");
	T_ASSERT(ft_memcmp(a, b, 3) > 0 && memcmp(a, b, 3) > 0, "compares past NUL as unsigned char (0x80 > 0x7F)");
	T_ASSERT(ft_memcmp(b, a, 3) < 0 && memcmp(b, a, 3) < 0, "0x7F < 0x80");
	T_ASSERT(ft_memcmp(a, c, 4) == 0, "identical 4 binary bytes");
	END_FN_TEST("ft_memcmp");
}

static void	test_ft_strnstr(void)
{
	const char	*hay = "aaabcabcd";

	BEGIN_FN_TEST("ft_strnstr");
	T_ASSERT(ft_strnstr(hay, "", 0) == hay, "empty needle with len=0 returns big");
	T_ASSERT(ft_strnstr(NULL, "", 0) == NULL, "empty needle with big=NULL returns NULL");
	T_ASSERT(ft_strnstr(NULL, "abc", 0) == NULL, "len=0 with big=NULL returns NULL safely");
	T_ASSERT(ft_strnstr("", "a", 5) == NULL, "empty haystack -> NULL");
	T_ASSERT(ft_strnstr("", "", 5) != NULL, "both empty -> big");

	for (size_t len = 0; len <= strlen(hay) + 3; len++)
	{
		T_ASSERT(ft_strnstr(hay, "aaabc", len) == ref_strnstr(hay, "aaabc", len), "overlap prefix aaabc");
		T_ASSERT(ft_strnstr(hay, "abcd", len) == ref_strnstr(hay, "abcd", len), "suffix abcd");
		T_ASSERT(ft_strnstr(hay, "a", len) == ref_strnstr(hay, "a", len), "single char a");
		T_ASSERT(ft_strnstr(hay, "cdX", len) == NULL, "needle longer than haystack");
	}
	T_ASSERT(ft_strnstr("abc\0def", "def", 7) == NULL, "stops at NUL in big");
	END_FN_TEST("ft_strnstr");
}

static void	test_ft_atoi(void)
{
	const char	*cases[] = {
		"0", "-0", "+0", "1", "-1", "+1", "42", "-42", "+42",
		"00000042", "-00042", "   42", " \t\n\v\f\r -42", " \t\n\v\f\r +42",
		"42abc99", "-42-3", "", "   ", "+", "-", "+-42", "-+42", "--42", "++42",
		"+ 42", "- 42", "a42", "\t\n\v\f\r2147483647", "\t\n\v\f\r-2147483648",
		"2147483646", "-2147483647",
		"9223372036854775807", "-9223372036854775808",
		"9999999999999999999999999999", "-9999999999999999999999999999"
	};
	size_t		n = sizeof(cases) / sizeof(cases[0]);

	BEGIN_FN_TEST("ft_atoi");
	for (size_t i = 0; i < n; i++)
		T_ASSERT(ft_atoi(cases[i]) == atoi(cases[i]), cases[i]);
	for (int c = 1; c < 32; c++)
	{
		char s[8] = {(char)c, '7', '\0'};
		T_ASSERT(ft_atoi(s) == atoi(s), "control character whitespace check vs libc atoi");
	}
	END_FN_TEST("ft_atoi");
}

static void	test_ft_calloc(void)
{
	unsigned char	*p;
	unsigned char	zeros[128];

	BEGIN_FN_TEST("ft_calloc");
	memset(zeros, 0, sizeof(zeros));

	p = (unsigned char *)ft_calloc(32, 4);
	T_ASSERT(p != NULL, "ft_calloc(32, 4) != NULL");
	T_ASSERT(memcmp(p, zeros, 128) == 0, "128 bytes zero-initialized");
	T_ASSERT(exact_alloc_matches(p, 128), "allocates exact 128 bytes");
	p[0] = 0xAB;
	p[127] = 0xCD;
	free(p);

	/* Subject v19.3 zero-size rules: must return non-NULL freeable pointer */
	size_t zero_pairs[][2] = {
		{0, 0}, {0, 8}, {8, 0}, {0, SIZE_MAX}, {SIZE_MAX, 0}
	};
	for (size_t i = 0; i < 5; i++)
	{
		void *z = ft_calloc(zero_pairs[i][0], zero_pairs[i][1]);
		T_ASSERT(z != NULL, "zero nmemb/size returns unique freeable pointer (v19.3)");
		free(z);
	}

	/* Multiplication overflow rules: must return NULL */
	T_ASSERT(ft_calloc(SIZE_MAX, 2) == NULL, "SIZE_MAX * 2 overflow -> NULL");
	T_ASSERT(ft_calloc(2, SIZE_MAX) == NULL, "2 * SIZE_MAX overflow -> NULL");
	T_ASSERT(ft_calloc(SIZE_MAX, SIZE_MAX) == NULL, "SIZE_MAX * SIZE_MAX -> NULL");
	T_ASSERT(ft_calloc((size_t)INT_MIN, (size_t)INT_MIN) == NULL, "huge * huge -> NULL");
	T_ASSERT(ft_calloc((size_t)-5, 3) == NULL, "-5 * 3 overflow -> NULL");
	END_FN_TEST("ft_calloc");
}

static void	test_ft_strdup(void)
{
	const char	*cases[] = {"", "a", "42 Nice", "hello\t\nworld\xff!"};

	BEGIN_FN_TEST("ft_strdup");
	for (size_t i = 0; i < 4; i++)
	{
		char *d = ft_strdup(cases[i]);
		T_ASSERT(d != NULL && d != cases[i], "allocates distinct buffer");
		T_ASSERT(strcmp(d, cases[i]) == 0, "content matches");
		T_ASSERT(exact_alloc_matches(d, strlen(cases[i]) + 1), "exact strlen + 1 bytes");
		free(d);
	}
	END_FN_TEST("ft_strdup");
}

void	run_part1_tests(void)
{
	printf(CLR_BOLD CLR_CYAN "--- Partie 1 : Fonctions de la libc (23 fonctions) ---" CLR_RESET "\n");
	test_ft_isalpha();
	test_ft_isdigit();
	test_ft_isalnum();
	test_ft_isascii();
	test_ft_isprint();
	test_ft_strlen();
	test_ft_memset();
	test_ft_bzero();
	test_ft_memcpy();
	test_ft_memmove();
	test_ft_strlcpy();
	test_ft_strlcat();
	test_ft_toupper();
	test_ft_tolower();
	test_ft_strchr();
	test_ft_strrchr();
	test_ft_strncmp();
	test_ft_memchr();
	test_ft_memcmp();
	test_ft_strnstr();
	test_ft_atoi();
	test_ft_calloc();
	test_ft_strdup();
}
