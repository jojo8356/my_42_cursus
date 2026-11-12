/* ************************************************************************** */
/*                                                                          */
/*                                                        :::      ::::::::   */
/*   test_builtins.c                                         :+:      :+:    :+:   */
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

    TEST("cd to /tmp and pwd");
    out = run_minishell("cd /tmp\npwd");
    CHECK_CONTAINS("cd changes directory", out, "/tmp");
    free(out);

    TEST("cd back with -");
    out = run_minishell("cd /tmp\ncd -\npwd");
    CHECK_CONTAINS("cd - returns to previous", out, "my_42_cursus");
    free(out);

    TEST("export variable");
    out = run_minishell("export MSVAR=testvalue\necho $MSVAR");
    CHECK_CONTAINS("export sets variable", out, "testvalue");
    free(out);

    TEST("unset variable");
    out = run_minishell("export MSVAR=testvalue\nunset MSVAR\necho $MSVAR");
    CHECK_CONTAINS("unset removes variable", out, "minishell>");
    free(out);

    TEST("env shows variables");
    out = run_minishell("export MSTEST=hello\nenv");
    CHECK_CONTAINS("env shows exported var", out, "MSTEST=hello");
    free(out);

    TEST("exit with code");
    out = run_minishell("exit 42");
    CHECK_CONTAINS("exit exits", out, "minishell>");
    free(out);

    print_summary();
    return (g_fail == 0 ? 0 : 1);
}
