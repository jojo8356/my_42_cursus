/* ************************************************************************** */
/*                                                                          */
/*                                                        :::      ::::::::   */
/*   5_builtin_echo.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/12 15:45:00 by jojo8356          #+#    #+#             */
/*   Updated: 2026/11/12 15:45:00 by jojo8356         ###   ########.fr       */
/*                                                                          */
/* ************************************************************************** */

# include "../includes/minishell.h"

int exec_echo(char **argv)
{
    int i;
    int no_newline;

    no_newline = 0;
    if (argv[1] && ft_strcmp(argv[1], "-n") == 0)
    {
        no_newline = 1;
        i = 2;
    }
    else
        i = 1;
    while (argv[i])
    {
        write(1, argv[i], ft_strlen(argv[i]));
        if (argv[i + 1])
            write(1, " ", 1);
        i++;
    }
    if (!no_newline)
        write(1, "\n", 1);
    return (0);
}
