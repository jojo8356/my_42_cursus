/*
** test_framework.h — Minimal assert framework for miniRT tests.
**
** Provides TEST_ASSERT, TEST_ASSERT_EQ, TEST_ASSERT_NEAR macros that record
** pass/fail counts and print detailed diagnostics on failure.
**
** Usage:
**   int main(void) {
**       TEST_BEGIN();
**       TEST_ASSERT(1 + 1 == 2);
**       TEST_ASSERT_NEAR(0.1 + 0.2, 0.3, 1e-9);
**       return TEST_END();
**   }
*/

# ifndef TEST_FRAMEWORK_H
# define TEST_FRAMEWORK_H

# include <stdio.h>
# include <stdlib.h>
# include <math.h>
# include <string.h>

static int g_tests_run = 0;
static int g_tests_passed = 0;
static int g_tests_failed = 0;
static const char *g_current_suite = "";

# define TEST_BEGIN() \
    do { \
        printf("═══════════════════════════════════════════════════════════\n"); \
        printf("  miniRT — Test Suite\n"); \
        printf("═══════════════════════════════════════════════════════════\n"); \
    } while (0)

# define TEST_END() \
    do { \
        printf("\n═══════════════════════════════════════════════════════════\n"); \
        printf("  Result: %d/%d passed, %d failed\n", g_tests_passed, \
            g_tests_run, g_tests_failed); \
        if (g_tests_failed == 0) \
            printf("  ✅ ALL TESTS PASSED\n"); \
        else \
            printf("  ❌ %d TEST(S) FAILED\n", g_tests_failed); \
        printf("═══════════════════════════════════════════════════════════\n"); \
        return (g_tests_failed == 0 ? 0 : 1); \
    } while (0)

# define TEST_SUITE(name) \
    do { \
        g_current_suite = name; \
        printf("\n▶ Suite: %s\n", name); \
    } while (0)

# define TEST_ASSERT(cond) \
    do { \
        g_tests_run++; \
        if (cond) { \
            g_tests_passed++; \
        } else { \
            g_tests_failed++; \
            printf("  ❌ FAIL: %s (%s:%d)\n", #cond, __FILE__, __LINE__); \
        } \
    } while (0)

# define TEST_ASSERT_EQ(actual, expected) \
    do { \
        long _a = (long)(actual); \
        long _e = (long)(expected); \
        g_tests_run++; \
        if (_a == _e) { \
            g_tests_passed++; \
        } else { \
            g_tests_failed++; \
            printf("  ❌ FAIL: expected %ld, got %ld (%s:%d)\n", \
                _e, _a, __FILE__, __LINE__); \
        } \
    } while (0)

# define TEST_ASSERT_NEAR(actual, expected, tol) \
    do { \
        double _a = (double)(actual); \
        double _e = (double)(expected); \
        double _t = (double)(tol); \
        g_tests_run++; \
        if (fabs(_a - _e) <= _t) { \
            g_tests_passed++; \
        } else { \
            g_tests_failed++; \
            printf("  ❌ FAIL: expected %g ± %g, got %g (%s:%d)\n", \
                _e, _t, _a, __FILE__, __LINE__); \
        } \
    } while (0)

# define TEST_ASSERT_STR(actual, expected) \
    do { \
        const char *_a = (actual); \
        const char *_e = (expected); \
        g_tests_run++; \
        if (_a && _e && strcmp(_a, _e) == 0) { \
            g_tests_passed++; \
        } else { \
            g_tests_failed++; \
            printf("  ❌ FAIL: expected \"%s\", got \"%s\" (%s:%d)\n", \
                _e ? _e : "(null)", _a ? _a : "(null)", __FILE__, __LINE__); \
        } \
    } while (0)

#endif
