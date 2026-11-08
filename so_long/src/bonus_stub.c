/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   bonus_stub.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/08 22:26:08 by jojo8356          #+#    #+#             */
/*   Updated: 2026/11/08 22:26:08 by jojo8356         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "so_long.h"

/*
** Mandatory build: the map has exactly the five symbols of chapter IV.
**
** The bonus adds a sixth one, so the character predicate is split in two: this
** file answers "is this an extra symbol?" with no, and the bonus build links a
** version that answers with 'X'. That keeps the mandatory binary free of any
** bonus symbol -- not a single #ifdef in the shared code.
*/

int	bonus_charset(char c)
{
	(void)c;
	return (0);
}

void	bonus_frame(t_game *g)
{
	(void)g;
}
