/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   do_ops.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/08 21:37:50 by jojo8356          #+#    #+#             */
/*   Updated: 2026/11/08 21:37:50 by jojo8356         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "push_swap.h"

/*
** The five instructions that only move what is on top. Each one changes the
** stacks and writes its name, in that order: the binary cannot log an
** instruction it did not apply, which is what makes the output trustworthy.
**
** The files are split by group (swaps, pushes, rotates) purely to stay inside
** the Norm's five-functions-per-file rule.
*/

void	do_sa(t_stack *a, t_stack *b, t_ops *o)
{
	(void)b;
	st_swap(a);
	op_emit(o, "sa");
}

void	do_sb(t_stack *a, t_stack *b, t_ops *o)
{
	(void)a;
	st_swap(b);
	op_emit(o, "sb");
}

void	do_ss(t_stack *a, t_stack *b, t_ops *o)
{
	st_swap(a);
	st_swap(b);
	op_emit(o, "ss");
}

void	do_pa(t_stack *a, t_stack *b, t_ops *o)
{
	if (st_push(a, b))
		op_emit(o, "pa");
}

void	do_pb(t_stack *a, t_stack *b, t_ops *o)
{
	if (st_push(b, a))
		op_emit(o, "pb");
}
