/*
** test_framework.h — Minimal assert framework for fract-ol tests.
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
        printf("  fract-ol test suite — %s\n", __DATE__ " " __TIME__); \
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

/* Helper: create a minimal fractal struct for headless testing. */
static inline void test_init_fractal(t_fractal *f, int type, double c_r, double c_i)
{
    memset(f, 0, sizeof(*f));
    f->type = (t_fractal_type)type;
    f->c_r = c_r;
    f->c_i = c_i;
    f->max_iter = 1000;
    f->zoom = 4.0;
    f->shift_r = 0.0;
    f->shift_i = 0.0;
    f->palette = 0;
    f->color_shift = 0;
    f->grayscale = 0;
    f->delta = 0.0;
    f->multibrot_power = 3;
}

# endif /* TEST_FRAMEWORK_H */
