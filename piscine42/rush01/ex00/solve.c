/* Backtracking sur les lignes candidates + forward checking. */
#include "rush01.h"

static int	cand_compatible(t_ctx *g, int r, int k)
{
	int	*row;
	int	c;

	row = &g->cand[r][k * g->n];
	c = 0;
	while (c < g->n)
	{
		if (g->col_used[c] & (1 << row[c]))
			return (0);
		c++;
	}
	return (1);
}

static int	future_ok(t_ctx *g, int depth)
{
	int	r;
	int	k;
	int	found;

	r = depth + 1;
	while (r < g->n)
	{
		found = 0;
		k = 0;
		while (k < g->cnt[r] && !found)
		{
			if (cand_compatible(g, r, k))
				found = 1;
			k++;
		}
		if (!found)
			return (0);
		r++;
	}
	return (1);
}

static int	row_place_ok(t_ctx *g, int depth, int *row)
{
	int	c;

	c = 0;
	while (c < g->n)
	{
		if (!col_top_ok(g, depth, row, c) || !col_bot_ok(g, depth, row, c))
			return (0);
		c++;
	}
	return (1);
}

int	solve(t_ctx *g, int depth)
{
	int	k;
	int	*row;
	int	i;

	if (depth == g->n)
		return (1);
	k = 0;
	while (k < g->cnt[depth])
	{
		row = &g->cand[depth][k * g->n];
		if (row_place_ok(g, depth, row))
		{
			i = -1;
			while (++i < g->n)
			{
				g->grid[depth * g->n + i] = row[i];
				g->col_used[i] |= (1 << row[i]);
			}
			if (future_ok(g, depth) && solve(g, depth + 1))
				return (1);
			i = -1;
			while (++i < g->n)
				g->col_used[i] &= ~(1 << row[i]);
		}
		k++;
	}
	return (0);
}
