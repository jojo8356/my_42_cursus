/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   pipex_child_bonus.c                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/08 12:00:00 by jojo8356          #+#    #+#             */
/*   Updated: 2026/11/08 12:00:00 by jojo8356         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "pipex_bonus.h"

/*
** The child side of the bonus pipeline.
**
** px_close_ends() closes every descriptor the child inherited but does not
** use. This is the single most important function of the project: forgetting
** one write end of the pipe leaves the reader waiting for EOF forever, and
** the programme hangs instead of finishing^C-free.
*/

void	px_close_ends(t_px *px, int pfd[2], int last)
{
	close(pfd[0]);
	if (!last)
		close(pfd[1]);
	if (px->prev >= 0 && px->prev != px->in_fd)
		close(px->prev);
}

/*
** The output file is opened by the last child, not by the parent: an
** unwritable path must kill cmd_n only, and let the earlier commands run in
** full, exactly like a failing `> outfile` in a shell.
*/
int	px_child_out(t_px *px)
{
	int	fd;

	if (!*px->out_file)
	{
		errno = ENOENT;
		fd = -1;
	}
	else if (px->append)
		fd = open(px->out_file, O_WRONLY | O_CREAT | O_APPEND, 0644);
	else
		fd = open(px->out_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if (fd < 0)
		px_error(px->out_file, NULL, EXIT_USAGE);
	return (fd);
}

void	px_child(t_px *px, int pfd[2], int idx)
{
	char	**cmd;
	int		last;
	int		fd;

	signal(SIGPIPE, SIG_DFL);
	last = (idx == px->ncmd - 1);
	cmd = px_split(px->argv[idx], ' ');
	if (!cmd)
		exit(EXIT_USAGE);
	if (px->prev >= 0 && dup2(px->prev, STDIN_FILENO) < 0)
		exit(EXIT_USAGE);
	if (last)
	{
		fd = px_child_out(px);
		if (fd < 0 || dup2(fd, STDOUT_FILENO) < 0)
			exit(EXIT_USAGE);
		close(fd);
	}
	else if (dup2(pfd[1], STDOUT_FILENO) < 0)
		exit(EXIT_USAGE);
	px_close_ends(px, pfd, last);
	px_exec(cmd, px->envp);
}

