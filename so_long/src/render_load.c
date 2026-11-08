/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   render_load.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/08 22:29:13 by jojo8356          #+#    #+#             */
/*   Updated: 2026/11/08 22:29:13 by jojo8356         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "so_long.h"
#include "mlx.h"

/*
** Loading the nine images. They are loaded once, at start-up, and blitted per
** tile: loading an XPM inside the draw loop would be correct but slow enough to
** be visible when a key is held, and the subject asks for smooth window
** management.
*/

static const char	*g_paths[9] = {
	"assets/floor.xpm", "assets/wall.xpm", "assets/collect.xpm",
	"assets/exit.xpm", "assets/exit_open.xpm", "assets/player1.xpm",
	"assets/player2.xpm", "assets/enemy1.xpm", "assets/enemy2.xpm"
};

int	render_init(t_game *g, int bonus)
{
	int	i;

	(void)bonus;
	i = 0;
	while (i < 9)
	{
		g->img[i].ptr = mlx_xpm_file_to_image(g->mlx, (char *)g_paths[i],
				&g->img[i].width, &g->img[i].height);
		if (!g->img[i].ptr)
			return (0);
		i++;
	}
	return (1);
}
