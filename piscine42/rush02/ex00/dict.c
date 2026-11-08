/* Lecture et analyse du dictionnaire : [cle]:[valeur] espaces toleres. */
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include "rush02.h"

static char	*read_file(char *path, int *len)
{
	int		fd;
	int		total;
	char	*buf;
	int		r;

	fd = open(path, O_RDONLY);
	if (fd < 0)
		return (0);
	buf = malloc(1 << 20);
	if (!buf)
	{
		close(fd);
		return (0);
	}
	total = 0;
	r = read(fd, buf + total, (1 << 20) - 1 - total);
	while (r > 0 && total < (1 << 20) - 1)
	{
		total += r;
		r = read(fd, buf + total, (1 << 20) - 1 - total);
	}
	close(fd);
	buf[total] = 0;
	*len = total;
	return (buf);
}

static int	parse_line(char *line, t_dict *d)
{
	int	p[4];
	int	ok;

	p[0] = 0;
	while (line[p[0]] && line[p[0]] != ':')
		p[0]++;
	if (!line[p[0]])
		return (0);
	p[1] = p[0];
	while (p[1] > 0 && line[p[1] - 1] == ' ')
		p[1]--;
	p[2] = p[0] + 1;
	while (line[p[2]] == ' ')
		p[2]++;
	p[3] = ft_strlen(line);
	while (p[3] > p[2] && (line[p[3] - 1] == ' ' || line[p[3] - 1] == '\r'))
		p[3]--;
	line[p[1]] = 0;
	line[p[3]] = 0;
	if (!ft_strcmp(line, "lang"))
	{
		d->lang = (ft_strcmp(line + p[2], "fr") == 0);
		return (1);
	}
	d->key[d->count] = ft_atoull(line, &ok);
	if (!ok)
		return (0);
	d->val[d->count] = ft_strdup_n(line + p[2], p[3] - p[2]);
	if (!d->val[d->count])
		return (0);
	d->count++;
	return (1);
}

int	dict_load(char *path, t_dict *d)
{
	char	*buf;
	int		len;
	int		start;
	int		i;

	buf = read_file(path, &len);
	if (!buf)
		return (0);
	d->count = 0;
	d->lang = 0;
	start = 0;
	i = -1;
	while (++i <= len)
	{
		if (buf[i] == '\n' || buf[i] == '\0')
		{
			buf[i] = '\0';
			if (ft_strlen(buf + start) > 0 && !parse_line(buf + start, d))
			{
				free(buf);
				return (0);
			}
			start = i + 1;
		}
	}
	free(buf);
	return (1);
}

char	*dict_find(t_dict *d, unsigned long long k)
{
	int	i;

	i = 0;
	while (i < d->count)
	{
		if (d->key[i] == k)
			return (d->val[i]);
		i++;
	}
	return (0);
}
