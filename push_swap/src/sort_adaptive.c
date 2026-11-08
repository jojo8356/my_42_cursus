/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   sort_adaptive.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/08 21:36:54 by jojo8356          #+#    #+#             */
/*   Updated: 2026/11/08 21:36:54 by jojo8356         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "push_swap.h"

/*
** Entry point of the sorting strategy. Everything below 6 values is done by
** dedicated code because general algorithms are wasteful there, and everything
** above goes through the rank + chunk + insert pipeline.
*/

void	sort_adaptive(t_stack *a, t_stack *b, t_ops *o)
{
	if (is_sorted(a))
		return ;
	if (a->size <= 1)
		return ;
	if (a->size == 2)
		sort_two(a, b, o);
	else if (a->size <= 3)
		sort_three(a, b, o);
	else if (a->size <= 5)
		sort_small(a, b, o);
	else
	{
		if (!ps_rank(a))
			return ;
		sort_chunk(a, b, o);
		sort_insert(a, b, o);
	}
}
