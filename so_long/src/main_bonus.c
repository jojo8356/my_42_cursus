/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main_bonus.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/08 22:24:38 by jojo8356          #+#    #+#             */
/*   Updated: 2026/11/08 22:24:38 by jojo8356         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "so_long.h"
#include "mlx.h"

/*
** Bonus entry point. It differs from the mandatory one in three ways, and only
** three: the bonus flag is set, the enemy patrols are spawned, and the HUD is
** drawn on top of the frame. Everything else -- parsing, validation, hooks,
** cleanup -- is the same code, because a bonus that reimplements the game is a
** bonus with its own bugs.
*/

/*
** Opens the window, plays, and releases everything -- including the enemy
** array, which the mandatory build never allocates. Split from main to keep
** both the setup and the cleanup readable.
*/

static int	run(t_game *g)
{
	g->mlx = mlx_init();
	if (!g->mlx)
		return (map_free(&g->map), free(g->enemies), 1);
	g->win = mlx_new_window(g->mlx, g->map.width * TILE,
			g->map.height * TILE, "so_long_bonus");
	if (!g->win)
		return (map_free(&g->map), free(g->enemies), 1);
	if (!render_init(g, 1))
		return (render_close(g), map_free(&g->map), free(g->enemies), 1);
	game_loop(g);
	render_close(g);
	mlx_destroy_window(g->mlx, g->win);
	mlx_destroy_display(g->mlx);
	map_free(&g->map);
	free(g->enemies);
	return (0);
}

int	main(int ac, char **av)
{
	t_game	g;
	int		res;

	if (ac != 2)
	{
		err_exit("usage: ./so_long_bonus <map.ber>");
		return (1);
	}
	res = game_init(&g, av[1]);
	if (res <= 0)
	{
		map_free(&g.map);
		if (res < 0)
			err_exit("cannot read the map file");
		return (1);
	}
	g.bonus = 1;
	if (!bonus_spawn_enemies(&g))
		return (map_free(&g.map), 1);
	return (run(&g));
}
