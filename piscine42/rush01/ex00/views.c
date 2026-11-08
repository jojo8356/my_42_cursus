/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   views.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/08 19:00:00 by jojo8356          #+#    #+#             */
/*   Updated: 2026/11/08 19:00:00 by jojo8356         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

/* Vues d'une ligne et bornes partielles sur les colonnes. */
#include "rush01.h"

int	visible_count(int *line, int n, int from_end)
{
	int	vis;
	int	max;
	int	i;

	vis = 0;
	max = 0;
	i = 0;
	while (i < n)
	{
		if (line[(n - 1) * from_end + i * (1 - 2 * from_end)] > max)
		{
			max = line[(n - 1) * from_end + i * (1 - 2 * from_end)];
			vis++;
		}
		i++;
	}
	return (vis);
}

int	col_top_ok(t_ctx *g, int depth, int *row, int c)
{
	int	col[9];
	int	vis;
	int	i;

	if (g->col_used[c] & (1 << row[c]))
		return (0);
	i = 0;
	while (i < depth)
	{
		col[i] = g->grid[i * g->n + c];
		i++;
	}
	col[depth] = row[c];
	vis = visible_count(col, depth + 1, 0);
	if (vis > g->clues[c])
		return (0);
	return (g->clues[c] <= vis + (g->n - 1 - depth));
}

int	max_unused(t_ctx *g, int c, int extra)
{
	int	used;
	int	m;
	int	v;

	used = g->col_used[c] | (1 << extra);
	m = 0;
	v = 1;
	while (v <= g->n)
	{
		if (!(used & (1 << v)) && v > m)
			m = v;
		v++;
	}
	return (m);
}

int	bot_contrib(int *col, int depth, int max_s)
{
	int	vis;
	int	i;

	vis = 0;
	i = depth;
	while (i >= 0)
	{
		if (col[i] > max_s)
		{
			max_s = col[i];
			vis++;
		}
		i--;
	}
	return (vis);
}

int	col_bot_ok(t_ctx *g, int depth, int *row, int c)
{
	int	col[9];
	int	known;
	int	rest;
	int	i;

	i = 0;
	while (i < depth)
	{
		col[i] = g->grid[i * g->n + c];
		i++;
	}
	col[depth] = row[c];
	rest = g->n - 1 - depth;
	if (rest == 0)
		return (visible_count(col, depth + 1, 1) == g->clues[g->n + c]);
	known = bot_contrib(col, depth, max_unused(g, c, row[c]));
	if (1 + known > g->clues[g->n + c])
		return (0);
	return (g->clues[g->n + c] <= rest + known);
}
