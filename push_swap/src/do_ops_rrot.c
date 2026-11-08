/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   do_ops_rrot.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/08 21:37:50 by jojo8356          #+#    #+#             */
/*   Updated: 2026/11/08 21:37:50 by jojo8356         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "push_swap.h"

/*
** Backward rotations, mirroring do_ops_rot.c. `rr` and `rrr` are only emitted
** when both stacks are large enough to actually move: emitting them on a
** single-element stack would be a wasted instruction, and the benchmark counts
** instructions.
*/

void	do_rra(t_stack *a, t_stack *b, t_ops *o)
{
	(void)b;
	if (a->size > 1)
	{
		st_reverse(a);
		op_emit(o, "rra");
	}
}

void	do_rrb(t_stack *a, t_stack *b, t_ops *o)
{
	(void)a;
	if (b->size > 1)
	{
		st_reverse(b);
		op_emit(o, "rrb");
	}
}

/*
** Mirror of `rr`: rrr rotates both stacks, and rotates whichever of them can be
** rotated when the other is too small. Cancelling the whole instruction would
** lose the rotation that did apply.
*/

void	do_rrr(t_stack *a, t_stack *b, t_ops *o)
{
	if (a->size < 2 && b->size < 2)
		return ;
	st_reverse(a);
	st_reverse(b);
	op_emit(o, "rrr");
}
