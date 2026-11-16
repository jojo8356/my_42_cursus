/* ************************************************************************** */
/*                                                                          */
/*                                                        :::      ::::::::   */
/*   5_builtin_exit.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/12 15:45:00 by jojo8356          #+#    #+#             */
/*   Updated: 2026/11/12 15:45:00 by jojo8356         ###   ########.fr       */
/*                                                                          */
/* ************************************************************************** */

# include "../includes/minishell.h"

int exec_exit(char **argv)
{
    int code;

    if (!argv[1])
        return (0);
    code = ft_atoi(argv[1]);
    code = code % 256;
    if (code < 0)
        code = 256 + code;
    return (code);
}
