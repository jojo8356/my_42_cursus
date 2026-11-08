/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   sort_small.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/08 21:36:54 by jojo8356          #+#    #+#             */
/*   Updated: 2026/11/08 21:36:54 by jojo8356         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "push_swap.h"

int	is_sorted(const t_stack *a)
{
	int	i;

	i = 0;
	while (i + 1 < a->size)
	{
		if (a->data[i] > a->data[i + 1])
			return (0);
		i++;
	}
	return (1);
}

int	index_of_min(const t_stack *a)
{
	int	i;
	int	best;

	i = 1;
	best = 0;
	while (i < a->size)
	{
		if (a->data[i] < a->data[best])
			best = i;
		i++;
	}
	return (best);
}

int	index_of_max(const t_stack *a)
{
	int	i;
	int	best;

	i = 1;
	best = 0;
	while (i < a->size)
	{
		if (a->data[i] > a->data[best])
			best = i;
		i++;
	}
	return (best);
}

void	sort_two(t_stack *a, t_stack *b, t_ops *o)
{
	(void)b;
	if (a->size == 2 && a->data[0] > a->data[1])
		do_sa(a, b, o);
}

/*
** Three values: six permutations, at most two instructions each. Written out
** instead of computed because a lookup is shorter than the logic that derives
** it, and this file has to stay inside the 25-line rule.
*/

void	sort_three(t_stack *a, t_stack *b, t_ops *o)
{
	int	top;
	int	mid;
	int	bot;

	top = a->data[0];
	mid = a->data[1];
	bot = a->data[2];
	if (top > mid && top > bot)
		do_ra(a, b, o);
	else if (mid > bot && mid > top)
		do_rra(a, b, o);
	if (a->data[0] > a->data[1])
		do_sa(a, b, o);
	if (!is_sorted(a))
		do_ra(a, b, o);
}
