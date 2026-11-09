/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   map_valid.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/08 22:23:31 by jojo8356          #+#    #+#             */
/*   Updated: 2026/11/08 22:23:31 by jojo8356         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "so_long.h"

/*
** Every rule of chapter IV, checked in the order that gives the clearest
** message. The subject allows any wording for the detail line, as long as
** "Error" comes first, so each rule gets its own sentence: a reviewer reading
** the terminal should never have to guess which rule fired.
*/

/*
** The error goes to stdout, not stderr.
**
** Chapter IV only requires "Error\n followed by an explicit message"; it does
** not name a stream. The community tester -- kept verbatim in tests/community/
** -- decides whether a map was rejected with `./so_long $MAP | grep Error`, so
** writing to stderr would make all thirteen of its checks fail on a program
** that is perfectly correct. Printing on stdout is the choice that satisfies
** both the subject and the tool.
*/

int	bad_map(const char *path, const char *msg)
{
	write(1, "Error\n", 6);
	if (path)
	{
		write(1, path, (size_t)ps_len(path));
		write(1, ": ", 2);
	}
	write(1, msg, (size_t)ps_len(msg));
	write(1, "\n", 1);
	return (0);
}

/*
** The five symbols of chapter IV, and only those five. 'X' -- the enemy of the
** bonus -- is deliberately absent: the mandatory program must reject it, and
** the bonus build relaxes this one predicate through a compile-time hook
** (see src/bonus/charset_bonus.c).
*/

int	charset_ok(char c)
{
	return (c == T_FLOOR || c == T_WALL || c == T_COLLECT
		|| c == T_EXIT || c == T_PLAYER || bonus_charset(c));
}

static int	check_charset(const t_map *m)
{
	int	x;
	int	y;

	y = 0;
	while (y < m->height)
	{
		x = 0;
		while (x < m->width)
		{
			if (!charset_ok(m->grid[y][x]))
				return (0);
			x++;
		}
		y++;
	}
	return (1);
}

static int	check_walls(const t_map *m)
{
	int	i;

	i = 0;
	while (i < m->width)
	{
		if (m->grid[0][i] != T_WALL || m->grid[m->height - 1][i] != T_WALL)
			return (0);
		i++;
	}
	i = 0;
	while (i < m->height)
	{
		if (m->grid[i][0] != T_WALL || m->grid[i][m->width - 1] != T_WALL)
			return (0);
		i++;
	}
	return (1);
}

/*
** Exactly one player, at least one exit and at least one collectible. Counts
** are gathered in one pass so a map with two players and no exit reports the
** player rule, which is the one a human would notice first.
*/

/*
** The sprite rules live in map_sprites.c: this file is already at the Norm's
** function budget with the geometric rules.
*/

int	map_valid(t_map *m, const char *path)
{
	int	y;

	if (!has_ber_extension(path))
		return (bad_map(path, "map file must end in .ber"));
	if (m->height < 3 || m->width < 3)
		return (bad_map(path, "map is too small"));
	y = 0;
	while (y < m->height)
	{
		if ((int)ps_len(m->grid[y]) != m->width)
			return (bad_map(path, "map is not rectangular"));
		y++;
	}
	if (!check_charset(m))
		return (bad_map(path, "map contains a forbidden character"));
	if (!check_walls(m))
		return (bad_map(path, "map is not closed by walls"));
	return (check_sprites(m, path));
}
