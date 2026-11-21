/* ************************************************************************** */
/*                                                                            */
/*   test_framework.h                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Framework de tests minimal (assertions + compteurs).                 */
/* ************************************************************************** */

#ifndef TEST_FRAMEWORK_H
# define TEST_FRAMEWORK_H

# include <stdio.h>

static int        g_tests = 0;
static int        g_passed = 0;
static int        g_failed = 0;

# define TEST(name) static void name(void)

# define RUN(name)                                    \
    do {                                              \
        int        fail_before = g_failed;            \
        g_tests++;                                    \
        printf("[ RUN  ] %s\n", #name);               \
        name();                                       \
        if (g_failed == fail_before)                  \
        {                                             \
            g_passed++;                               \
            printf("[  OK  ] %s\n", #name);          \
        }                                             \
        else                                          \
            printf("[ FAIL ] %s\n", #name);           \
    } while (0)

# define ASSERT(cond, msg)                                   \
    do {                                                     \
        if (!(cond)) {                                       \
            g_failed++;                                      \
            printf("  ASSERT FAILED: %s [%s:%d]\n",         \
                msg, __FILE__, __LINE__);                   \
        }                                                    \
    } while (0)

# define ASSERT_EQ_INT(actual, expected, msg)                \
    do {                                                     \
        if ((actual) != (expected)) {                        \
            g_failed++;                                      \
            printf("  ASSERT FAILED: %s (expected %d, got %d) [%s:%d]\n", \
                msg, (int)(expected), (int)(actual), __FILE__, __LINE__); \
        }                                                    \
    } while (0)

# define SUMMARY()                                           \
    do {                                                     \
        printf("\n=== %d tests, %d passed, %d failed ===\n", \
            g_tests, g_passed, g_failed);                    \
        if (g_failed == 0)                                   \
            printf("ALL TESTS PASSED\n");                    \
        else                                                 \
            printf("SOME TESTS FAILED\n");                   \
        return (g_failed == 0 ? 0 : 1);                      \
    } while (0)

#endif
