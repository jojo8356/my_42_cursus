/* ************************************************************************** */
/*                                                                            */
/*   test_framework.h                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Framework de test minimal pour NetPratice.                          */
/* ************************************************************************** */

# ifndef TEST_FRAMEWORK_H
# define TEST_FRAMEWORK_H

# include <stdio.h>
# include <string.h>
# include "netpratice.h"

static int        g_tests = 0;
static int        g_passed = 0;
static int        g_failed = 0;

# define TEST(name) static void name(void)

# define RUN(name) do { \
    g_tests++; \
    int _before = g_failed; \
    fprintf(stderr, "[ RUN  ] %s\n", #name); \
    name(); \
    if (g_failed == _before) \
    { \
        g_passed++; \
        fprintf(stderr, "[  OK  ] %s\n", #name); \
    } \
    else \
        fprintf(stderr, "[ FAIL ] %s\n", #name); \
} while (0)

# define ASSERT(cond, msg) do { \
    g_tests++; \
    if (cond) \
    { \
        g_passed++; \
    } \
    else \
    { \
        g_failed++; \
        fprintf(stderr, "  ASSERT FAILED: %s (%s:%d)\n", msg, __FILE__, __LINE__); \
    } \
} while (0)

# define ASSERT_EQ_INT(a, b, msg) do { \
    g_tests++; \
    long _a = (long)(a), _b = (long)(b); \
    if (_a == _b) \
        g_passed++; \
    else \
    { \
        g_failed++; \
        fprintf(stderr, "  ASSERT FAILED: %s (expected %ld, got %ld) [%s:%d]\n", \
            msg, _b, _a, __FILE__, __LINE__); \
    } \
} while (0)

# define ASSERT_EQ_STR(a, b, msg) do { \
    g_tests++; \
    if (strcmp((a), (b)) == 0) \
        g_passed++; \
    else \
    { \
        g_failed++; \
        fprintf(stderr, "  ASSERT FAILED: %s (expected \"%s\", got \"%s\") [%s:%d]\n", \
            msg, (b), (a), __FILE__, __LINE__); \
    } \
} while (0)

# define SUMMARY() do { \
    fprintf(stderr, "\n=== %d tests, %d passed, %d failed ===\n", \
        g_tests, g_passed, g_failed); \
    return (g_failed == 0 ? 0 : 1); \
} while (0)

# endif
