/* ************************************************************************** */
/*                                                                          */
/*                                                        :::      ::::::::   */
/*   5_builtin_pwd.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/12 15:45:00 by jojo8356          #+#    #+#             */
/*   Updated: 2026/11/12 15:45:00 by jojo8356         ###   ########.fr       */
/*                                                                          */
/* ************************************************************************** */

# include "../includes/minishell.h"

int exec_pwd(char **argv)
{
    char cwd[4096];

    (void)argv;
    if (getcwd(cwd, sizeof(cwd)))
    {
        write(1, cwd, ft_strlen(cwd));
        write(1, "\n", 1);
        return (0);
    }
    return (1);
}
