/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/08 19:00:00 by jojo8356          #+#    #+#             */
/*   Updated: 2026/11/08 19:00:00 by jojo8356         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

/* BSQ : ./bsq [fichier...]   (sans argument : lit stdin).
   Sortie : carte avec le plus grand carre trace, ou "map error".
   Plusieurs fichiers : sortie separee par une ligne vide. */
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include "bsq.h"

static void	print_map(t_map *m)
{
	int	r;

	r = 0;
	while (r < m->rows)
	{
		ft_putstr(m->grid[r]);
		ft_putstr("\n");
		r++;
	}
}

static void	process_fd(int fd)
{
	char	*buf;
	int		len;
	t_map	m;

	m.grid = 0;
	m.rows = 0;
	buf = read_fd(fd, &len);
	if (!buf)
	{
		ft_putstr("map error\n");
		return ;
	}
	if (!load_map(buf, len, &m))
	{
		free(buf);
		free_map(&m);
		ft_putstr("map error\n");
		return ;
	}
	free(buf);
	solve(&m);
	print_map(&m);
	free_map(&m);
}

int	main(int argc, char **argv)
{
	int	i;
	int	fd;
	int	first;

	first = 1;
	i = 0;
	if (argc == 1)
		process_fd(0);
	while (++i < argc)
	{
		if (!first)
			ft_putstr("\n");
		first = 0;
		fd = open(argv[i], O_RDONLY);
		if (fd < 0)
			ft_putstr("map error\n");
		else
		{
			process_fd(fd);
			close(fd);
		}
	}
	return (0);
}
