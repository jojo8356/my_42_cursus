/* ************************************************************************** */
/*                                                                          */
/*                                                        :::      ::::::::   */
/*   5_builtin_cd.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/12 15:45:00 by jojo8356          #+#    #+#             */
/*   Updated: 2026/11/12 15:45:00 by jojo8356         ###   ########.fr       */
/*                                                                          */
/* ************************************************************************** */

# include "../includes/minishell.h"

static void update_pwd_env(char *old_pwd)
{
    char cwd[4096];
    char *oldpwd_str;

    if (getcwd(cwd, sizeof(cwd)))
    {
        putenv(ft_strjoin("PWD=", cwd));
        if (old_pwd)
        {
            oldpwd_str = ft_strjoin("OLDPWD=", old_pwd);
            putenv(oldpwd_str);
        }
    }
}

int exec_cd(char **argv)
{
    char *path;
    char *home;
    char *oldpwd;
    char *oldpwd_env;
    char *arg;

    oldpwd_env = getenv("PWD");
    arg = argv[1];
    if (!arg || ft_strcmp(arg, "") == 0 || ft_strcmp(arg, "~") == 0)
    {
        home = getenv("HOME");
        path = home ? home : "/";
    }
    else if (ft_strcmp(arg, "-") == 0)
    {
        oldpwd = getenv("OLDPWD");
        path = oldpwd ? oldpwd : getenv("HOME");
    }
    else
        path = arg;
    if (!path)
        return (1);
    if (chdir(path) == -1)
    {
        ft_putstr_fd("cd: no such file or directory: ", 2);
        ft_putendl_fd(path, 2);
        return (1);
    }
    update_pwd_env(oldpwd_env);
    return (0);
}
