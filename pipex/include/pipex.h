/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   pipex.h                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/08 12:00:00 by jojo8356          #+#    #+#             */
/*   Updated: 2026/11/08 12:00:00 by jojo8356         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PIPEX_H
# define PIPEX_H

# include <unistd.h>
# include <stdlib.h>
# include <stdio.h>
# include <string.h>
# include <errno.h>
# include <signal.h>
# include <fcntl.h>
# include <sys/types.h>
# include <sys/wait.h>
# include <sys/stat.h>

/* Exit codes used across the project. */
# define EXIT_USAGE	1
# define EXIT_CMD	127
# define EXIT_NOEXEC	126

/* The size of the read buffer used by the here_doc reader. */
# ifndef PX_BUFSIZE
#  define PX_BUFSIZE 4096
# endif

/* pipex_error.c */
void	px_error(const char *context, const char *arg, int code);
void	px_usage(void);

/* pipex_split.c */
char	**px_split(const char *line, char sep);
void	px_free_tab(char **tab);
size_t	px_tablen(char **tab);

/* pipex_path.c */
char	*px_path_value(char **envp);
char	*px_resolve(const char *cmd, char **envp);
int		px_is_executable(const char *path);

/* pipex_exec.c */
void	px_exec(char *const argv[], char **envp);

/* pipex_here_doc.c */
int		px_here_doc(const char *limiter);

/* pipex_run.c */
int		px_pipeline(int ac, char **av, char **envp);

#endif
