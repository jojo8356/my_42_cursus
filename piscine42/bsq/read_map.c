/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   read_map.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/08 19:00:00 by jojo8356          #+#    #+#             */
/*   Updated: 2026/11/08 19:00:00 by jojo8356         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

/* Lecture complete d'un flux puis analyse/validation de la carte. */
#include <stdlib.h>
#include <unistd.h>
#include "bsq.h"

static char	*grow_buf(char *buf, int total, int *cap)
{
	char	*tmp;
	int		i;

	tmp = malloc((*cap) * 2);
	if (!tmp)
	{
		free(buf);
		return (0);
	}
	*cap *= 2;
	i = 0;
	while (i < total)
	{
		tmp[i] = buf[i];
		i++;
	}
	free(buf);
	return (tmp);
}

char	*read_fd(int fd, int *len)
{
	char	*buf;
	int		total;
	int		cap;
	int		r;

	cap = 1 << 20;
	buf = malloc(cap);
	if (!buf)
		return (0);
	total = 0;
	r = read(fd, buf, cap - 1);
	while (r > 0)
	{
		total += r;
		if (total >= cap - 1)
			buf = grow_buf(buf, total, &cap);
		if (!buf)
			return (0);
		r = read(fd, buf + total, cap - 1 - total);
	}
	buf[total] = 0;
	*len = total;
	return (buf);
}

static int	parse_header(char *buf, int hlen, t_map *m)
{
	int	i;
	int	n;

	if (hlen < 4)
		return (0);
	n = 0;
	i = -1;
	while (++i < hlen - 3)
	{
		if (buf[i] < '0' || buf[i] > '9')
			return (0);
		n = n * 10 + (buf[i] - '0');
	}
	m->empty = buf[hlen - 3];
	m->obst = buf[hlen - 2];
	m->full = buf[hlen - 1];
	if (n <= 0 || m->empty == m->obst || m->empty == m->full
		|| m->obst == m->full || m->empty < 32 || m->obst < 32
		|| m->full < 32 || m->empty > 126 || m->obst > 126 || m->full > 126)
		return (0);
	m->rows = n;
	return (1);
}

static int	copy_row(char *buf, int start, int r, t_map *m)
{
	int	c;

	m->grid[r] = malloc(m->cols + 1);
	if (!m->grid[r])
		return (0);
	c = 0;
	while (c < m->cols)
	{
		if (buf[start + c] != m->empty && buf[start + c] != m->obst)
			return (0);
		m->grid[r][c] = buf[start + c];
		c++;
	}
	m->grid[r][m->cols] = 0;
	return (1);
}

int	load_map(char *buf, int len, t_map *m)
{
	int	p[3];

	p[0] = 0;
	while (p[0] < len && buf[p[0]] != '\n')
		p[0]++;
	if (p[0] == len || !parse_header(buf, p[0]++, m))
		return (0);
	m->grid = malloc(sizeof(char *) * m->rows);
	if (!m->grid)
		return (0);
	p[1] = -1;
	while (++p[1] < m->rows)
		m->grid[p[1]] = 0;
	p[1] = -1;
	while (++p[1] < m->rows && (p[2] = p[0]) >= 0)
	{
		while (p[0] < len && buf[p[0]] != '\n')
			p[0]++;
		if (p[1] == 0)
			m->cols = p[0] - p[2];
		if (p[0] - p[2] < 1 || p[0] - p[2] != m->cols || p[0]++ >= len
			|| !copy_row(buf, p[2], p[1], m))
			return (0);
	}
	return (p[0] == len);
}
