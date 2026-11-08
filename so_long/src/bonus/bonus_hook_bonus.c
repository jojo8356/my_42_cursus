/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   bonus_hook_bonus.c                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/08 22:24:51 by jojo8356          #+#    #+#             */
/*   Updated: 2026/11/08 22:24:51 by jojo8356         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "so_long.h"

/*
** The one bonus hook the shared loop needs.
**
** game_hooks.c is compiled into both binaries, so it cannot reference a bonus
** symbol directly: the mandatory link would fail. Every bonus source provides
** `bonus_frame`, and the mandatory build links the empty one below in
** src/bonus_stub.c. One indirection, and the two flavours share the loop.
*/

void	bonus_frame(t_game *g)
{
	bonus_tick(g);
	bonus_draw_hud(g);
}
