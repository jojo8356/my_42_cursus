/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   pipex_here_doc_bonus.c                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/08 12:00:00 by jojo8356          #+#    #+#             */
/*   Updated: 2026/11/08 12:00:00 by jojo8356         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "pipex_bonus.h"

/*
** here_doc, like the shell's `<<`:
**   - read stdin line by line until a line contains exactly the limiter,
**   - write everything read *before* the limiter into a pipe,
**   - the limiter line itself is never transmitted,
**   - EOF on stdin also stops the loop (no infinite loop on a closed stdin).
**
** The comparison is `strncmp(line, limiter, len(line)) == 0` on the line
** including its '\n' removed by px_rstrip(): "HELLO\n" stops on limiter
** "HELLO", but "HELLOX\n" does not.
*/

static char	*px_rstrip(char *line, ssize_t len)
{
	while (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r'))
	{
		line[len - 1] = '\0';
		len--;
	}
	return (line);
}

static int	px_is_limiter(const char *line, const char *limiter)
{
	size_t	llen;

	llen = strlen(line);
	return (strlen(limiter) == llen && !strncmp(line, limiter, llen));
}

/*
** Writes into `out_fd` every line read before the limiter. Returns 0 on
** success, -1 on a read or write error.
*/
int	px_read_until(const char *limiter, int out_fd)
{
	char	*line;
	size_t	cap;
	ssize_t	len;

	line = NULL;
	cap = 0;
	len = 1;
	while (len > 0)
	{
		len = getline(&line, &cap, stdin);
		if (len < 0)
			break ;
		px_rstrip(line, len);
		if (px_is_limiter(line, limiter))
			break ;
		write(out_fd, line, strlen(line));
		write(out_fd, "\n", 1);
	}
	free(line);
	if (len < 0 && !feof(stdin))
		return (-1);
	return (0);
}

/*
** Collects the here_doc body into an anonymous pipe and returns its read end.
** A pipe is used rather than a temporary file: nothing is written on disk, so
** the project leaves no stray file behind (the subject only allows unlink()
** for those who do it that way).
*/
int	px_here_doc_file(const char *limiter)
{
	int	pfd[2];
	int	ret;

	if (pipe(pfd) < 0)
		return (-1);
	ret = px_read_until(limiter, pfd[1]);
	close(pfd[1]);
	if (ret < 0)
	{
		close(pfd[0]);
		return (-1);
	}
	return (pfd[0]);
}
