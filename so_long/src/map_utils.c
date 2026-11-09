/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   map_utils.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/08 22:23:31 by jojo8356          #+#    #+#             */
/*   Updated: 2026/11/08 22:23:31 by jojo8356         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "so_long.h"

/*
** Small helpers the validation and the game share. They are in their own file
** to keep every translation unit inside the Norm's five-function budget.
*/

int	ps_len(const char *s)
{
	int	n;

	n = 0;
	while (s && s[n])
		n++;
	return (n);
}

int	has_ber_extension(const char *path)
{
	int	n;

	n = ps_len(path);
	if (n < 5)
		return (0);
	return (path[n - 4] == '.' && path[n - 3] == 'b'
		&& path[n - 2] == 'e' && path[n - 1] == 'r');
}

int	count_char(const t_map *m, char c)
{
	int	x;
	int	y;
	int	n;

	n = 0;
	y = 0;
	while (y < m->height)
	{
		x = 0;
		while (x < m->width)
		{
			if (m->grid[y][x] == c)
				n++;
			x++;
		}
		y++;
	}
	return (n);
}

void	find_player(t_map *m)
{
	int	x;
	int	y;

	y = 0;
	while (y < m->height)
	{
		x = 0;
		while (x < m->width)
		{
			if (m->grid[y][x] == T_PLAYER)
			{
				m->start_x = x;
				m->start_y = y;
				return ;
			}
			x++;
		}
		y++;
	}
}

void	map_free_rows(char **rows)
{
	int	i;

	if (!rows)
		return ;
	i = 0;
	while (rows[i])
	{
		free(rows[i]);
		i++;
	}
	free(rows);
}
