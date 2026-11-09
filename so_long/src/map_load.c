/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   map_load.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/08 22:23:39 by jojo8356          #+#    #+#             */
/*   Updated: 2026/11/08 22:23:39 by jojo8356         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "so_long.h"

/*
** Loads a .ber file and validates it. Returns 1 on success, 0 when the map was
** rejected (after having printed "Error" and the reason), -1 on an allocation
** or read failure, which the caller reports separately so the two cases stay
** distinguishable in the terminal.
*/

static void	set_width(t_map *m)
{
	int	y;
	int	w;

	m->width = 0;
	y = 0;
	while (y < m->height)
	{
		w = ps_len(m->grid[y]);
		if (w > m->width)
			m->width = w;
		y++;
	}
}

int	map_load(const char *path, t_map *m)
{
	char	*content;

	m->grid = NULL;
	m->height = 0;
	m->width = 0;
	content = read_file(path);
	if (!content)
		return (-1);
	m->height = count_lines(content);
	if (m->height == 0)
		return (free(content), bad_map(path, "map file is empty"));
	m->grid = split_lines(content, m->height);
	free(content);
	if (!m->grid)
		return (-1);
	set_width(m);
	return (map_valid(m, path));
}

void	map_free(t_map *m)
{
	if (!m)
		return ;
	map_free_rows(m->grid);
	m->grid = NULL;
	m->height = 0;
	m->width = 0;
}

/*
** The only way the program reports a fatal, non-map error: the map rules print
** their own reason, everything else (missing file, out of memory) lands here.
*/

void	err_exit(const char *msg)
{
	write(1, "Error\n", 6);
	if (msg)
	{
		write(1, msg, (size_t)ps_len(msg));
		write(1, "\n", 1);
	}
}
