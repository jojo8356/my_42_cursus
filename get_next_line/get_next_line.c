/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   get_next_line.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/08 19:30:00 by jojo8356          #+#    #+#             */
/*   Updated: 2026/11/08 19:30:00 by jojo8356         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "get_next_line.h"

static int	gnl_fill(int fd, t_stash *s, size_t bs, size_t *llen)
{
	ssize_t	r;

	while (!gnl_find_nl(s, llen))
	{
		if (!gnl_reserve(s, bs))
			return (0);
		r = read(fd, s->buf + s->len, bs);
		if (r < 0)
			return (0);
		if (r == 0)
		{
			*llen = s->len;
			return (1);
		}
		s->len += (size_t)r;
		s->buf[s->len] = '\0';
	}
	*llen += 1;
	return (1);
}

static char	*gnl_extract(t_stash *s, size_t llen)
{
	char	*line;
	size_t	rem;

	line = malloc(llen + 1);
	if (!line)
	{
		gnl_clear(s);
		return (NULL);
	}
	gnl_memcpy(line, s->buf, llen);
	line[llen] = '\0';
	rem = s->len - llen;
	if (rem == 0)
		gnl_clear(s);
	else
	{
		gnl_memcpy(s->buf, s->buf + llen, rem);
		s->len = rem;
		s->buf[rem] = '\0';
		s->scan = 0;
	}
	return (line);
}

char	*get_next_line(int fd)
{
	static t_stash	stash;
	ssize_t			bs;
	size_t			llen;

	bs = BUFFER_SIZE;
	if (fd < 0 || bs <= 0)
		return (NULL);
	llen = 0;
	if (!gnl_fill(fd, &stash, (size_t)bs, &llen) || stash.len == 0)
	{
		gnl_clear(&stash);
		return (NULL);
	}
	return (gnl_extract(&stash, llen));
}
