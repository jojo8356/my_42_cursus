/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   pipex_run_bonus.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/08 12:00:00 by jojo8356          #+#    #+#             */
/*   Updated: 2026/11/08 12:00:00 by jojo8356         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "pipex_bonus.h"

/*
** Bonus part.
**
** Multiple pipes:
**      ./pipex file1 cmd1 cmd2 cmd3 ... cmdn file2
** is `< file1 cmd1 | cmd2 | cmd3 | ... | cmdn > file2`.
**
** here_doc:
**      ./pipex here_doc LIMITER cmd1 cmd2 ... cmdn file
** is `cmd1 << LIMITER | cmd2 | ... | cmdn >> file`.
**
** Three decisions make or break this file:
**
**   1. The parent closes the write end of each pipe right after the fork that
**      created it. If it keeps one open, the reader never gets EOF and the
**      programme hangs: that is the pipex deadlock.
**   2. here_doc is read by the parent, before any fork. If the children read
**      stdin themselves they compete for the same lines and the body is split
**      randomly between them.
**   3. A failing `< infile` does not stop the chain (bash keeps going), but the
**      first command is skipped because its redirect failed. Every following
**      command must then read an empty stream, not inherit pipex's own stdin.
**      That is why the fallback is /dev/null and not "leave fd 0 alone".
*/



/*
** Returns the status of the last command: a shell pipeline without pipefail
** reports nothing else. pids[] is filled in command order.
*/
static int	px_reap(pid_t *pids, int n)
{
	int	i;
	int	status;
	int	ret;

	i = 0;
	ret = EXIT_USAGE;
	while (i < n)
	{
		if (waitpid(pids[i], &status, 0) < 0)
			return (free(pids), EXIT_USAGE);
		if (i == n - 1)
		{
			if (WIFEXITED(status))
				ret = WEXITSTATUS(status);
			else if (WIFSIGNALED(status))
				ret = 128 + WTERMSIG(status);
		}
		i++;
	}
	free(pids);
	return (ret);
}

static int	px_spawn(t_px *px)
{
	pid_t	*pids;
	int		pfd[2];
	int		i;

	pids = malloc(sizeof(pid_t) * (size_t)px->ncmd);
	if (!pids)
		return (EXIT_USAGE);
	i = 0;
	while (i < px->ncmd)
	{
		pfd[0] = -1;
		pfd[1] = -1;
		if (i < px->ncmd - 1 && pipe(pfd) < 0)
			return (free(pids), EXIT_USAGE);
		pids[i] = fork();
		if (pids[i] < 0)
			return (free(pids), EXIT_USAGE);
		if (pids[i] == 0)
			px_child(px, pfd, i);
		if (i < px->ncmd - 1)
			close(pfd[1]);
		if (px->prev >= 0 && px->prev != px->in_fd)
			close(px->prev);
		px->prev = pfd[0];
		i++;
	}
	return (px_reap(pids, px->ncmd));
}

static int	px_setup_in(t_px *px, char *file)
{
	if (!*file)
	{
		errno = ENOENT;
		px->prev = -1;
	}
	else
		px->prev = open(file, O_RDONLY);
	if (px->prev < 0)
	{
		px_error(file, NULL, EXIT_USAGE);
		px->prev = open("/dev/null", O_RDONLY);
		if (px->prev < 0)
			return (-1);
	}
	return (0);
}

int	px_run_bonus(int ac, char **av, char **envp)
{
	t_px	px;
	int		first;

	memset(&px, 0, sizeof(px));
	px.envp = envp;
	px.in_fd = -1;
	px.prev = -1;
	first = 2;
	/*
	** ./pipex infile cmd1 cmd2 outfile is 5 arguments, the smallest valid
	** form; `./pipex in cat cat` is a mistake and must be refused *before*
	** anything is opened, otherwise "cat" would be created as the output
	** file and an empty outfile would appear in the user's directory.
	*/
	if (ac < 5)
		return (px_usage(), EXIT_USAGE);
	if (ac >= 5 && !strcmp(av[1], "here_doc"))
	{
		px.in_fd = px_here_doc_file(av[2]);
		if (px.in_fd < 0)
			return (EXIT_USAGE);
		px.append = 1;
		first = 3;
	}
	if (ac - first - 1 < 1)
		return (px_usage(), EXIT_USAGE);
	px.argv = av + first;
	px.ncmd = ac - first - 1;
	px.out_file = av[ac - 1];
	if (px.in_fd >= 0)
		px.prev = px.in_fd;
	else if (px_setup_in(&px, av[1]) < 0)
		return (EXIT_USAGE);
	return (px_spawn(&px));
}
