/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   sort_chunk.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/08 21:43:34 by jojo8356          #+#    #+#             */
/*   Updated: 2026/11/08 21:43:34 by jojo8356         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "push_swap.h"

/*
** Phase 1 of the sort: empty a into b, each value at the slot the cost model
** says is cheapest now and later.
**
** Rotating a is free during this phase. a is not sorted yet, and phase 2
** rebuilds it from b, so nothing an extra rotation of a does can be broken.
** That is why the cost model is allowed to move a as much as it likes to save
** a rotation of b.
*/

void	ps_rotate_to(t_stack *a, t_stack *b, int idx, int which, t_ops *o)
{
	t_stack	*s;
	int		half;

	s = a;
	if (which == 'b')
		s = b;
	half = s->size / 2;
	while (idx > 0 && idx <= half)
	{
		if (which == 'b')
			do_rb(a, b, o);
		else
			do_ra(a, b, o);
		idx--;
	}
	while (idx > half && idx < s->size)
	{
		if (which == 'b')
			do_rrb(a, b, o);
		else
			do_rra(a, b, o);
		idx++;
	}
}

/*
** Rotate a and b towards their targets together: the shared prefix is paid
** once, with rr or rrr, and the remainder separately. Splitting this in two
** loops is the whole point of the function, and it is why `rotate_pair` cannot
** be replaced by two `ps_rotate_to` calls.
*/

static void	rotate_same_way(t_stack *a, t_stack *b, int *ia, int *ib, t_ops *o)
{
	int	forward;

	forward = (*ia <= a->size / 2 && *ib <= b->size / 2);
	if (forward)
	{
		while (*ia > 0 && *ib > 0)
		{
			do_rr(a, b, o);
			(*ia)--;
			(*ib)--;
		}
	}
	else if (*ia > a->size / 2 && *ib > b->size / 2)
	{
		while (*ia < a->size && *ib < b->size)
		{
			do_rrr(a, b, o);
			(*ia)++;
			(*ib)++;
		}
	}
}

void	rotate_pair(t_stack *a, t_stack *b, int ia, int ib, t_ops *o)
{
	rotate_same_way(a, b, &ia, &ib, o);
	ps_rotate_to(a, b, ia, 'a', o);
	ps_rotate_to(a, b, ib, 'b', o);
}

static void	pick_best(t_stack *a, t_stack *b, int *out)
{
	int	i;
	int	pos;
	int	cost;
	int	best;

	best = -1;
	out[0] = 0;
	out[1] = 0;
	i = 0;
	while (i < a->size)
	{
		cost = pair_cost(a->size, i, b->size,
				ps_b_place(b, a->data[i], &pos));
		if (best < 0 || cost < best)
		{
			best = cost;
			out[0] = i;
			out[1] = pos;
		}
		i++;
	}
}

void	sort_chunk(t_stack *a, t_stack *b, t_ops *o)
{
	int	choice[2];

	while (a->size > 0)
	{
		pick_best(a, b, choice);
		rotate_pair(a, b, choice[0], choice[1], o);
		do_pb(a, b, o);
	}
}
