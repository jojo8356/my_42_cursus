/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   get_next_line_utils.c                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/08 19:30:00 by jojo8356          #+#    #+#             */
/*   Updated: 2026/11/08 19:30:00 by jojo8356         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "get_next_line.h"

void	gnl_clear(t_stash *s)
{
	if (s->buf)
		free(s->buf);
	s->buf = NULL;
	s->len = 0;
	s->cap = 0;
	s->scan = 0;
}

void	gnl_memcpy(char *dst, const char *src, size_t n)
{
	size_t	i;

	i = 0;
	while (i < n)
	{
		dst[i] = src[i];
		i++;
	}
}

int	gnl_find_nl(t_stash *s, size_t *nl_idx)
{
	size_t	i;

	if (!s->buf)
		return (0);
	i = s->scan;
	while (i < s->len)
	{
		if (s->buf[i] == '\n')
		{
			*nl_idx = i;
			s->scan = i;
			return (1);
		}
		i++;
	}
	s->scan = s->len;
	return (0);
}

int	gnl_reserve(t_stash *s, size_t add)
{
	char	*nbuf;
	size_t	need;
	size_t	ncap;

	if (add > (size_t)-1 - s->len - 1)
		return (0);
	need = s->len + add + 1;
	if (need <= s->cap)
		return (1);
	ncap = 64;
	if (s->cap > 0 && s->cap <= ((size_t)-1) / 2)
		ncap = s->cap * 2;
	if (ncap < need)
		ncap = need;
	nbuf = malloc(ncap);
	if (!nbuf)
		return (0);
	if (s->buf && s->len > 0)
		gnl_memcpy(nbuf, s->buf, s->len);
	if (s->buf)
		free(s->buf);
	s->buf = nbuf;
	s->cap = ncap;
	return (1);
}
