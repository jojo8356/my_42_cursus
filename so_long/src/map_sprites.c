/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   map_sprites.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/08 22:29:13 by jojo8356          #+#    #+#             */
/*   Updated: 2026/11/08 22:29:13 by jojo8356         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "so_long.h"

/*
** The sprite rules of chapter IV: at least one collectible, exactly one exit,
** exactly one player. They are separate from the geometry because they are the
** rules a map most often gets wrong, and a separate file keeps both small
** enough for the Norm.
*/

int	check_sprites(t_map *m, const char *path)
{
	m->collectibles = count_char(m, T_COLLECT);
	if (m->collectibles < 1)
		return (bad_map(path, "map has no collectible"));
	if (count_char(m, T_EXIT) != 1)
		return (bad_map(path, "map needs exactly one exit"));
	if (count_char(m, T_PLAYER) != 1)
		return (bad_map(path, "map needs exactly one player"));
	find_player(m);
	return (1);
}
