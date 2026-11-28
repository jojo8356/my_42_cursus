#include "test_utils.h"
#include "test_framework.h"
#include "exam_rank02.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void test_first_word(void)
{
    char output[64];
    t_test_capture capture;
    int length;

    TEST_SUITE("first_word / fizzbuzz / ft_putstr");
    test_capture_begin(&capture, output, sizeof(output));
    length = first_word("hello world");
    test_capture_end(&capture);
    TEST_ASSERT_EQ(length, 5);
    TEST_ASSERT_STREQ(output, "hello");
}

static void test_fizzbuzz(void)
{
    char output[2048];
    char expected[2048];
    t_test_capture capture;
    size_t used;
    int value;

    used = 0;
    value = 1;
    while (value <= 100)
    {
        if (value % 15 == 0)
            used += (size_t)snprintf(expected + used, sizeof(expected) - used,
                    "FizzBuzz\n");
        else if (value % 3 == 0)
            used += (size_t)snprintf(expected + used, sizeof(expected) - used,
                    "Fizz\n");
        else if (value % 5 == 0)
            used += (size_t)snprintf(expected + used, sizeof(expected) - used,
                    "Buzz\n");
        else
            used += (size_t)snprintf(expected + used, sizeof(expected) - used,
                    "%d\n", value);
        value++;
    }
    test_capture_begin(&capture, output, sizeof(output));
    fizzbuzz();
    test_capture_end(&capture);
    TEST_ASSERT_STREQ(output, expected);
}

static void test_ft_putstr_and_copy(void)
{
    char output[32];
    char destination[16];
    char empty_destination[4];
    t_test_capture capture;

    TEST_SUITE("ft_putstr / ft_strcpy / ft_strlen / ft_swap");
    test_capture_begin(&capture, output, sizeof(output));
    ft_putstr("42!");
    test_capture_end(&capture);
    TEST_ASSERT_STREQ(output, "42!");
    TEST_ASSERT(ft_strcpy(destination, "exam") == destination);
    TEST_ASSERT_STREQ(destination, "exam");
    ft_strcpy(empty_destination, "");
    TEST_ASSERT_STREQ(empty_destination, "");
    TEST_ASSERT_EQ(ft_strlen(""), 0);
    TEST_ASSERT_EQ(ft_strlen("Rank 02"), 7);
    {
        int left;
        int right;

        left = -8;
        right = 42;
        ft_swap(&left, &right);
        TEST_ASSERT_EQ(left, 42);
        TEST_ASSERT_EQ(right, -8);
    }
}

static void test_printing_transformations(void)
{
    char output[128];
    char rot13_text[] = "Hello, World!";
    char rotone_text[] = "zZ-9";
    char replacement[] = "banana";
    t_test_capture capture;

    TEST_SUITE("repeat_alpha / rev_print / ROT / replacement");
    test_capture_begin(&capture, output, sizeof(output));
    repeat_alpha("abc");
    test_capture_end(&capture);
    TEST_ASSERT_STREQ(output, "abbccc");

    test_capture_begin(&capture, output, sizeof(output));
    rev_print("OpenAI!");
    test_capture_end(&capture);
    TEST_ASSERT_STREQ(output, "!IAnepO");

    test_capture_begin(&capture, output, sizeof(output));
    rot_13(rot13_text);
    test_capture_end(&capture);
    TEST_ASSERT_STREQ(rot13_text, "Uryyb, Jbeyq!");
    TEST_ASSERT_STREQ(output, "Uryyb, Jbeyq!\n");

    test_capture_begin(&capture, output, sizeof(output));
    rotone(rotone_text);
    test_capture_end(&capture);
    TEST_ASSERT_STREQ(rotone_text, "aA-9");
    TEST_ASSERT_STREQ(output, "aA-9\n");

    search_and_replace('a', 'o', replacement);
    TEST_ASSERT_STREQ(replacement, "bonono");
}

static void test_ulstr(void)
{
    char hello[] = "Hello";
    char mixed[] = "zA09";
    char punctuation[] = "A!b";

    TEST_SUITE("ulstr: lowercase, then sort characters");
    TEST_ASSERT(ulstr(hello) == hello);
    TEST_ASSERT_STREQ(hello, "ehllo");
    ulstr(mixed);
    TEST_ASSERT_STREQ(mixed, "09az");
    ulstr(punctuation);
    TEST_ASSERT_STREQ(punctuation, "!ab");
}

int main(void)
{
    TEST_BEGIN();
    test_first_word();
    test_fizzbuzz();
    test_ft_putstr_and_copy();
    test_printing_transformations();
    test_ulstr();
    TEST_FINISH();
}
