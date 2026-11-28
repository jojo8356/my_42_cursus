#include "test_utils.h"
#include "test_framework.h"
#include "exam_rank02.h"

#include <stdlib.h>
#include <string.h>

static void free_string_array(char **array)
{
    int index;

    if (!array)
        return ;
    index = 0;
    while (array[index])
    {
        free(array[index]);
        index++;
    }
    free(array);
}

static char *heap_string(const char *source)
{
    size_t length;
    char *copy;

    length = strlen(source);
    copy = (char *)malloc(length + 1);
    if (copy)
        memcpy(copy, source, length + 1);
    return (copy);
}

static void test_prime_and_string_helpers(void)
{
    char *single[] = {"program", "10"};
    char *multiple[] = {"program", "10", "5"};
    char compact[] = "   hello   rank   02  ";
    char *expanded;
    char *input;

    TEST_SUITE("prime sums / string helpers / base conversion");
    TEST_ASSERT_EQ(add_prime_sum(2, single), 17);
    TEST_ASSERT_EQ(add_prime_sum(3, multiple), 22);
    TEST_ASSERT(epur_str(compact) == compact);
    TEST_ASSERT_STREQ(compact, "hello rank 02");

    input = heap_string("a3b2");
    TEST_ASSERT(input != NULL);
    expanded = expand_str(input);
    TEST_ASSERT_STREQ(expanded, "aaabb");
    free(expanded);

    TEST_ASSERT_EQ(ft_atoi_base("  -101", "01"), -5);
    TEST_ASSERT_EQ(ft_atoi_base("2a", "0123456789abcdef"), 42);
}

static void test_lists_and_ranges(void)
{
    int first_value;
    int second_value;
    int third_value;
    list_t third;
    list_t second;
    list_t first;
    char **range;
    char **reverse_range;

    TEST_SUITE("list size / generated ranges");
    first_value = 1;
    second_value = 2;
    third_value = 3;
    first.content = &first_value;
    first.next = &second;
    second.content = &second_value;
    second.next = &third;
    third.content = &third_value;
    third.next = NULL;
    TEST_ASSERT_EQ(ft_list_size(NULL), 0);
    TEST_ASSERT_EQ(ft_list_size(&first), 3);

    range = ft_range(-2, 3);
    TEST_ASSERT(range != NULL);
    TEST_ASSERT_STREQ(range[0], "-2");
    TEST_ASSERT_STREQ(range[1], "-1");
    TEST_ASSERT_STREQ(range[2], "0");
    TEST_ASSERT_STREQ(range[3], "1");
    TEST_ASSERT_STREQ(range[4], "2");
    TEST_ASSERT(range[5] == NULL);
    free_string_array(range);

    reverse_range = ft_rrange(-1, 3);
    TEST_ASSERT(reverse_range != NULL);
    TEST_ASSERT_STREQ(reverse_range[0], "2");
    TEST_ASSERT_STREQ(reverse_range[1], "1");
    TEST_ASSERT_STREQ(reverse_range[2], "0");
    TEST_ASSERT_STREQ(reverse_range[3], "-1");
    TEST_ASSERT(reverse_range[4] == NULL);
    free_string_array(reverse_range);
    TEST_ASSERT(ft_range(4, 4) == NULL);
}

static void test_numeric_and_output_helpers(void)
{
    char output[128];
    char capitalized[] = "hELLO wORLD";
    char reverse_capitalized[] = "HELLO world";
    char **table;
    t_test_capture capture;

    TEST_SUITE("hidenp / gcd / numeric output / capitalization");
    test_capture_begin(&capture, output, sizeof(output));
    hidenp("abc", "a--b--c");
    test_capture_end(&capture);
    TEST_ASSERT_STREQ(output, "abc");

    TEST_ASSERT_EQ(lcm(4, 6), 12);
    TEST_ASSERT_EQ(lcm(0, 6), 0);
    TEST_ASSERT_EQ(paramsum("12 -4 +3 0"), 15);
    TEST_ASSERT_EQ(paramsum("1 2 3"), 6);
    TEST_ASSERT_EQ(pgcd(54, 24), 6);
    TEST_ASSERT_EQ(pgcd(17, 13), 1);

    test_capture_begin(&capture, output, sizeof(output));
    print_hex(42);
    test_capture_end(&capture);
    TEST_ASSERT_STREQ(output, "2a");
    test_capture_begin(&capture, output, sizeof(output));
    print_hex(0);
    test_capture_end(&capture);
    TEST_ASSERT_STREQ(output, "0");

    TEST_ASSERT(str_capitalizer(capitalized) == capitalized);
    TEST_ASSERT_STREQ(capitalized, "Hello World");
    TEST_ASSERT(rstr_capitalizer(reverse_capitalized) == reverse_capitalized);
    TEST_ASSERT_STREQ(reverse_capitalized, "hellO worlD");

    table = tab_mult(3);
    TEST_ASSERT(table != NULL);
    TEST_ASSERT_STREQ(table[0], "0");
    TEST_ASSERT_STREQ(table[1], "3");
    TEST_ASSERT_STREQ(table[2], "6");
    TEST_ASSERT_STREQ(table[3], "9");
    TEST_ASSERT(table[4] == NULL);
    free_string_array(table);
}

int main(void)
{
    TEST_BEGIN();
    test_prime_and_string_helpers();
    test_lists_and_ranges();
    test_numeric_and_output_helpers();
    TEST_FINISH();
}
