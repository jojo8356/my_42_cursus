/* ************************************************************************** */
/*                                                                          */
/*                                                        :::      ::::::::   */
/*   test_basic.c                                           :+:      :+:    :+:   */
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

    TEST("echo simple");
    out = run_minishell("echo hello");
    CHECK_CONTAINS("echo prints argument", out, "hello");
    free(out);

    TEST("echo -n");
    out = run_minishell("echo -n hello");
    CHECK_CONTAINS("echo -n no newline", out, "hello");
    free(out);

    TEST("echo multiple args");
    out = run_minishell("echo a b c");
    CHECK_CONTAINS("echo multiple args", out, "a b c");
    free(out);

    TEST("echo empty");
    out = run_minishell("echo");
    CHECK_CONTAINS("echo empty line", out, "\n");
    free(out);

    TEST("pwd");
    out = run_minishell("pwd");
    CHECK_CONTAINS("pwd returns path", out, "/");
    free(out);

    TEST("env");
    out = run_minishell("env");
    CHECK_CONTAINS("env shows SHELL", out, "SHELL=");
    free(out);

    TEST("exit");
    out = run_minishell("exit 0");
    CHECK_CONTAINS("exit exits shell", out, "minishell>");
    free(out);

    TEST("unknown command");
    out = run_minishell("nonexistentcommand123");
    CHECK_CONTAINS("unknown command error", out, "command not found");
    free(out);

    print_summary();
    return (g_fail == 0 ? 0 : 1);
}
