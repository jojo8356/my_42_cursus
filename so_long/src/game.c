/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   game.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/08 22:24:24 by jojo8356          #+#    #+#             */
/*   Updated: 2026/11/08 22:24:24 by jojo8356         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "so_long.h"
#include "mlx.h"

/*
** Game state, movement and the event hooks.
**
** The player's position lives in the game structure, not in the map grid: the
** grid stays exactly as loaded, so a collectible tile is found again by a plain
** character lookup even after the player has walked over it. Writing '0' into
** the grid would work for collectibles but destroys the exit tile the first
** time the player steps on it, which is the classic so_long bug.
*/

int	game_init(t_game *g, const char *path)
{
	int	res;

	ft_memset(g, 0, sizeof(*g));
	res = map_load(path, &g->map);
	if (res <= 0)
		return (res);
	g->player_x = g->map.start_x;
	g->player_y = g->map.start_y;
	return (1);
}

/*
** Picking a collectible up. Done by comparing coordinates rather than by
** rewriting the grid, so the tile below the player is still a 'C' for anyone
** who asks -- and the count of remaining ones is authoritative.
*/

static void	pick_up(t_game *g, int x, int y)
{
	if (tile_at(&g->map, x, y) == T_COLLECT)
		g->collected++;
	if (tile_at(&g->map, x, y) == T_EXIT
		&& g->collected >= g->map.collectibles)
		g->over = 1;
}

/*
** One move. Refused against a wall, and refused once the game is over so the
** move counter cannot be inflated after winning or after ESC.
*/

void	game_move(t_game *g, int dx, int dy)
{
	int	nx;
	int	ny;

	if (g->over)
		return ;
	nx = g->player_x + dx;
	ny = g->player_y + dy;
	if (tile_at(&g->map, nx, ny) == T_WALL)
		return ;
	if (tile_at(&g->map, nx, ny) == T_EXIT
		&& g->collected < g->map.collectibles)
		return ;
	g->player_x = nx;
	g->player_y = ny;
	g->moves++;
	printf("%d\n", g->moves);
	fflush(stdout);
	pick_up(g, nx, ny);
}
