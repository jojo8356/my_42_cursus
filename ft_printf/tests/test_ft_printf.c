/* Differential test: ft_printf vs libc printf on thousands of combinations.
 *
 * Reference output is produced with snprintf (no I/O). ft_printf output is
 * captured by redirecting fd 1 to a single reused temporary file. Both the
 * bytes and the return value are compared.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include "ft_printf.h"

typedef struct s_case
{
	const char	*fmt;
	char		conv;
	long long	val;
	const char	*sval;
	int			w;
	int			p;
	int			mode;
}	t_case;

#include "cases.h"

static int	g_pass;
static int	g_fail;
static int	g_fd = -1;
static int	g_saved = -1;

static void	report(const t_case *c, const char *exp, int re,
		const char *got, int rg)
{
	g_fail++;
	if (g_fail <= 25)
		fprintf(stderr, "FAIL fmt=\"%s\"\n  exp=\"%s\" (ret=%d)\n"
			"  got=\"%s\" (ret=%d)\n", c->fmt, exp, re, got, rg);
}

/* Call a printf-like function with the correctly-typed argument(s). */
static int	call_fn(int (*fn)(const char *, ...), const t_case *c)
{
	if (c->conv == 's')
	{
		if (c->mode == 3)
			return (fn(c->fmt, c->w, c->p, c->sval));
		if (c->mode == 1)
			return (fn(c->fmt, c->w, c->sval));
		if (c->mode == 2)
			return (fn(c->fmt, c->p, c->sval));
		return (fn(c->fmt, c->sval));
	}
	if (c->conv == 'p')
	{
		void *ptr = (void *)(unsigned long long)c->val;
		if (c->mode == 3)
			return (fn(c->fmt, c->w, c->p, ptr));
		if (c->mode == 1)
			return (fn(c->fmt, c->w, ptr));
		if (c->mode == 2)
			return (fn(c->fmt, c->p, ptr));
		return (fn(c->fmt, ptr));
	}
	if (c->mode == 3)
		return (fn(c->fmt, c->w, c->p, (int)c->val));
	if (c->mode == 1)
		return (fn(c->fmt, c->w, (int)c->val));
	if (c->mode == 2)
		return (fn(c->fmt, c->p, (int)c->val));
	return (fn(c->fmt, (int)c->val));
}

/* Reference via snprintf (same argument dispatch, but into a buffer). */
static int	reference(const t_case *c, char *buf, int sz)
{
	if (c->conv == 's')
	{
		if (c->mode == 3)
			return (snprintf(buf, sz, c->fmt, c->w, c->p, c->sval));
		if (c->mode == 1)
			return (snprintf(buf, sz, c->fmt, c->w, c->sval));
		if (c->mode == 2)
			return (snprintf(buf, sz, c->fmt, c->p, c->sval));
		return (snprintf(buf, sz, c->fmt, c->sval));
	}
	if (c->conv == 'p')
	{
		void *ptr = (void *)(unsigned long long)c->val;
		if (c->mode == 3)
			return (snprintf(buf, sz, c->fmt, c->w, c->p, ptr));
		if (c->mode == 1)
			return (snprintf(buf, sz, c->fmt, c->w, ptr));
		if (c->mode == 2)
			return (snprintf(buf, sz, c->fmt, c->p, ptr));
		return (snprintf(buf, sz, c->fmt, ptr));
	}
	if (c->mode == 3)
		return (snprintf(buf, sz, c->fmt, c->w, c->p, (int)c->val));
	if (c->mode == 1)
		return (snprintf(buf, sz, c->fmt, c->w, (int)c->val));
	if (c->mode == 2)
		return (snprintf(buf, sz, c->fmt, c->p, (int)c->val));
	return (snprintf(buf, sz, c->fmt, (int)c->val));
}

static int	capture_ft(const t_case *c, char *out, int *outlen)
{
	int	ret;
	int	n;

	ftruncate(g_fd, 0);
	lseek(g_fd, 0, SEEK_SET);
	fflush(stdout);
	dup2(g_fd, 1);
	ret = call_fn(ft_printf, c);
	fflush(stdout);
	dup2(g_saved, 1);
	lseek(g_fd, 0, SEEK_SET);
	n = read(g_fd, out, 4095);
	if (n < 0)
		n = 0;
	out[n] = '\0';
	*outlen = n;
	return (ret);
}

static void	run_case(const t_case *c)
{
	char	exp[4096];
	char	got[4096];
	int		re;
	int		rg;
	int		lg;

	re = reference(c, exp, sizeof(exp));
	rg = capture_ft(c, got, &lg);
	if (re != rg || re != lg || memcmp(exp, got, re) != 0)
		report(c, exp, re, got, rg);
	else
		g_pass++;
}

static void	test_write_error(void)
{
	int	fd;
	int	ret;

	fd = open("/dev/full", O_WRONLY);
	if (fd < 0)
		return ;
	fflush(stdout);
	dup2(fd, 1);
	ret = ft_printf("hello %d %s\n", 42, "world");
	fflush(stdout);
	dup2(g_saved, 1);
	close(fd);
	if (ret == -1)
		g_pass++;
	else
	{
		g_fail++;
		fprintf(stderr, "FAIL write-error: expected -1, got %d\n", ret);
	}
}

static void	test_null_format(void)
{
	int	ret;

	ret = ft_printf(NULL);
	if (ret == -1)
		g_pass++;
	else
	{
		g_fail++;
		fprintf(stderr, "FAIL null-format: expected -1, got %d\n", ret);
	}
}

/* Compare a multi-argument format: reference snprintf vs captured ft_printf. */
#define MIX_CHECK(fmt, ...) do { \
	char	mexp[4096]; \
	char	mgot[4096]; \
	int		mre; \
	int		mrg; \
	int		mn; \
	mre = snprintf(mexp, sizeof(mexp), fmt, __VA_ARGS__); \
	ftruncate(g_fd, 0); \
	lseek(g_fd, 0, SEEK_SET); \
	fflush(stdout); \
	dup2(g_fd, 1); \
	mrg = ft_printf(fmt, __VA_ARGS__); \
	fflush(stdout); \
	dup2(g_saved, 1); \
	lseek(g_fd, 0, SEEK_SET); \
	mn = read(g_fd, mgot, 4095); \
	if (mn < 0) \
		mn = 0; \
	mgot[mn] = '\0'; \
	if (mre != mrg || mre != mn || memcmp(mexp, mgot, mre) != 0) \
	{ \
		g_fail++; \
		fprintf(stderr, "FAIL mix fmt=\"%s\"\n  exp=\"%s\"(%d) got=\"%s\"(%d)\n", \
			fmt, mexp, mre, mgot, mrg); \
	} \
	else \
		g_pass++; \
} while (0)

static void	test_mixed(void)
{
	MIX_CHECK("%c%s%p%d%i%u%x%X%%", 'A', "42", (void *)0x42, 42, 42, 42u, 42u, 42u);
	MIX_CHECK("%d %i %u %x %X", -2147483647 - 1, 2147483647, 4294967295u, 0u, 4294967295u);
	MIX_CHECK("[%+08d][%- 10i][%#010x][%#.8X]", 42, -42, 0xabcu, 0xabcu);
	MIX_CHECK("[%*.*d][%*s][%.*s]", -8, 3, 42, 10, "hi", 3, "hello");
	MIX_CHECK("[%+p][% p][%20p][%-20p]", (void *)0x1, (void *)0x2, (void *)0, (void *)0x3);
	MIX_CHECK("%s %s %s", NULL, "", "non-null");
	MIX_CHECK("%.0d %.0i %.0u %.0x %.0X", 0, 0, 0u, 0u, 0u);
	MIX_CHECK("%+.0d % .0d %#.0x %#.0X", 0, 0, 0u, 0u);
	MIX_CHECK("100%% done %d/%d", 1, 2);
	MIX_CHECK("%c%c%c", 0, 'a', 0);
	MIX_CHECK("no conversions here %%%%", 0);
}

static void	test_literals(void)
{
	int	r;

	r = ft_printf("");
	if (r == 0)
		g_pass++;
	else
	{
		g_fail++;
		fprintf(stderr, "FAIL empty-format: expected 0, got %d\n", r);
	}
	MIX_CHECK("plain text only %d", 7);
	MIX_CHECK("%%%%", 0);
	MIX_CHECK("%%", 0);
}

int	main(void)
{
	char	path[] = "/tmp/ftprintf_cap_XXXXXX";
	int		i;

	g_fd = mkstemp(path);
	unlink(path);
	g_saved = dup(1);
	if (g_fd < 0 || g_saved < 0)
		return (2);
	i = 0;
	while (i < CASE_COUNT)
	{
		run_case(&g_cases[i]);
		i++;
	}
	test_write_error();
	test_null_format();
	test_mixed();
	test_literals();
	dup2(g_saved, 1);
	close(g_saved);
	close(g_fd);
	printf("Differential tests: %d passed, %d failed (total %d)\n",
		g_pass, g_fail, g_pass + g_fail);
	if (g_fail > 0)
		return (1);
	return (0);
}
