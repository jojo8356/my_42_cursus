/* ************************************************************************** */
/*                                                                          */
/*                                                        :::      ::::::::   */
/*   4_executor.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/12 15:45:00 by jojo8356          #+#    #+#             */
/*   Updated: 2026/11/12 15:45:00 by jojo8356         ###   ########.fr       */
/*                                                                          */
/* ************************************************************************** */

# include "../includes/minishell.h"

extern char **environ;

static char *find_executable(char *name, char **paths)
{
    int i;
    char *path;
    char *full_path;

    if (ft_strchr(name, '/'))
        return (ft_strdup(name));
    i = 0;
    while (paths[i])
    {
        path = paths[i];
        full_path = ft_strjoin(path, "/");
        full_path = ft_strjoin(full_path, name);
        if (access(full_path, X_OK) == 0)
            return (full_path);
        free(full_path);
        i++;
    }
    return (NULL);
}

static char **split_path(char *path_str)
{
    char **paths;
    char *path;
    int i;

    if (!path_str)
        return (NULL);
    paths = ft_split(path_str, ':');
    i = 0;
    while (paths && paths[i])
    {
        path = ft_strdup(paths[i]);
        free(paths[i]);
        paths[i] = path;
        i++;
    }
    return (paths);
}

static void free_paths(char **paths)
{
    int i;

    if (!paths)
        return;
    i = 0;
    while (paths[i])
    {
        free(paths[i]);
        i++;
    }
    free(paths);
}

int execute_external(cmd_t *cmd)
{
    char *path_str;
    char **paths;
    char **argv;
    char *full_path;

    argv = cmd->argv;
    if (!argv || !argv[0])
        return (127);
    path_str = getenv("PATH");
    paths = split_path(path_str);
    full_path = find_executable(argv[0], paths);
    free_paths(paths);
    if (!full_path)
    {
        ft_putstr_fd("minishell: ", 2);
        ft_putstr_fd(argv[0], 2);
        ft_putendl_fd(": command not found", 2);
        return (127);
    }
    free(argv[0]);
    argv[0] = full_path;
    execve(argv[0], argv, environ);
    ft_putstr_fd("minishell: ", 2);
    ft_putstr_fd(argv[0], 2);
    ft_putendl_fd(": Permission denied", 2);
    return (126);
}
