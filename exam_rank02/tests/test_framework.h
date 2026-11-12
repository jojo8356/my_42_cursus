#ifndef TEST_FRAMEWORK_H
# define TEST_FRAMEWORK_H

# include <stdio.h>
# include <stdlib.h>
# include <string.h>

static int g_pass = 0;
static int g_fail = 0;
static int g_suite = 0;

# define TEST_BEGIN() \
    do { \
        printf("═══════════════════════════════════════════════════════════\n"); \
        printf("  Exam Rank 02 — Test Suite\n"); \
        printf("═══════════════════════════════════════════════════════════\n"); \
    } while (0)

# define TEST_SUITE(name) \
    do { \
        g_suite++; \
        printf("\n▶ Suite %d: %s\n", g_suite, name); \
    } while (0)

# define TEST_ASSERT(cond) \
    do { \
        if (cond) { g_pass++; } \
        else { \
            g_fail++; \
            printf("  ❌ FAIL: %s (line %d)\n", #cond, __LINE__); \
        } \
    } while (0)

# define TEST_ASSERT_EQ(a, b) \
    do { \
        long _va = (long)(a); \
        long _vb = (long)(b); \
        if (_va == _vb) { g_pass++; } \
        else { \
            g_fail++; \
            printf("  ❌ FAIL: expected %ld, got %ld (line %d)\n", _vb, _va, __LINE__); \
        } \
    } while (0)

# define TEST_ASSERT_STREQ(a, b) \
    do { \
        const char *_sa = (a); \
        const char *_sb = (b); \
        if (_sa && _sb && strcmp(_sa, _sb) == 0) { g_pass++; } \
        else { \
            g_fail++; \
            printf("  ❌ FAIL: expected \"%s\", got \"%s\" (line %d)\n", \
                   _sb ? _sb : "(null)", _sa ? _sa : "(null)", __LINE__); \
        } \
    } while (0)

# define TEST_END() \
    do { \
        printf("\n═══════════════════════════════════════════════════════════\n"); \
        printf("  Result: %d/%d passed, %d failed\n", g_pass, g_pass + g_fail, g_fail); \
        if (g_fail == 0) \
            printf("  ✅ ALL TESTS PASSED\n"); \
        else \
            printf("  ❌ %d TEST(S) FAILED\n", g_fail); \
        printf("═══════════════════════════════════════════════════════════\n"); \
    } while (0)

# define TEST_FINISH() \
    do { \
        TEST_END(); \
        return g_fail; \
    } while (0)

#endif
