/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   render.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/08 22:24:07 by jojo8356          #+#    #+#             */
/*   Updated: 2026/11/08 22:24:07 by jojo8356         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "so_long.h"
#include "mlx.h"

/*
** Drawing. One tile, one blit, then the actors on top.
**
** The player's position lives in the game, not in the grid, so drawing is a
** pure function of (map, state): nothing here mutates the level, which is what
** lets a test replay a game and get the same frames every time.
*/

char	tile_at(const t_map *m, int x, int y)
{
	if (x < 0 || y < 0 || x >= m->width || y >= m->height)
		return (T_WALL);
	return (m->grid[y][x]);
}

/*
** One tile, one blit. The exit switches to its lit frame once every
** collectible is gone, which is the visual cue that the level can be left.
*/

static void	draw_tile(t_game *g, int x, int y)
{
	char	c;
	int		idx;

	c = tile_at(&g->map, x, y);
	idx = 0;
	if (c == T_WALL)
		idx = 1;
	else if (c == T_COLLECT)
		idx = 2;
	else if (c == T_EXIT)
		idx = (g->collected >= g->map.collectibles) ? 4 : 3;
	mlx_put_image_to_window(g->mlx, g->win, g->img[idx].ptr,
		x * TILE, y * TILE);
}

/*
** The player and the enemies are drawn after the tiles so they always sit on
** top of the floor and the collectibles.
*/

static void	draw_actors(t_game *g)
{
	int	i;

	if (g->frame % 2 == 0)
		mlx_put_image_to_window(g->mlx, g->win, g->img[5].ptr,
			g->player_x * TILE, g->player_y * TILE);
	else
		mlx_put_image_to_window(g->mlx, g->win, g->img[6].ptr,
			g->player_x * TILE, g->player_y * TILE);
	if (!g->bonus || !g->enemies)
		return ;
	i = 0;
	while (i < g->enemy_count)
	{
		if (g->frame % 2 == 0)
			mlx_put_image_to_window(g->mlx, g->win, g->img[7].ptr,
				g->enemies[i].x * TILE, g->enemies[i].y * TILE);
		else
			mlx_put_image_to_window(g->mlx, g->win, g->img[8].ptr,
				g->enemies[i].x * TILE, g->enemies[i].y * TILE);
		i++;
	}
}

void	render_draw(t_game *g)
{
	int	x;
	int	y;

	y = 0;
	while (y < g->map.height)
	{
		x = 0;
		while (x < g->map.width)
		{
			draw_tile(g, x, y);
			x++;
		}
		y++;
	}
	draw_actors(g);
}

void	render_close(t_game *g)
{
	int	i;

	i = 0;
	while (i < 9)
	{
		if (g->img[i].ptr)
			mlx_destroy_image(g->mlx, g->img[i].ptr);
		g->img[i].ptr = NULL;
		i++;
	}
}
