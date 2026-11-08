/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ps_rank.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/08 21:37:58 by jojo8356          #+#    #+#             */
/*   Updated: 2026/11/08 21:37:58 by jojo8356         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "push_swap.h"

/*
** Replace every value by its rank in [0, n): 4 67 3 becomes 1 3 0. Everything
** downstream then works on small dense integers, so no algorithm has to worry
** about magnitude, overflow, or negative values again.
**
** O(n^2) on purpose: n is at most a few hundred and the constant of a counting
** loop beats the setup cost of anything cleverer.
*/

static void	fill_ranks(const t_stack *a, int *ranks)
{
	int	i;
	int	j;
	int	less;

	i = 0;
	while (i < a->size)
	{
		less = 0;
		j = 0;
		while (j < a->size)
		{
			if (a->data[j] < a->data[i])
				less++;
			j++;
		}
		ranks[i] = less;
		i++;
	}
}

int	ps_rank(t_stack *a)
{
	int	*ranks;
	int	i;

	ranks = (int *)malloc(sizeof(int) * (size_t)a->size);
	if (!ranks)
		return (0);
	fill_ranks(a, ranks);
	i = 0;
	while (i < a->size)
	{
		a->data[i] = ranks[i];
		i++;
	}
	free(ranks);
	return (1);
}
