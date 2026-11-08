/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   pipex_exec.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/08 12:00:00 by jojo8356          #+#    #+#             */
/*   Updated: 2026/11/08 12:00:00 by jojo8356         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "pipex.h"

/*
** Called in the child, after the fds are in place. It never returns: either
** execve() replaces the process, or the child prints the shell-like error and
** exits with 127 ("command not found") or 126 ("permission denied").
**
** The path is resolved *after* fork() on purpose: the lookup allocates, and
** doing it before the fork would leak the allocation in every child.
*/

void	px_exec(char *const argv[], char **envp)
{
	char	*path;

	if (!argv || !argv[0] || !*argv[0])
		exit(EXIT_CMD);
	path = px_resolve(argv[0], envp);
	if (!path)
	{
		if (strchr(argv[0], '/'))
			px_error(argv[0], NULL, EXIT_NOEXEC);
		else
			px_error(NULL, argv[0], EXIT_CMD);
		exit(EXIT_CMD);
	}
	execve(path, argv, envp);
	px_error(path, NULL, EXIT_NOEXEC);
	free(path);
	exit(EXIT_NOEXEC);
}
