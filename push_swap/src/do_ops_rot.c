/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   do_ops_rot.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/08 21:37:50 by jojo8356          #+#    #+#             */
/*   Updated: 2026/11/08 21:37:50 by jojo8356         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "push_swap.h"

/*
** Forward rotations. `ra` on a stack of one element would be a no-op followed
** by a logged instruction, so the size is checked first: an instruction that
** changes nothing is not printed, exactly like the subject's "do nothing"
** wording for the swap and push cases.
*/

void	do_ra(t_stack *a, t_stack *b, t_ops *o)
{
	(void)b;
	if (a->size > 1)
	{
		st_rotate(a);
		op_emit(o, "ra");
	}
}

void	do_rb(t_stack *a, t_stack *b, t_ops *o)
{
	(void)a;
	if (b->size > 1)
	{
		st_rotate(b);
		op_emit(o, "rb");
	}
}

/*
** `rr` must do whatever is possible, not nothing: with a single value in b the
** rotation of a still happens and the instruction is still printed. The
** official checker disagrees with any other reading, and a checker that
** cancelled the rotation of a would report KO on sorted inputs.
*/

void	do_rr(t_stack *a, t_stack *b, t_ops *o)
{
	if (a->size < 2 && b->size < 2)
		return ;
	st_rotate(a);
	st_rotate(b);
	op_emit(o, "rr");
}
