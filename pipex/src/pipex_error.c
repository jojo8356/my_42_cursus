/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   pipex_error.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/08 12:00:00 by jojo8356          #+#    #+#             */
/*   Updated: 2026/11/08 12:00:00 by jojo8356         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "pipex.h"

/*
** The subject says: "If you have any doubt, handle the errors like the shell
** command". A shell prints, for a failing command, one line on stderr:
**     <program>: <argument>: <strerror(errno)>
** and for a missing one:
**     <program>: <command>: command not found
** px_error() and px_usage() are the only functions of the project that write
** on stderr, so the error format is decided in exactly one place.
*/

static void	px_putstr(int fd, const char *str)
{
	if (str)
		write(fd, str, strlen(str));
}

void	px_error(const char *context, const char *arg, int code)
{
	if (code == EXIT_CMD)
	{
		px_putstr(2, "pipex: ");
		px_putstr(2, arg);
		px_putstr(2, ": command not found\n");
	}
	else if (code == EXIT_NOEXEC)
	{
		px_putstr(2, "pipex: ");
		px_putstr(2, arg);
		px_putstr(2, ": Permission denied\n");
	}
	else
	{
		px_putstr(2, "pipex: ");
		px_putstr(2, context);
		if (arg)
		{
			px_putstr(2, ": ");
			px_putstr(2, arg);
		}
		px_putstr(2, ": ");
		px_putstr(2, strerror(errno));
		px_putstr(2, "\n");
	}
}

void	px_usage(void)
{
	px_putstr(2, "Usage: ./pipex infile cmd1 cmd2 outfile\n");
	px_putstr(2, "       ./pipex file1 cmd1 cmd2 ... cmdn file2\n");
	px_putstr(2, "       ./pipex here_doc LIMITER cmd1 ... cmdn file\n");
}
