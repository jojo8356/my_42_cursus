/* ************************************************************************** */
/*                                                                          */
/*                                                        :::      ::::::::   */
/*   test_logical.c                                         :+:      :+:    :+:   */
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

    TEST("AND operator");
    out = run_minishell("echo a && echo b");
    CHECK_CONTAINS("AND executes both", out, "a");
    CHECK_CONTAINS("AND executes second", out, "b");
    free(out);

    TEST("AND with failure");
    out = run_minishell("false && echo b");
    CHECK_CONTAINS("AND stops on failure - prompt shown", out, "minishell>");
    if (strstr(out, "b\n") != NULL)
    {
        g_total++;
        g_fail++;
        printf(RED "✗" NC " AND should not execute b\n");
    }
    else
    {
        g_total++;
        g_pass++;
        printf(GREEN "✓" NC " AND stops on failure - no b\n");
    }
    free(out);

    TEST("OR operator");
    out = run_minishell("false || echo b");
    CHECK_CONTAINS("OR executes on failure", out, "b");
    free(out);

    TEST("OR with success");
    out = run_minishell("echo a || echo b");
    CHECK_CONTAINS("OR stops on success - has a", out, "a");
    if (strstr(out, "b\n") != NULL)
    {
        g_total++;
        g_fail++;
        printf(RED "✗" NC " OR should not execute b\n");
    }
    else
    {
        g_total++;
        g_pass++;
        printf(GREEN "✓" NC " OR stops on success - no b\n");
    }
    free(out);

    TEST("parentheses");
    out = run_minishell("(echo a && echo b)");
    CHECK_CONTAINS("parentheses group", out, "a");
    CHECK_CONTAINS("parentheses group second", out, "b");
    free(out);

    TEST("nested parentheses");
    out = run_minishell("((echo a))");
    CHECK_CONTAINS("nested parentheses", out, "a");
    free(out);

    print_summary();
    return (g_fail == 0 ? 0 : 1);
}
