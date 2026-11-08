/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   pipex_path.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/08 12:00:00 by jojo8356          #+#    #+#             */
/*   Updated: 2026/11/08 12:00:00 by jojo8356         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "pipex.h"

/*
** PATH lookup, the way the shell does it:
**   1. a command containing a '/' is used as-is (no PATH search),
**   2. otherwise every PATH entry is tried in order,
**   3. the first entry that is a regular executable file wins.
** access(X_OK) is not enough: a directory passes access(X_OK) but execve()
** fails on it with EACCES, so the stat check is required to reproduce the
** shell's "command not found" instead of a misleading "Permission denied".
*/

char	*px_path_value(char **envp)
{
	size_t	i;
	size_t	len;

	i = 0;
	while (envp && envp[i])
	{
		len = 0;
		while (envp[i][len] && envp[i][len] != '=')
			len++;
		if (len == 4 && !strncmp(envp[i], "PATH", 4))
			return (envp[i] + 5);
		i++;
	}
	return (NULL);
}

int	px_is_executable(const char *path)
{
	if (access(path, X_OK) != 0)
		return (0);
	if (access(path, F_OK) != 0)
		return (0);
	return (1);
}

static char	*px_join_path(const char *dir, const char *cmd)
{
	size_t	dlen;
	size_t	clen;
	char	*full;
	size_t	i;

	dlen = strlen(dir);
	clen = strlen(cmd);
	full = malloc(dlen + clen + 2);
	if (!full)
		return (NULL);
	i = 0;
	while (i < dlen)
	{
		full[i] = dir[i];
		i++;
	}
	full[i++] = '/';
	i = 0;
	while (i < clen)
	{
		full[i + dlen + 1] = cmd[i];
		i++;
	}
	full[dlen + clen + 1] = '\0';
	return (full);
}

/*
** strdup() is not in the subject's external functions list (only malloc and
** free are), so the copy is done by hand.
*/
static char	*px_strdup(const char *str)
{
	size_t	len;
	char	*copy;

	len = strlen(str);
	copy = malloc(len + 1);
	if (!copy)
		return (NULL);
	while (len + 1)
	{
		copy[len] = str[len];
		len--;
	}
	return (copy);
}

char	*px_resolve(const char *cmd, char **envp)
{
	char	**dirs;
	char	*candidate;
	char	*path;
	size_t	i;

	if (!cmd || !*cmd)
		return (NULL);
	if (strchr(cmd, '/'))
	{
		if (px_is_executable(cmd))
			return (px_strdup(cmd));
		return (NULL);
	}
	path = px_path_value(envp);
	if (!path)
		path = "/usr/local/bin:/usr/bin:/bin";
	dirs = px_split(path, ':');
	if (!dirs)
		return (NULL);
	i = 0;
	candidate = NULL;
	while (dirs[i])
	{
		if (candidate)
			free(candidate);
		candidate = px_join_path(dirs[i], cmd);
		if (candidate && px_is_executable(candidate))
			return (px_free_tab(dirs), candidate);
		i++;
	}
	free(candidate);
	px_free_tab(dirs);
	return (NULL);
}
