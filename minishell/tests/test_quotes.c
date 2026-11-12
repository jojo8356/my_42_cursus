/* ************************************************************************** */
/*                                                                          */
/*                                                        :::      ::::::::   */
/*   test_quotes.c                                          :+:      :+:    :+:   */
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

    TEST("single quotes");
    out = run_minishell("echo 'hello world'");
    CHECK_CONTAINS("single quotes preserved", out, "hello world");
    free(out);

    TEST("double quotes");
    out = run_minishell("echo \"hello world\"");
    CHECK_CONTAINS("double quotes stripped", out, "hello world");
    free(out);

    TEST("nested quotes");
    out = run_minishell("echo 'hello \"world\"'");
    CHECK_CONTAINS("nested quotes", out, "hello \"world\"");
    free(out);

    TEST("empty quotes");
    out = run_minishell("echo ''");
    CHECK_CONTAINS("empty quotes empty output", out, "minishell>");
    free(out);

    TEST("quotes with spaces");
    out = run_minishell("echo 'a b c'");
    CHECK_CONTAINS("quotes with spaces", out, "a b c");
    free(out);

    TEST("double quotes with spaces");
    out = run_minishell("echo \"a b c\"");
    CHECK_CONTAINS("double quotes with spaces", out, "a b c");
    free(out);

    TEST("unclosed single quote");
    out = run_minishell("echo 'hello");
    CHECK_CONTAINS("unclosed quote kept", out, "hello");
    free(out);

    TEST("unclosed double quote");
    out = run_minishell("echo \"hello");
    CHECK_CONTAINS("unclosed double quote kept", out, "hello");
    free(out);

    print_summary();
    return (g_fail == 0 ? 0 : 1);
}
