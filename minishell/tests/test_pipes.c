/* ************************************************************************** */
/*                                                                          */
/*                                                        :::      ::::::::   */
/*   test_pipes.c                                           :+:      :+:    :+:   */
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

    TEST("simple pipe");
    out = run_minishell("echo hello | cat");
    CHECK_CONTAINS("simple pipe works", out, "hello");
    free(out);

    TEST("two pipes");
    out = run_minishell("echo hello | cat | cat");
    CHECK_CONTAINS("two pipes work", out, "hello");
    free(out);

    TEST("pipe with builtin");
    out = run_minishell("echo a | echo b");
    CHECK_CONTAINS("pipe with builtin", out, "b");
    free(out);

    TEST("pipe with external");
    out = run_minishell("echo a | tr a-z A-Z");
    CHECK_CONTAINS("pipe with external", out, "A");
    free(out);

    TEST("pipe multiple args");
    out = run_minishell("echo a b c | cat");
    CHECK_CONTAINS("pipe multiple args", out, "a b c");
    free(out);

    print_summary();
    return (g_fail == 0 ? 0 : 1);
}
