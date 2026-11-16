/* ************************************************************************** */
/*                                                                          */
/*                                                        :::      ::::::::   */
/*   test_redirects.c                                        :+:      :+:    :+:   */
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

    TEST("output redirect");
    out = run_minishell("echo hello > /tmp/ms_test_out.txt");
    {
        FILE *fp = fopen("/tmp/ms_test_out.txt", "r");
        char buf[256];
        if (fp)
        {
            fgets(buf, sizeof(buf), fp);
            fclose(fp);
            CHECK("output redirect writes to file", buf, "hello\n");
        }
        else
            CHECK("output redirect file exists", "no", "yes");
    }
    free(out);

    TEST("append redirect");
    out = run_minishell("echo world >> /tmp/ms_test_out.txt");
    {
        FILE *fp = fopen("/tmp/ms_test_out.txt", "r");
        char buf[256];
        if (fp)
        {
            fgets(buf, sizeof(buf), fp);
            fclose(fp);
            CHECK("append redirect adds to file", buf, "hello\n");
        }
        else
            CHECK("append redirect file exists", "no", "yes");
    }
    free(out);

    TEST("input redirect");
    out = run_minishell("cat < /tmp/ms_test_out.txt");
    CHECK_CONTAINS("input redirect reads file", out, "hello");
    free(out);

    TEST("input redirect with command");
    out = run_minishell("tr a-z A-Z < /tmp/ms_test_out.txt");
    CHECK_CONTAINS("input redirect with command", out, "HELLO");
    free(out);

    TEST("redirect to nonexistent");
    out = run_minishell("echo hello > /nonexistent/path/file.txt");
    CHECK_CONTAINS("redirect error message", out, "Permission denied");
    free(out);

    remove("/tmp/ms_test_out.txt");

    print_summary();
    return (g_fail == 0 ? 0 : 1);
}
