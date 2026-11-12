/* ************************************************************************** */
/*                                                                          */
/*                                                        :::      ::::::::   */
/*   readline_mock.h                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/12 15:45:00 by jojo8356          #+#    #+#             */
/*   Updated: 2026/11/12 15:45:00 by jojo8356         ###   ########.fr       */
/*                                                                          */
/* ************************************************************************** */

# ifndef READLINE_MOCK_H
# define READLINE_MOCK_H

# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <unistd.h>

/* Mock of readline() using getline() */
char *readline(const char *prompt);

/* Mock of add_history() */
void add_history(char *line);

/* Mock of rl_clear_history() */
void rl_clear_history(void);

/* Mock of rl_on_new_line() */
void rl_on_new_line(void);

/* Mock of rl_replace_line() */
void rl_replace_line(char *line, int len);

/* Mock of rl_redisplay() */
void rl_redisplay(void);

/* History storage for mock */
# ifndef MOCK_RL_MAX_HISTORY
# define MOCK_RL_MAX_HISTORY 100
# endif

# endif
