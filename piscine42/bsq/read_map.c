/* Lecture complete d'un flux puis analyse/validation de la carte. */
#include <stdlib.h>
#include <unistd.h>
#include "bsq.h"

char	*read_fd(int fd, int *len)
{
	char	*buf;
	char	*tmp;
	int		total;
	int		cap;
	int		r;

	cap = 1 << 20;
	buf = malloc(cap);
	total = 0;
	if (!buf)
		return (0);
	r = read(fd, buf, cap - 1);
	while (r > 0)
	{
		total += r;
		if (total >= cap - 1)
		{
			tmp = malloc(cap * 2);
			if (!tmp)
			{
				free(buf);
				return (0);
			}
			cap *= 2;
			r = -1;
			while (++r < total)
				tmp[r] = buf[r];
			free(buf);
			buf = tmp;
		}
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
	i = 0;
	while (i < hlen - 3)
	{
		if (buf[i] < '0' || buf[i] > '9')
			return (0);
		n = n * 10 + (buf[i] - '0');
		i++;
	}
	m->empty = buf[hlen - 3];
	m->obst = buf[hlen - 2];
	m->full = buf[hlen - 1];
	if (n <= 0 || m->empty == m->obst || m->empty == m->full
		|| m->obst == m->full)
		return (0);
	if (m->empty < 32 || m->obst < 32 || m->full < 32
		|| m->empty > 126 || m->obst > 126 || m->full > 126)
		return (0);
	m->rows = n;
	return (1);
}

static int	fill_grid(char *buf, int pos, int len, t_map *m)
{
	int	r;
	int	c;
	int	start;

	m->grid = malloc(sizeof(char *) * m->rows);
	if (!m->grid)
		return (0);
	r = -1;
	while (++r < m->rows)
		m->grid[r] = 0;
	r = -1;
	while (++r < m->rows)
	{
		start = pos;
		while (pos < len && buf[pos] != '\n')
			pos++;
		if (pos - start < 1)
			return (0);
		if (r == 0)
			m->cols = pos - start;
		if (pos - start != m->cols)
			return (0);
		m->grid[r] = malloc(m->cols + 1);
		if (!m->grid[r])
			return (0);
		c = -1;
		while (++c < m->cols)
		{
			if (buf[start + c] != m->empty && buf[start + c] != m->obst)
				return (0);
			m->grid[r][c] = buf[start + c];
		}
		m->grid[r][m->cols] = 0;
		if (pos < len && buf[pos] == '\n')
			pos++;
	}
	return (pos == len);
}

int	load_map(char *buf, int len, t_map *m)
{
	int	hlen;

	hlen = 0;
	while (hlen < len && buf[hlen] != '\n')
		hlen++;
	if (hlen == len)
		return (0);
	if (!parse_header(buf, hlen, m))
		return (0);
	if (!fill_grid(buf, hlen + 1, len, m))
		return (0);
	return (1);
}

void	free_map(t_map *m)
{
	int	r;

	if (!m->grid)
		return ;
	r = 0;
	while (r < m->rows)
	{
		if (m->grid[r])
			free(m->grid[r]);
		r++;
	}
	free(m->grid);
	m->grid = 0;
}
