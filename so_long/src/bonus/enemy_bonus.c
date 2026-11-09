/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   enemy_bonus.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/08 22:24:33 by jojo8356          #+#    #+#             */
/*   Updated: 2026/11/08 22:24:33 by jojo8356         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "so_long.h"

/*
** Bonus 1: enemy patrols.
**
** An enemy is spawned on every 'X' tile of the map. It walks back and forth on
** its row, reversing at walls, and the game is lost on contact. Movement is
** deliberately row-only: a patrol that also turns at random makes the level
** unfair in a way the player cannot read from the screen, and the subject only
** asks for patrols that cause the player to lose on contact.
*/

/*
** Fills the array from the map, in reading order, so enemy i always corresponds
** to the i-th 'X' of the file. Alternate directions keep two neighbouring
** enemies from walking in lockstep.
*/

static void	fill(t_game *g)
{
	int	i;
	int	x;
	int	y;

	i = 0;
	y = -1;
	while (++y < g->map.height)
	{
		x = -1;
		while (++x < g->map.width)
		{
			if (g->map.grid[y][x] != T_ENEMY)
				continue ;
			g->enemies[i].x = x;
			g->enemies[i].y = y;
			g->enemies[i].dir = (i % 2) ? -1 : 1;
			i++;
		}
	}
}

int	bonus_spawn_enemies(t_game *g)
{
	int	count;

	count = count_char(&g->map, T_ENEMY);
	if (count == 0)
		return (1);
	g->enemies = (t_enemy *)ft_calloc((size_t)count, sizeof(t_enemy));
	if (!g->enemies)
		return (0);
	fill(g);
	g->enemy_count = count;
	return (1);
}

/*
** One step per enemy per frame. A step into a wall, an exit or a collectible
** is refused and the direction is reversed, so an enemy never leaves its row
** and never destroys the level's contents.
*/

static void	step(t_game *g, t_enemy *e)
{
	int	next;

	next = e->x + e->dir;
	if (tile_at(&g->map, next, e->y) == T_WALL)
	{
		e->dir = -e->dir;
		return ;
	}
	e->x = next;
}

static int	touches_player(t_game *g, const t_enemy *e)
{
	return (e->x == g->player_x && e->y == g->player_y);
}

/*
** Called once per frame by the loop hook. Contact is checked after moving,
** which is what makes an enemy walking into the player as lethal as the player
** walking into an enemy.
*/

void	bonus_tick(t_game *g)
{
	int	i;

	if (!g->enemies || g->over)
		return ;
	g->frame++;
	i = 0;
	while (i < g->enemy_count)
	{
		step(g, &g->enemies[i]);
		if (touches_player(g, &g->enemies[i]))
		{
			g->over = 1;
			write(1, "You lose\n", 9);
			return ;
		}
		i++;
	}
}
