/* ************************************************************************** */
/*                                                                          */
/*                                                        :::      ::::::::   */
/*   test_heredoc.c                                         :+:      :+:    :+:   */
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

    TEST("simple heredoc");
    out = run_minishell("cat << EOF\nhello heredoc\nEOF");
    CHECK_CONTAINS("heredoc content printed", out, "hello heredoc");
    free(out);

    TEST("heredoc with multiple lines");
    out = run_minishell("cat << EOF\nline1\nline2\nline3\nEOF");
    CHECK_CONTAINS("heredoc line1", out, "line1");
    CHECK_CONTAINS("heredoc line2", out, "line2");
    CHECK_CONTAINS("heredoc line3", out, "line3");
    free(out);

    TEST("heredoc with command");
    out = run_minishell("tr a-z A-Z << EOF\nhello\nEOF");
    CHECK_CONTAINS("heredoc with command", out, "HELLO");
    free(out);

    print_summary();
    return (g_fail == 0 ? 0 : 1);
}
