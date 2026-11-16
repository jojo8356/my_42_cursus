#ifndef TEST_FRAMEWORK_H
#define TEST_FRAMEWORK_H

#include "../libft/libft.h"
#include <ctype.h>
#include <fcntl.h>
#include <limits.h>
#include <malloc.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define CLR_GREEN "\033[0;32m"
#define CLR_RED   "\033[0;31m"
#define CLR_CYAN  "\033[0;36m"
#define CLR_BOLD  "\033[1m"
#define CLR_RESET "\033[0m"

typedef struct s_stats
{
	int	fn_index;
	int	fn_passed;
	int	fn_failed;
	int	total_assertions;
	int	failed_assertions;
	int	cur_fn_assertions;
	int	cur_fn_failures;
}	t_stats;

extern t_stats	g_stats;

#define BEGIN_FN_TEST(fn_name) \
	do { \
		g_stats.fn_index++; \
		g_stats.cur_fn_assertions = 0; \
		g_stats.cur_fn_failures = 0; \
		(void)(fn_name); \
	} while (0)

#define END_FN_TEST(fn_name) \
	do { \
		if (g_stats.cur_fn_failures == 0) { \
			g_stats.fn_passed++; \
			printf("  " CLR_GREEN "[PASS]" CLR_RESET " [%02d/43] %-18s (%d assertions)\n", \
				g_stats.fn_index, fn_name, g_stats.cur_fn_assertions); \
		} else { \
			g_stats.fn_failed++; \
			printf("  " CLR_RED "[FAIL]" CLR_RESET " [%02d/43] %-18s (%d/%d echecs)\n", \
				g_stats.fn_index, fn_name, g_stats.cur_fn_failures, g_stats.cur_fn_assertions); \
		} \
	} while (0)

#define T_ASSERT(cond, msg) \
	do { \
		g_stats.total_assertions++; \
		g_stats.cur_fn_assertions++; \
		if (!(cond)) { \
			g_stats.failed_assertions++; \
			g_stats.cur_fn_failures++; \
			fprintf(stderr, "    " CLR_RED "✗ Echec (%s:%d): %s" CLR_RESET "\n", \
				__FILE__, __LINE__, msg); \
		} \
	} while (0)

static inline int	exact_alloc_matches(void *ptr, size_t expected_bytes)
{
	void	*ref;
	size_t	got_usable;
	size_t	ref_usable;

	if (!ptr)
		return (0);
	ref = malloc(expected_bytes);
	if (!ref)
		return (0);
	got_usable = malloc_usable_size(ptr);
	ref_usable = malloc_usable_size(ref);
	free(ref);
	return (got_usable == ref_usable);
}

void	run_part1_tests(void);
void	run_part2_tests(void);
void	run_part3_tests(void);

#endif
