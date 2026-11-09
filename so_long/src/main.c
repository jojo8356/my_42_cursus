/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
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
** Entry point. The map is fully validated before the minilibX is initialised,
** so a malformed map never flashes a window open and closed: the subject's
** "exit properly" is easier to satisfy when nothing graphical has happened yet.
*/

static int	win_width(t_game *g)
{
	return (g->map.width * TILE);
}

static int	win_height(t_game *g)
{
	return (g->map.height * TILE);
}

/*
** Opens the window, plays, and releases everything. Split from main so that
** both the window creation and the cleanup stay short enough to read: the
** cleanup path is the part of a graphical program that is easiest to get wrong
** and hardest to notice.
*/

static int	run(t_game *g)
{
	g->mlx = mlx_init();
	if (!g->mlx)
		return (map_free(&g->map), 1);
	g->win = mlx_new_window(g->mlx, win_width(g), win_height(g), "so_long");
	if (!g->win)
		return (map_free(&g->map), 1);
	if (!render_init(g, g->bonus))
		return (render_close(g), map_free(&g->map), 1);
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
		err_exit("usage: ./so_long <map.ber>");
		return (1);
	}
	res = game_init(&g, av[1]);
	if (res <= 0)
	{
		/*
		** An invalid map leaves the grid allocated: the rules need the rows
		** to explain what is wrong with them. Freeing it here is what makes
		** "exit properly" true for a rejected map as well as a played one.
		*/
		map_free(&g.map);
		if (res < 0)
			err_exit("cannot read the map file");
		return (1);
	}
	return (run(&g));
}
