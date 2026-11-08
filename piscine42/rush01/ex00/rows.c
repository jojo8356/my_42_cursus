/* Generation des lignes candidates : permutations de 1..n verifiant
   les vues gauche/droite de chaque ligne. */
#include <stdlib.h>
#include "rush01.h"

static int	fact(int n)
{
	if (n <= 1)
		return (1);
	return (n * fact(n - 1));
}

static int	partial_left_ok(int *cur, int pos, int n, int want)
{
	int	vis;
	int	max;
	int	i;

	vis = 0;
	max = 0;
	i = 0;
	while (i <= pos)
	{
		if (cur[i] > max)
		{
			max = cur[i];
			vis++;
		}
		i++;
	}
	if (vis > want)
		return (0);
	return (want <= vis + (n - 1 - pos));
}

static void	gen(t_ctx *g, int r, int *cur, int used, int pos, int *found)
{
	int	v;
	int	i;

	if (pos == g->n)
	{
		if (visible_count(cur, g->n, 1) == g->clues[3 * g->n + r])
		{
			i = 0;
			while (i < g->n)
			{
				g->cand[r][(*found) * g->n + i] = cur[i];
				i++;
			}
			(*found)++;
		}
		return ;
	}
	v = 1;
	while (v <= g->n)
	{
		if (!(used & (1 << v)))
		{
			cur[pos] = v;
			if (partial_left_ok(cur, pos, g->n, g->clues[2 * g->n + r]))
				gen(g, r, cur, used | (1 << v), pos + 1, found);
		}
		v++;
	}
}

int	build_rows(t_ctx *g)
{
	int	cur[9];
	int	r;

	r = 0;
	while (r < g->n)
	{
		g->cand[r] = malloc(sizeof(int) * g->n * fact(g->n));
		if (!g->cand[r])
			return (0);
		g->cnt[r] = 0;
		gen(g, r, cur, 0, 0, &g->cnt[r]);
		r++;
	}
	return (1);
}

void	free_rows(t_ctx *g)
{
	int	r;

	r = 0;
	while (r < g->n)
	{
		free(g->cand[r]);
		g->cand[r] = 0;
		r++;
	}
}
