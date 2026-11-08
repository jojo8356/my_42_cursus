/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   checker_read.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/08 21:43:43 by jojo8356          #+#    #+#             */
/*   Updated: 2026/11/08 21:43:43 by jojo8356         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "checker.h"

/*
** Reads stdin one line at a time.
**
** Three states have to stay distinct: a line arrived, input ended cleanly, and
** input ended in the middle of a line. The last one is an error because the
** subject requires every instruction to be followed by a newline, and a checker
** that silently accepted "sa" without "\n" would pass a broken push_swap.
*/

static int	fill(t_reader *r)
{
	int	n;

	n = (int)read(0, r->buf, CHUNK);
	r->pos = 0;
	r->len = n;
	return (n);
}

/*
** Returns 1 when a byte was obtained in *c, 0 at a clean end of input, and -1
** when the stream ended in the middle of a line.
*/

static int	take_char(t_reader *r, char *c, int *got)
{
	int	n;

	if (r->pos >= r->len)
	{
		n = fill(r);
		if (n < 0)
			return (-1);
		if (n == 0)
			return (*got ? -1 : 0);
	}
	*c = r->buf[r->pos++];
	return (1);
}

int	ch_next_line(t_reader *r, char *line, int cap)
{
	int		i;
	char	c;
	int		state;

	i = 0;
	while (1)
	{
		state = take_char(r, &c, &i);
		if (state == 0)
			return (0);
		if (state < 0)
			return (-1);
		if (c == '\n')
			return (line[i] = '\0', i);
		if (i + 1 >= cap)
			return (-2);
		line[i++] = c;
	}
}
