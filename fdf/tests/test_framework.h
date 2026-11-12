/*
** test_framework.h — Minimal assert framework for FDF tests.
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
        printf("  fdf test suite — %s\n", __DATE__ " " __TIME__); \
        printf("═══════════════════════════════════════════════════════════\n"); \
    } while(0)

# define TEST_END() \
    ({ \
        printf("\n────────────────────────────────────────────────────────────\n"); \
        printf("  Result: %d/%d passed, %d failed\n", \
               g_tests_passed, g_tests_run, g_tests_failed); \
        if (g_tests_failed == 0) \
            printf("  ✅ ALL TESTS PASSED\n"); \
        else \
            printf("  ❌ %d TEST(S) FAILED\n", g_tests_failed); \
        printf("────────────────────────────────────────────────────────────\n"); \
        g_tests_failed == 0 ? 0 : 1; \
    })

# define TEST_SUITE(name) \
    do { g_current_suite = name; \
         printf("\n── %s\n", name); } while(0)

# define TEST_ASSERT(cond) \
    do { \
        g_tests_run++; \
        if (cond) { \
            g_tests_passed++; \
        } else { \
            g_tests_failed++; \
            printf("  ❌ FAIL [%s] %s:%d: %s\n", \
                   g_current_suite, __FILE__, __LINE__, #cond); \
        } \
    } while(0)

# define TEST_ASSERT_EQ(actual, expected) \
    do { \
        g_tests_run++; \
        long a_ = (long)(actual); \
        long e_ = (long)(expected); \
        if (a_ == e_) { \
            g_tests_passed++; \
        } else { \
            g_tests_failed++; \
            printf("  ❌ FAIL [%s] %s:%d: expected %ld, got %ld\n", \
                   g_current_suite, __FILE__, __LINE__, e_, a_); \
        } \
    } while(0)

# define TEST_ASSERT_NEAR(actual, expected, tol) \
    do { \
        g_tests_run++; \
        double a_ = (double)(actual); \
        double e_ = (double)(expected); \
        double d_ = a_ - e_; \
        if (d_ < 0) d_ = -d_; \
        if (d_ <= (double)(tol)) { \
            g_tests_passed++; \
        } else { \
            g_tests_failed++; \
            printf("  ❌ FAIL [%s] %s:%d: expected %.10g ± %g, got %.10g\n", \
                   g_current_suite, __FILE__, __LINE__, e_, (double)(tol), a_); \
        } \
    } while(0)

# define TEST_ASSERT_STR(actual, expected) \
    do { \
        g_tests_run++; \
        if (strcmp((actual), (expected)) == 0) { \
            g_tests_passed++; \
        } else { \
            g_tests_failed++; \
            printf("  ❌ FAIL [%s] %s:%d: expected \"%s\", got \"%s\"\n", \
                   g_current_suite, __FILE__, __LINE__, (expected), (actual)); \
        } \
    } while(0)

# endif
