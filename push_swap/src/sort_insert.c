/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   sort_insert.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/08 21:37:58 by jojo8356          #+#    #+#             */
/*   Updated: 2026/11/08 21:37:58 by jojo8356         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "push_swap.h"

/*
** Phase 2: drain b back into a.
**
** a must stay sorted from here on, so a is never rotated: any rotation of a
** would insert the next value at the wrong depth. Only b moves, always towards
** its maximum, because the maximum is the value that belongs at the bottom of
** what is already in a.
**
** The saving comes from recomputing the maximum after every push: b shrinks, so
** the distance to the new maximum keeps falling instead of being rediscovered
** from the same starting point each time.
*/

static void	rotate_to_max(t_stack *a, t_stack *b, t_ops *o)
{
	int	max;
	int	down;

	max = index_of_max(b);
	if (max <= b->size / 2)
	{
		while (max-- > 0)
			do_rb(a, b, o);
	}
	else
	{
		down = b->size - max;
		while (down-- > 0)
			do_rrb(a, b, o);
	}
}

void	sort_insert(t_stack *a, t_stack *b, t_ops *o)
{
	while (b->size > 0)
	{
		rotate_to_max(a, b, o);
		do_pa(a, b, o);
	}
}
