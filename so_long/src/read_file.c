/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   read_file.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/08 22:23:18 by jojo8356          #+#    #+#             */
/*   Updated: 2026/11/08 22:23:18 by jojo8356         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "so_long.h"

/*
** Reads a whole file. The map is small by nature (a few dozen rows), so the
** size is discovered by reading, not by stat: one pass, no assumption about
** the file being seekable (a map can be piped in).
*/

/*
** Appends one chunk to the buffer, growing it by exactly what was read. Returns
** the new buffer, or NULL on allocation failure -- in which case the old one is
** already freed, so the caller only has to return.
*/

static char	*append(char *buf, int len, const char *chunk, int n)
{
	char	*out;

	out = ft_calloc(len + n + 1, 1);
	if (!out)
		return (free(buf), NULL);
	ft_memcpy(out, buf, len);
	ft_memcpy(out + len, chunk, n);
	free(buf);
	return (out);
}

char	*read_file(const char *path)
{
	int		fd;
	char	*buf;
	char	chunk[4096];
	int		n;
	int		len;

	fd = open(path, O_RDONLY);
	if (fd < 0)
		return (NULL);
	buf = ft_calloc(1, 1);
	if (!buf)
		return (close(fd), NULL);
	len = 0;
	n = (int)read(fd, chunk, sizeof(chunk));
	while (n > 0 && buf)
	{
		buf = append(buf, len, chunk, n);
		len += n;
		n = (int)read(fd, chunk, sizeof(chunk));
	}
	close(fd);
	if (!buf || n < 0)
		return (free(buf), NULL);
	return (buf);
}

/*
** Number of rows in the file. A single trailing newline terminates the last row
** rather than starting a new one; every other newline, empty or not, starts a
** row. That is what makes an empty line in the middle of a map detectable: it
** becomes a zero-length row and the rectangularity check rejects it.
*/

int	count_lines(const char *content)
{
	int	i;
	int	rows;

	rows = 0;
	i = 0;
	while (content[i])
	{
		rows++;
		while (content[i] && content[i] != '\n')
			i++;
		if (content[i] == '\n')
			i++;
	}
	return (rows);
}

static char	*copy_span(const char *s, int n)
{
	char	*out;

	out = (char *)malloc((size_t)n + 1);
	if (!out)
		return (NULL);
	ft_memcpy(out, s, (size_t)n);
	out[n] = '\0';
	return (out);
}

char	**split_lines(const char *content, int height)
{
	char	**rows;
	int		i;
	int		start;
	int		r;

	rows = (char **)ft_calloc((size_t)height + 1, sizeof(char *));
	if (!rows)
		return (NULL);
	i = 0;
	r = 0;
	while (r < height)
	{
		start = i;
		while (content[i] && content[i] != '\n')
			i++;
		rows[r] = copy_span(content + start, i - start);
		if (!rows[r])
			return (map_free_rows(rows), NULL);
		r++;
		if (content[i] == '\n')
			i++;
	}
	return (rows);
}
