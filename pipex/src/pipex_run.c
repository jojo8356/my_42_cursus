/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   pipex_run.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/08 12:00:00 by jojo8356          #+#    #+#             */
/*   Updated: 2026/11/08 12:00:00 by jojo8356         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "pipex.h"

/*
** Mandatory part: exactly one pipe, two commands.
**      ./pipex infile cmd1 cmd2 outfile
** must behave like
**          < infile cmd1 | cmd2 > outfile
**
** Each redirection is performed by the child that owns it, at the moment that
** command starts, exactly like the shell:
**
**   - a failing `< infile` only kills cmd1. cmd2 still runs, on an empty
**     stream, so `./pipex nope cat "wc -l" out` writes "0" and returns 0 --
**     which is what `< nope cat | wc -l > out` does;
**   - a failing `> outfile` only kills cmd2. cmd1 still runs (and its output is
**     discarded), so the timing of the first command is preserved;
**   - the exit status is the one of the last command: bash has no pipefail.
**
** Opening the files in the parent instead would make both commands run with a
** redirect that the shell would have refused, and would skip cmd1 whenever the
** output file is unwritable. That is why the opens live in the children.
*/

static void	px_child_first(char *infile, char *cmd, int pfd[2], char **envp)
{
	int		fd;
	char	**argv;

	signal(SIGPIPE, SIG_DFL);
	if (!*infile)
	{
		errno = ENOENT;
		fd = -1;
	}
	else
		fd = open(infile, O_RDONLY);
	if (fd < 0)
	{
		px_error(infile, NULL, EXIT_USAGE);
		close(pfd[1]);
		close(pfd[0]);
		exit(EXIT_USAGE);
	}
	argv = px_split(cmd, ' ');
	if (!argv || dup2(fd, STDIN_FILENO) < 0
		|| dup2(pfd[1], STDOUT_FILENO) < 0)
		exit(EXIT_USAGE);
	close(fd);
	close(pfd[0]);
	close(pfd[1]);
	px_exec(argv, envp);
}

static void	px_child_second(char *outfile, char *cmd, int pfd[2], char **envp)
{
	int		fd;
	char	**argv;

	signal(SIGPIPE, SIG_DFL);
	if (!*outfile)
	{
		errno = ENOENT;
		fd = -1;
	}
	else
		fd = open(outfile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if (fd < 0)
	{
		px_error(outfile, NULL, EXIT_USAGE);
		close(pfd[0]);
		exit(EXIT_USAGE);
	}
	argv = px_split(cmd, ' ');
	if (!argv || dup2(pfd[0], STDIN_FILENO) < 0
		|| dup2(fd, STDOUT_FILENO) < 0)
		exit(EXIT_USAGE);
	close(fd);
	close(pfd[0]);
	close(pfd[1]);
	px_exec(argv, envp);
}

static int	px_wait_last(pid_t first, pid_t last)
{
	int	status;

	if (waitpid(first, &status, 0) < 0)
		return (EXIT_USAGE);
	if (waitpid(last, &status, 0) < 0)
		return (EXIT_USAGE);
	if (WIFEXITED(status))
		return (WEXITSTATUS(status));
	if (WIFSIGNALED(status))
		return (128 + WTERMSIG(status));
	return (EXIT_USAGE);
}

int	px_pipeline(int ac, char **av, char **envp)
{
	int		pfd[2];
	pid_t	pids[2];

	if (ac != 5)
		return (px_usage(), EXIT_USAGE);
	if (pipe(pfd) < 0)
		return (px_error("pipe", NULL, EXIT_USAGE), EXIT_USAGE);
	pids[0] = fork();
	if (pids[0] == 0)
		px_child_first(av[1], av[2], pfd, envp);
	pids[1] = fork();
	if (pids[1] == 0)
		px_child_second(av[4], av[3], pfd, envp);
	close(pfd[0]);
	close(pfd[1]);
	return (px_wait_last(pids[0], pids[1]));
}
