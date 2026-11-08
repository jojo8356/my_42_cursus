/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   sort_small2.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/08 21:36:54 by jojo8356          #+#    #+#             */
/*   Updated: 2026/11/08 21:36:54 by jojo8356         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "push_swap.h"

static void	push_min(t_stack *a, t_stack *b, t_ops *o)
{
	int	idx;
	int	half;

	idx = index_of_min(a);
	half = a->size / 2;
	while (idx > 0 && idx <= half)
	{
		do_ra(a, b, o);
		idx--;
	}
	while (idx > half && idx < a->size)
	{
		do_rra(a, b, o);
		idx++;
	}
	do_pb(a, b, o);
}

/*
** For 4 and 5 values, pushing the minimum to b once or twice leaves three
** values in a, which sort_three handles in at most two instructions.
*/

void	sort_small(t_stack *a, t_stack *b, t_ops *o)
{
	while (a->size > 3)
		push_min(a, b, o);
	sort_three(a, b, o);
	while (b->size > 0)
		do_pa(a, b, o);
}
