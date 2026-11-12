/* ************************************************************************** */
/*                                                                          */
/*                                                        :::      ::::::::   */
/*   5_builtins.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/12 15:45:00 by jojo8356          #+#    #+#             */
/*   Updated: 2026/11/12 15:45:00 by jojo8356         ###   ########.fr       */
/*                                                                          */
/* ************************************************************************** */

# include "../includes/minishell.h"

static char *g_builtin_names[] = {
    "echo", "cd", "pwd", "export", "unset", "env", "exit", NULL
};

int is_builtin(char *name)
{
    int i;

    if (!name)
        return (0);
    i = 0;
    while (g_builtin_names[i])
    {
        if (ft_strcmp(name, g_builtin_names[i]) == 0)
            return (1);
        i++;
    }
    return (0);
}

int exec_builtin(cmd_t *cmd, int *exit_code)
{
    char *name;

    if (!cmd->argv || !cmd->argv[0])
    {
        *exit_code = 0;
        return (0);
    }
    name = cmd->argv[0];
    if (ft_strcmp(name, "echo") == 0)
        *exit_code = exec_echo(cmd->argv);
    else if (ft_strcmp(name, "cd") == 0)
        *exit_code = exec_cd(cmd->argv);
    else if (ft_strcmp(name, "pwd") == 0)
        *exit_code = exec_pwd(cmd->argv);
    else if (ft_strcmp(name, "export") == 0)
        *exit_code = exec_export(cmd->argv);
    else if (ft_strcmp(name, "unset") == 0)
        *exit_code = exec_unset(cmd->argv);
    else if (ft_strcmp(name, "env") == 0)
        *exit_code = exec_env(cmd->argv);
    else if (ft_strcmp(name, "exit") == 0)
    {
        *exit_code = exec_exit(cmd->argv);
        request_exit(*exit_code);
    }
    else
        *exit_code = 0;
    return (*exit_code);
}
