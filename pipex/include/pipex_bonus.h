/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   pipex_bonus.h                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/08 12:00:00 by jojo8356          #+#    #+#             */
/*   Updated: 2026/11/08 12:00:00 by jojo8356         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PIPEX_BONUS_H
# define PIPEX_BONUS_H

# include "pipex.h"

/*
** Everything the mandatory part needs, plus the two bonus features:
**   - a chain of N pipes (N + 1 commands) instead of exactly two,
**   - the here_doc mode: px_read_until() feeds the first command from
**     stdin until the limiter line is found, exactly like `<<` in a shell.
*/

typedef struct s_px
{
	char	**argv;
	char	**envp;
	int		ncmd;
	int		in_fd;
	char	*out_file;
	int		append;
	int		prev;
}	t_px;

/* pipex_here_doc_bonus.c */
int		px_read_until(const char *limiter, int out_fd);
int		px_here_doc_file(const char *limiter);

/* pipex_run_bonus.c */
int		px_run_bonus(int ac, char **av, char **envp);

/* pipex_child_bonus.c */
void	px_close_ends(t_px *px, int pfd[2], int last);
int		px_child_out(t_px *px);
void	px_child(t_px *px, int pfd[2], int idx);

#endif
