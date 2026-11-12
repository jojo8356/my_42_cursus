/* ************************************************************************** */
/*                                                                          */
/*                                                        :::      ::::::::   */
/*   test_wildcards.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/12 15:45:00 by jojo8356          #+#    #+#             */
/*   Updated: 2026/11/12 15:45:00 by jojo8356         ###   ########.fr       */
/*                                                                          */
/* ************************************************************************** */

# include "test_framework.h"

int main(void)
{
    char *out;

    /* create test files in current directory */
    system("touch ms_wild1.txt ms_wild2.txt ms_wild3.dat");

    TEST("wildcard star");
    out = run_minishell("echo *");
    CHECK_CONTAINS("wildcard matches all", out, "ms_wild1.txt");
    CHECK_CONTAINS("wildcard matches all 2", out, "ms_wild2.txt");
    free(out);

    TEST("wildcard extension");
    out = run_minishell("echo *.txt");
    CHECK_CONTAINS("wildcard matches .txt", out, "ms_wild1.txt");
    CHECK_CONTAINS("wildcard matches .txt 2", out, "ms_wild2.txt");
    free(out);

    TEST("wildcard no match");
    out = run_minishell("echo *.xyz");
    CHECK_CONTAINS("wildcard no match keeps pattern", out, "*.xyz");
    free(out);

    TEST("wildcard prefix");
    out = run_minishell("echo ms_wild*");
    CHECK_CONTAINS("wildcard prefix matches", out, "ms_wild1.txt");
    CHECK_CONTAINS("wildcard prefix matches 3", out, "ms_wild3.dat");
    free(out);

    /* cleanup */
    system("rm -f ms_wild1.txt ms_wild2.txt ms_wild3.dat");

    print_summary();
    return (g_fail == 0 ? 0 : 1);
}
