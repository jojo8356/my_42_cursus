/* ************************************************************************** */
/*                                                                          */
/*                                                        :::      ::::::::   */
/*   5_builtin_env.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/12 15:45:00 by jojo8356          #+#    #+#             */
/*   Updated: 2026/11/12 15:45:00 by jojo8356         ###   ########.fr       */
/*                                                                          */
/* ************************************************************************** */

# include "../includes/minishell.h"

extern char **environ;

static void print_environ(void)
{
    int i;

    i = 0;
    while (environ[i])
    {
        write(1, environ[i], ft_strlen(environ[i]));
        write(1, "\n", 1);
        i++;
    }
}

int exec_env(char **argv)
{
    (void)argv;
    print_environ();
    return (0);
}

int exec_export(char **argv)
{
    int i;

    if (!argv[1])
    {
        print_environ();
        return (0);
    }
    i = 1;
    while (argv[i])
    {
        putenv(ft_strdup(argv[i]));
        i++;
    }
    return (0);
}

int exec_unset(char **argv)
{
    int i;

    i = 1;
    while (argv[i])
    {
        unsetenv(argv[i]);
        i++;
    }
    return (0);
}
