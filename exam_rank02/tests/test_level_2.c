#include "test_utils.h"
#include "test_framework.h"
#include "exam_rank02.h"

#include <stdlib.h>
#include <string.h>

static void bits_to_text(unsigned int value, char output[34])
{
    int bit;

    bit = 31;
    while (bit >= 0)
    {
        output[31 - bit] = ((value >> bit) & 1U) ? '1' : '0';
        bit--;
    }
    output[32] = '\n';
    output[33] = '\0';
}

static void test_string_transforms(void)
{
    char mirror[] = "aZ-0";
    char camel[32] = "CamelCase";
    char snake[] = "hello_world";
    char reverse[] = "Rank 02";
    char last[] = "one two \t\n";

    TEST_SUITE("string conversions and spans");
    TEST_ASSERT(alpha_mirror(mirror) == mirror);
    TEST_ASSERT_STREQ(mirror, "zA-0");
    TEST_ASSERT(camel_to_snake(camel) == camel);
    TEST_ASSERT_STREQ(camel, "camel_case");
    TEST_ASSERT(snake_to_camel(snake) == snake);
    TEST_ASSERT_STREQ(snake, "helloWorld");
    TEST_ASSERT(ft_strrev(reverse) == reverse);
    TEST_ASSERT_STREQ(reverse, "20 knaR");
    TEST_ASSERT_EQ(ft_strcspn("abcdef", "xzdf"), 3);
    TEST_ASSERT_EQ(ft_strcspn("abcdef", "XYZ"), 6);
    TEST_ASSERT_EQ(ft_strspn("123abc", "0123456789"), 3);
    TEST_ASSERT_EQ(ft_strspn("abc", "0123456789"), 0);
    TEST_ASSERT_STREQ(last_word(last), "two");
    TEST_ASSERT_STREQ(last, "one two");
}

static void test_arithmetic_and_comparison(void)
{
    char *values[] = {"1", "2", "2", "3"};
    char *duplicates[] = {"5", "5", "5"};
    int numbers[] = {-7, 4, 0, 2};

    TEST_SUITE("arithmetic / conversion / comparison");
    TEST_ASSERT_EQ(do_op(7, 5, '+'), 12);
    TEST_ASSERT_EQ(do_op(7, 5, '-'), 2);
    TEST_ASSERT_EQ(do_op(7, 5, '*'), 35);
    TEST_ASSERT_EQ(do_op(14, 3, '/'), 4);
    TEST_ASSERT_EQ(do_op(14, 3, '%'), 2);
    TEST_ASSERT_EQ(do_op(7, 5, '?'), 0);
    TEST_ASSERT_EQ(ft_atoi("  \t-42tail"), -42);
    TEST_ASSERT_EQ(ft_atoi("+17"), 17);
    TEST_ASSERT_EQ(ft_atoi("no digits"), 0);
    TEST_ASSERT_EQ(ft_strcmp("same", "same"), 0);
    TEST_ASSERT_EQ(ft_strcmp("abc", "abd"), -1);
    TEST_ASSERT_EQ(ft_strcmp("abd", "abc"), 1);
    TEST_ASSERT_EQ(inter(4, values), 1);
    TEST_ASSERT_EQ(inter(3, duplicates), 3);
    TEST_ASSERT_EQ(is_power_of_2(1), 1);
    TEST_ASSERT_EQ(is_power_of_2(64), 1);
    TEST_ASSERT_EQ(is_power_of_2(0), 0);
    TEST_ASSERT_EQ(is_power_of_2(18), 0);
    TEST_ASSERT_EQ(max(numbers, 4), 4);
    TEST_ASSERT_EQ(max(numbers, 0), 0);
}

static void test_allocating_and_matching_functions(void)
{
    char source[] = "Rank02";
    char first[] = "abcdef";
    char *copy;
    char *found;
    char *joined;

    TEST_SUITE("allocation / matching / bit output");
    copy = ft_strdup(source);
    TEST_ASSERT(copy != NULL);
    TEST_ASSERT_STREQ(copy, "Rank02");
    TEST_ASSERT(copy != source);
    free(copy);

    found = ft_strpbrk(first, "xzdf");
    TEST_ASSERT(found == first + 3);
    TEST_ASSERT(ft_strpbrk(first, "XYZ") == NULL);

    joined = union_words("a bca", "cabxy");
    TEST_ASSERT_STREQ(joined, "abcxy");
    free(joined);

    TEST_ASSERT_EQ(wdmatch("abc", "abc"), 1);
    TEST_ASSERT_EQ(wdmatch("abc", "axc"), 0);
    TEST_ASSERT_EQ(wdmatch("a*c", "abc"), 1);

    {
        char output[64];
        char expected[34];
        t_test_capture capture;

        bits_to_text(5U, expected);
        test_capture_begin(&capture, output, sizeof(output));
        print_bits(5U);
        test_capture_end(&capture);
        TEST_ASSERT_STREQ(output, expected);

        bits_to_text(0x80000000U, expected);
        test_capture_begin(&capture, output, sizeof(output));
        reverse_bits(1U);
        test_capture_end(&capture);
        TEST_ASSERT_STREQ(output, expected);

        bits_to_text(0x14U, expected);
        test_capture_begin(&capture, output, sizeof(output));
        swap_bits();
        test_capture_end(&capture);
        TEST_ASSERT_STREQ(output, expected);
    }
}

int main(void)
{
    TEST_BEGIN();
    test_string_transforms();
    test_arithmetic_and_comparison();
    test_allocating_and_matching_functions();
    TEST_FINISH();
}
