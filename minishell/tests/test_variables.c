/* ************************************************************************** */
/*                                                                          */
/*                                                        :::      ::::::::   */
/*   test_variables.c                                       :+:      :+:    :+:   */
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

    TEST("variable expansion");
    out = run_minishell("export TESTVAR=hello\necho $TESTVAR");
    CHECK_CONTAINS("variable expanded", out, "hello");
    free(out);

    TEST("unset variable");
    out = run_minishell("echo $NONEXISTENTVAR");
    CHECK_CONTAINS("unset variable empty", out, "minishell>");
    free(out);

    TEST("exit code $?");
    out = run_minishell("echo hello\necho $?");
    CHECK_CONTAINS("exit code 0", out, "0");
    free(out);

    TEST("multiple variables");
    out = run_minishell("export A=1\nexport B=2\necho $A$B");
    CHECK_CONTAINS("multiple variables", out, "12");
    free(out);

    TEST("variable in quotes");
    out = run_minishell("export NAME=world\necho \"hello $NAME\"");
    CHECK_CONTAINS("variable in double quotes", out, "hello world");
    free(out);

    TEST("variable in single quotes");
    out = run_minishell("export NAME=world\necho 'hello $NAME'");
    CHECK_CONTAINS("variable in single quotes NOT expanded", out, "$NAME");
    free(out);

    print_summary();
    return (g_fail == 0 ? 0 : 1);
}
