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

static void increment_content(void *content)
{
    (*(int *)content)++;
}

static int remove_value_two(void *content, void *unused)
{
    (void)unused;
    return (*(int *)content == 2);
}

static void ignore_content(void *content)
{
    (void)content;
}

static void test_flood_fill_and_factorization(void)
{
    int row0[] = {0, 0, 1};
    int row1[] = {0, 1, 0};
    int row2[] = {0, 0, 0};
    int *map[] = {row0, row1, row2};
    char *factorization;

    TEST_SUITE("flood fill / prime factorization / integer strings");
    flood_fill(0, 0, 7, map, 3, 3);
    TEST_ASSERT_EQ(row0[0], 7);
    TEST_ASSERT_EQ(row0[1], 7);
    TEST_ASSERT_EQ(row0[2], 1);
    TEST_ASSERT_EQ(row1[0], 7);
    TEST_ASSERT_EQ(row1[1], 1);
    TEST_ASSERT_EQ(row1[2], 7);
    TEST_ASSERT_EQ(row2[0], 7);
    TEST_ASSERT_EQ(row2[1], 7);
    TEST_ASSERT_EQ(row2[2], 7);
    flood_fill(-1, 0, 9, map, 3, 3);
    TEST_ASSERT_EQ(row0[0], 7);

    factorization = fprime(12);
    TEST_ASSERT_STREQ(factorization, "223");
    free(factorization);
    factorization = fprime(13);
    TEST_ASSERT_STREQ(factorization, "13");
    free(factorization);
    factorization = fprime(1);
    TEST_ASSERT_STREQ(factorization, "1");
    free(factorization);

    factorization = ft_itoa(0);
    TEST_ASSERT_STREQ(factorization, "0");
    free(factorization);
    factorization = ft_itoa(-42);
    TEST_ASSERT_STREQ(factorization, "-42");
    free(factorization);
    factorization = ft_itoa(1234);
    TEST_ASSERT_STREQ(factorization, "1234");
    free(factorization);
}

static void test_linked_lists(void)
{
    int a;
    int b;
    int c;
    list_t third;
    list_t second;
    list_t first;
    list_t remove_first;
    list_t remove_last;
    list_t *removed;
    int one;
    int two;
    int three;

    TEST_SUITE("linked-list traversal / removal / sorting");
    a = 1;
    b = 2;
    c = 3;
    first.content = &a;
    first.next = &second;
    second.content = &b;
    second.next = &third;
    third.content = &c;
    third.next = NULL;
    ft_list_foreach(&first, increment_content);
    TEST_ASSERT_EQ(a, 2);
    TEST_ASSERT_EQ(b, 3);
    TEST_ASSERT_EQ(c, 4);

    one = 1;
    two = 2;
    three = 3;
    removed = (list_t *)malloc(sizeof(*removed));
    TEST_ASSERT(removed != NULL);
    removed->content = &two;
    removed->next = &remove_last;
    remove_first.content = &one;
    remove_first.next = removed;
    remove_last.content = &three;
    remove_last.next = NULL;
    ft_list_remove_if(&remove_first, remove_value_two, ignore_content);
    TEST_ASSERT(remove_first.next == &remove_last);
    TEST_ASSERT(remove_last.next == NULL);

    a = 3;
    b = 1;
    c = 2;
    first.content = &a;
    first.next = &second;
    second.content = &b;
    second.next = &third;
    third.content = &c;
    third.next = NULL;
    sort_list(&first);
    TEST_ASSERT_EQ(*(int *)first.content, 1);
    TEST_ASSERT_EQ(*(int *)first.next->content, 2);
    TEST_ASSERT_EQ(*(int *)first.next->next->content, 3);
}

static void test_string_arrays_and_transforms(void)
{
    char **parts;
    char reversed_words[] = "hello world";
    char rotated[] = "abcd";
    char output[64];
    int numbers[] = {4, -1, 3, 0};
    int expected[] = {-1, 0, 3, 4};
    t_test_capture capture;
    int index;

    TEST_SUITE("split / word operations / array sorting");
    parts = ft_split(",,alpha,beta,,", ',');
    TEST_ASSERT(parts != NULL);
    TEST_ASSERT_STREQ(parts[0], "alpha");
    TEST_ASSERT_STREQ(parts[1], "beta");
    TEST_ASSERT(parts[2] == NULL);
    free_string_array(parts);

    TEST_ASSERT(rev_wstr(reversed_words) == reversed_words);
    TEST_ASSERT_STREQ(reversed_words, "world hello");

    test_capture_begin(&capture, output, sizeof(output));
    rostring(rotated);
    test_capture_end(&capture);
    TEST_ASSERT_STREQ(rotated, "bcda");
    TEST_ASSERT_STREQ(output, "bcda\n");

    sort_int_tab(numbers, 4);
    index = 0;
    while (index < 4)
    {
        TEST_ASSERT_EQ(numbers[index], expected[index]);
        index++;
    }
    sort_int_tab(numbers, 0);
    TEST_ASSERT_EQ(numbers[0], -1);
}

int main(void)
{
    TEST_BEGIN();
    test_flood_fill_and_factorization();
    test_linked_lists();
    test_string_arrays_and_transforms();
    TEST_FINISH();
}
