/* Filtrage par coherence d'arc : supprime les lignes candidates qui
   ne peuvent coexister avec aucune candidate d'une autre ligne. */
#include "rush01.h"

static int	rows_compatible(t_ctx *g, int r, int k, int r2, int k2)
{
	int	c;

	c = 0;
	while (c < g->n)
	{
		if (g->cand[r][k * g->n + c] == g->cand[r2][k2 * g->n + c])
			return (0);
		c++;
	}
	return (1);
}

static int	partner_missing(t_ctx *g, int r, int k)
{
	int	r2;
	int	j;
	int	found;

	r2 = 0;
	while (r2 < g->n)
	{
		found = 0;
		j = 0;
		while (r2 != r && j < g->cnt[r2] && !found)
		{
			if (rows_compatible(g, r, k, r2, j))
				found = 1;
			j++;
		}
		if (r2 != r && !found)
			return (r2);
		r2++;
	}
	return (-1);
}

static void	copy_cand(t_ctx *g, int r, int from, int to)
{
	int	c;

	c = 0;
	while (c < g->n)
	{
		g->cand[r][to * g->n + c] = g->cand[r][from * g->n + c];
		c++;
	}
}

void	filter_rows(t_ctx *g)
{
	int	r;
	int	k;
	int	changed;

	changed = 1;
	while (changed)
	{
		changed = 0;
		r = -1;
		while (++r < g->n)
		{
			k = 0;
			while (k < g->cnt[r])
			{
				if (partner_missing(g, r, k) >= 0)
				{
					g->cnt[r]--;
					copy_cand(g, r, g->cnt[r], k);
					changed = 1;
				}
				else
					k++;
			}
		}
	}
}
