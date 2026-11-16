/* ************************************************************************** */
/*                                                                          */
/*                                                        :::      ::::::::   */
/*   test_framework.h                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/12 15:45:00 by jojo8356          #+#    #+#             */
/*   Updated: 2026/11/12 15:45:00 by jojo8356         ###   ########.fr       */
/*                                                                          */
/* ************************************************************************** */

# ifndef TEST_FRAMEWORK_H
# define TEST_FRAMEWORK_H

# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <unistd.h>
# include <sys/wait.h>

# define GREEN "\033[0;32m"
# define RED "\033[0;31m"
# define YELLOW "\033[1;33m"
# define BLUE "\033[0;34m"
# define NC "\033[0m"

# define PROMPT "minishell> "
# define PROMPT_LEN 12

static int g_pass = 0;
static int g_fail = 0;
static int g_total = 0;

# define TEST(name) \
    printf(BLUE "=== %s ===\n" NC, name);

# define CHECK(desc, actual, expected) \
    do { \
        g_total++; \
        if (strcmp(actual, expected) == 0) \
        { \
            g_pass++; \
            printf(GREEN "✓" NC " %s\n", desc); \
        } \
        else \
        { \
            g_fail++; \
            printf(RED "✗" NC " %s\n", desc); \
            printf(YELLOW "  Expected: %s\n" NC, expected); \
            printf(YELLOW "  Actual:   %s\n" NC, actual); \
        } \
    } while (0)

# define CHECK_INT(desc, actual, expected) \
    do { \
        g_total++; \
        if (actual == expected) \
        { \
            g_pass++; \
            printf(GREEN "✓" NC " %s\n", desc); \
        } \
        else \
        { \
            g_fail++; \
            printf(RED "✗" NC " %s\n", desc); \
            printf(YELLOW "  Expected: %d\n" NC, expected); \
            printf(YELLOW "  Actual:   %d\n" NC, actual); \
        } \
    } while (0)

# define CHECK_CONTAINS(desc, actual, expected) \
    do { \
        g_total++; \
        if (strstr(actual, expected) != NULL) \
        { \
            g_pass++; \
            printf(GREEN "✓" NC " %s\n", desc); \
        } \
        else \
        { \
            g_fail++; \
            printf(RED "✗" NC " %s\n", desc); \
            printf(YELLOW "  Expected to contain: %s\n" NC, expected); \
            printf(YELLOW "  Actual: %s\n" NC, actual); \
        } \
    } while (0)

static char *run_minishell(char *input)
{
    char cmd[2048];
    char *output;
    FILE *fp;
    FILE *infile;

    infile = fopen("/tmp/ms_test_input.txt", "w");
    if (!infile)
        return (NULL);
    fputs(input, infile);
    fclose(infile);

    snprintf(cmd, sizeof(cmd), "./minishell < /tmp/ms_test_input.txt 2>&1");
    output = (char *)malloc(8192);
    fp = popen(cmd, "r");
    if (!fp)
    {
        free(output);
        return (NULL);
    }
    {
        size_t bytes_read = fread(output, 1, 8191, fp);
        output[bytes_read] = '\0';
    }
    pclose(fp);
    remove("/tmp/ms_test_input.txt");
    return (output);
}

static void print_summary()
{
    printf("\n");
    printf("=== %s ===\n", "SUMMARY");
    printf("Passed: %d/%d\n", g_pass, g_total);
    if (g_fail > 0)
        printf("Failed: %d/%d\n", g_fail, g_total);
    else
        printf("Failed: 0/%d\n", g_total);
    if (g_fail == 0)
        printf(GREEN "ALL TESTS PASSED!\n" NC);
    else
        printf(RED "SOME TESTS FAILED!\n" NC);
}

# endif
