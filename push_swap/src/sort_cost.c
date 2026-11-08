/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   sort_cost.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/08 21:43:34 by jojo8356          #+#    #+#             */
/*   Updated: 2026/11/08 21:43:34 by jojo8356         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "push_swap.h"

/*
** The cost model behind the sort. Choosing which value to push where is the
** whole performance question of push_swap, and these three functions are the
** complete answer: how far a target is from the top of a stack, what that pair
** of rotations really costs, and where a value belongs inside b.
*/

int	rot_cost(int size, int idx)
{
	if (idx <= size / 2)
		return (idx);
	return (size - idx);
}

/*
** Cost of the pair (rotate a to ia, rotate b to ib). Rotating both the same way
** for the shared prefix costs one instruction per step instead of two, so
** min(ia, ib) steps are free. That discount is what brings 500 values from
** ~6000 to ~5150 moves: without it the model over-estimates every pair whose
** targets lie in the same half, and picks the wrong slot.
*/

int	pair_cost(int sa, int ia, int sb, int ib)
{
	int	best;
	int	fwd;
	int	bwd;

	best = rot_cost(sa, ia) + rot_cost(sb, ib);
	fwd = (ia > ib ? ia : ib);
	bwd = (sa - ia > sb - ib ? sa - ia : sb - ib);
	if (ia <= sa / 2 && ib <= sb / 2 && fwd < best)
		best = fwd;
	if (ia > sa / 2 && ib > sb / 2 && bwd < best)
		best = bwd;
	return (best);
}

/*
** Landing slot for `v` in b: right under the largest smaller value. When
** nothing in b is smaller, v belongs above the maximum, which is the same
** position the rotation would take it to.
*/

int	ps_b_place(const t_stack *b, int v, int *pos)
{
	int	i;
	int	best;

	best = -1;
	*pos = -1;
	i = 0;
	while (i < b->size)
	{
		if (b->data[i] < v && b->data[i] > best)
		{
			best = b->data[i];
			*pos = i;
		}
		i++;
	}
	if (*pos < 0)
		*pos = index_of_max(b);
	return (*pos);
}
