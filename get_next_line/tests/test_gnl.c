#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

#ifdef TEST_BONUS
# include "../get_next_line_bonus.h"
#else
# include "../get_next_line.h"
#endif

static int	g_pass = 0;
static int	g_fail = 0;

static void	check(int cond, const char *label)
{
	if (cond)
		g_pass++;
	else
	{
		g_fail++;
		fprintf(stderr, "  [FAIL] %s (BUFFER_SIZE=%d)\n", label, BUFFER_SIZE);
	}
}

static void	test_invalid_and_empty(void)
{
	int		fd;
	char	path[] = "/tmp/gnl_empty_XXXXXX";

	check(get_next_line(-1) == NULL, "fd = -1 returns NULL");
	check(get_next_line(-42) == NULL, "fd = -42 returns NULL");
	check(get_next_line(999) == NULL, "unopened fd = 999 returns NULL");
	fd = mkstemp(path);
	unlink(path);
	close(fd);
	fd = open("/dev/null", O_RDONLY);
	check(fd >= 0, "open /dev/null");
	check(get_next_line(fd) == NULL, "/dev/null first call -> NULL");
	check(get_next_line(fd) == NULL, "/dev/null second call -> NULL");
	close(fd);
	check(get_next_line(fd) == NULL, "closed fd -> NULL");
}

static void	test_pipe_fragmented(void)
{
	int		pfd[2];
	char	*line;

	if (BUFFER_SIZE <= 0)
	{
		check(pipe(pfd) == 0, "pipe creation");
		write(pfd[1], "abc\n", 4);
		close(pfd[1]);
		check(get_next_line(pfd[0]) == NULL, "BS<=0 returns NULL on pipe");
		close(pfd[0]);
		return ;
	}
	check(pipe(pfd) == 0, "pipe creation");
	write(pfd[1], "Hello ", 6);
	write(pfd[1], "from ", 5);
	write(pfd[1], "pipe!\nSecond line no nl", 23);
	close(pfd[1]);
	line = get_next_line(pfd[0]);
	check(line && strcmp(line, "Hello from pipe!\n") == 0, "pipe line 1");
	free(line);
	line = get_next_line(pfd[0]);
	check(line && strcmp(line, "Second line no nl") == 0, "pipe line 2 EOF");
	free(line);
	check(get_next_line(pfd[0]) == NULL, "pipe EOF NULL");
	close(pfd[0]);
}

static void	test_binary_embedded_nul(void)
{
	int		pfd[2];
	char	*line;

	if (BUFFER_SIZE <= 0)
		return ;
	check(pipe(pfd) == 0, "binary pipe");
	write(pfd[1], "ab\0cd\nxy\0z", 10);
	close(pfd[1]);
	line = get_next_line(pfd[0]);
	check(line != NULL && memcmp(line, "ab\0cd\n\0", 7) == 0,
		"binary embedded NUL line 1 preserved");
	free(line);
	line = get_next_line(pfd[0]);
	check(line != NULL && memcmp(line, "xy\0z\0", 5) == 0,
		"binary embedded NUL line 2 preserved");
	free(line);
	check(get_next_line(pfd[0]) == NULL, "binary pipe EOF NULL");
	close(pfd[0]);
}

static void	test_linear_perf_large_line(void)
{
	const size_t	sz = 200000;
	char			path[] = "/tmp/gnl_large_XXXXXX";
	char			*src;
	char			*got;
	int				fd;
	struct timeval	t0;
	struct timeval	t1;
	long			elapsed_ms;

	if (BUFFER_SIZE <= 0)
		return ;
	fd = mkstemp(path);
	src = malloc(sz + 2);
	memset(src, 'A', sz);
	src[sz] = '\n';
	src[sz + 1] = '\0';
	write(fd, src, sz + 1);
	write(fd, "tail", 4);
	close(fd);
	fd = open(path, O_RDONLY);
	unlink(path);
	gettimeofday(&t0, NULL);
	got = get_next_line(fd);
	gettimeofday(&t1, NULL);
	elapsed_ms = (t1.tv_sec - t0.tv_sec) * 1000L
		+ (t1.tv_usec - t0.tv_usec) / 1000L;
	check(got && strlen(got) == sz + 1 && memcmp(got, src, sz + 1) == 0,
		"200KB single line content exact");
	check(elapsed_ms < 2000, "200KB single line O(L) speed (< 2000 ms)");
	free(got);
	got = get_next_line(fd);
	check(got && strcmp(got, "tail") == 0, "tail after 200KB line");
	free(got);
	check(get_next_line(fd) == NULL, "EOF after 200KB line");
	close(fd);
	free(src);
}

#ifdef TEST_BONUS

static void	test_bonus_high_and_interleaved_fds(void)
{
	int		p1[2];
	int		p2[2];
	int		p3[2];
	int		high_fd;
	char	*s;

	if (BUFFER_SIZE <= 0)
		return ;
	pipe(p1);
	pipe(p2);
	pipe(p3);
	write(p1[1], "A1\nA2\nA3", 8);
	close(p1[1]);
	write(p2[1], "B1_long_line_1234567890\nB2\n", 27);
	close(p2[1]);
	write(p3[1], "\nC2_end", 7);
	close(p3[1]);
	high_fd = 555;
	check(dup2(p2[0], high_fd) == high_fd, "dup2 to high fd 555");
	close(p2[0]);
	s = get_next_line(p1[0]);
	check(s && strcmp(s, "A1\n") == 0, "bonus p1 line 1");
	free(s);
	s = get_next_line(high_fd);
	check(s && strcmp(s, "B1_long_line_1234567890\n") == 0, "bonus fd555 l1");
	free(s);
	s = get_next_line(p3[0]);
	check(s && strcmp(s, "\n") == 0, "bonus p3 line 1");
	free(s);
	check(get_next_line(888) == NULL, "bonus invalid fd 888 interleaved");
	s = get_next_line(p1[0]);
	check(s && strcmp(s, "A2\n") == 0, "bonus p1 line 2");
	free(s);
	s = get_next_line(high_fd);
	check(s && strcmp(s, "B2\n") == 0, "bonus fd555 l2");
	free(s);
	s = get_next_line(p3[0]);
	check(s && strcmp(s, "C2_end") == 0, "bonus p3 line 2");
	free(s);
	s = get_next_line(p1[0]);
	check(s && strcmp(s, "A3") == 0, "bonus p1 line 3");
	free(s);
	check(get_next_line(p1[0]) == NULL, "bonus p1 EOF");
	check(get_next_line(high_fd) == NULL, "bonus fd555 EOF");
	check(get_next_line(p3[0]) == NULL, "bonus p3 EOF");
	close(p1[0]);
	close(high_fd);
	close(p3[0]);
}
#endif

int	main(void)
{
	test_invalid_and_empty();
	test_pipe_fragmented();
	test_binary_embedded_nul();
	test_linear_perf_large_line();
#ifdef TEST_BONUS
	test_bonus_high_and_interleaved_fds();
#endif
	printf("  [BS=%d%s] %d passed, %d failed\n", BUFFER_SIZE,
#ifdef TEST_BONUS
		" BONUS",
#else
		" MANDATORY",
#endif
		g_pass, g_fail);
	return (g_fail != 0);
}
