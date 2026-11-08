/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dict.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/08 19:00:00 by jojo8356          #+#    #+#             */
/*   Updated: 2026/11/08 19:00:00 by jojo8356         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include "rush02.h"

static char	*read_file(char *path, int *len)
{
	int		fd;
	int		total;
	int		r;
	char	*buf;

	fd = open(path, O_RDONLY);
	if (fd < 0)
		return (0);
	buf = malloc(1 << 20);
	if (!buf)
		return (close(fd), (char *)0);
	total = 0;
	r = read(fd, buf, (1 << 20) - 1);
	while (r > 0 && total + r < (1 << 20) - 1)
	{
		total += r;
		r = read(fd, buf + total, (1 << 20) - 1 - total);
	}
	if (r > 0)
		total += r;
	close(fd);
	buf[total] = '\0';
	*len = total;
	return (buf);
}

static int	store_entry(t_dict *d, char *k, int klen, char *v)
{
	char	*nk;
	char	*nv;

	if (klen == 4 && ft_strcmp(k, "lang") == 0)
	{
		d->lang = (ft_strcmp(v, "fr") == 0);
		return (1);
	}
	if (d->count >= MAX_ENTRIES)
		return (0);
	nk = norm_digits(k, klen);
	if (!nk)
		return (0);
	nv = ft_strdup_n(v, ft_strlen(v));
	if (!nv)
	{
		free(nk);
		return (0);
	}
	d->key[d->count] = nk;
	d->val[d->count] = nv;
	d->count++;
	return (1);
}

static int	parse_line(char *line, t_dict *d)
{
	int	colon;
	int	klen;
	int	vstart;
	int	vend;

	colon = 0;
	while (line[colon] && line[colon] != ':')
		colon++;
	if (!line[colon])
		return (0);
	klen = colon;
	while (klen > 0 && line[klen - 1] == ' ')
		klen--;
	vstart = colon + 1;
	while (line[vstart] == ' ')
		vstart++;
	vend = ft_strlen(line);
	while (vend > vstart && (line[vend - 1] == ' ' || line[vend - 1] == '\r'))
		vend--;
	line[klen] = '\0';
	line[vend] = '\0';
	return (store_entry(d, line, klen, line + vstart));
}

void	dict_free(t_dict *d)
{
	int	i;

	i = 0;
	while (i < d->count)
	{
		free(d->key[i]);
		free(d->val[i]);
		i++;
	}
	d->count = 0;
}

int	dict_load(char *path, t_dict *d)
{
	char	*buf;
	int		len;
	int		start;
	int		i;

	d->count = 0;
	d->lang = 0;
	buf = read_file(path, &len);
	if (!buf)
		return (0);
	start = 0;
	i = -1;
	while (++i <= len)
	{
		if (buf[i] == '\n' || buf[i] == '\0')
		{
			buf[i] = '\0';
			if (ft_strlen(buf + start) > 0 && !parse_line(buf + start, d))
				return (dict_free(d), free(buf), 0);
			start = i + 1;
		}
	}
	free(buf);
	return (1);
}
