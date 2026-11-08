/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   game_hooks.c                                       :+:      :+:    :+:   */
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
** Hooks and the main loop. The two ways out the subject requires are both
** wired here: the red cross of the window frame (event 17) and the ESC key.
** Both set the same flag, so the cleanup path is single.
*/

static int	on_close(t_game *g)
{
	g->over = 1;
	mlx_loop_end(g->mlx);
	return (0);
}

static int	on_key(int key, t_game *g)
{
	if (key == KEY_ESC)
	{
		g->over = 1;
		mlx_loop_end(g->mlx);
		return (0);
	}
	if (key == KEY_W)
		game_move(g, 0, -1);
	else if (key == KEY_S)
		game_move(g, 0, 1);
	else if (key == KEY_A)
		game_move(g, -1, 0);
	else if (key == KEY_D)
		game_move(g, 1, 0);
	return (0);
}

/*
** The loop hook does two things: advance the animation frame, and stop the game
** when the player has won. Checking the win condition here (rather than only in
** game_move) also covers the bonus, where an enemy can end the game without any
** key press.
*/

static int	on_loop(t_game *g)
{
	render_draw(g);
	if (g->bonus)
		bonus_frame(g);
	if (g->over)
		mlx_loop_end(g->mlx);
	return (0);
}

int	game_hooks(t_game *g)
{
	mlx_hook(g->win, 17, 0, (int (*)())&on_close, g);
	mlx_key_hook(g->win, (int (*)())&on_key, g);
	mlx_loop_hook(g->mlx, (int (*)())&on_loop, g);
	return (1);
}

int	game_loop(t_game *g)
{
	game_hooks(g);
	mlx_loop(g->mlx);
	return (0);
}
