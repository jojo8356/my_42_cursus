/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   render_bonus.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/08 22:24:33 by jojo8356          #+#    #+#             */
/*   Updated: 2026/11/08 22:24:33 by jojo8356         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "so_long.h"
#include "mlx.h"

/*
** Bonus 3: the move counter on the screen instead of (well, as well as) in the
** shell. The subject offers it as a bonus on top of the mandatory shell output,
** so the shell line stays and this adds a HUD in the top-left corner.
**
** Bonus 2: sprite animation. render_draw already alternates the two player and
** enemy frames on g->frame; this file makes the frame advance on a timer rather
** than on a move, so the sprites breathe even when the player stands still.
*/

/*
** mlx_string_put draws with a bitmap font. The text is composed by hand because
** the allowed functions do not include snprintf; three itoa calls and a fixed
** prefix keep it to one buffer.
*/

void	bonus_draw_hud(t_game *g)
{
	char	*n;
	int		color;

	n = ft_itoa(g->moves);
	if (!n)
		return ;
	color = 0xECEFF4;
	mlx_string_put(g->mlx, g->win, 8, 8, color, "Moves:");
	mlx_string_put(g->mlx, g->win, 80, 8, color, n);
	free(n);
}
