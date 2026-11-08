/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   rows.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/08 19:00:00 by jojo8356          #+#    #+#             */
/*   Updated: 2026/11/08 19:00:00 by jojo8356         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

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

static void	gen(t_ctx *g, int *cur, int *st, int *found)
{
	int	v;
	int	nxt[3];

	if (st[2] == g->n && visible_count(cur, g->n, 1) != g->clues[3 * g->n + *st])
		return ;
	if (st[2] == g->n && ++(*found))
	{
		v = -1;
		while (++v < g->n)
			g->cand[*st][(*found - 1) * g->n + v] = cur[v];
		return ;
	}
	v = 0;
	while (++v <= g->n)
	{
		if (!(st[1] & (1 << v)))
		{
			cur[st[2]] = v;
			nxt[0] = st[0];
			nxt[1] = st[1] | (1 << v);
			nxt[2] = st[2] + 1;
			if (partial_left_ok(cur, st[2], g->n, g->clues[2 * g->n + st[0]]))
				gen(g, cur, nxt, found);
		}
	}
}

int	build_rows(t_ctx *g)
{
	int	cur[9];
	int	st[3];
	int	r;

	r = 0;
	while (r < g->n)
	{
		g->cand[r] = malloc(sizeof(int) * g->n * fact(g->n));
		if (!g->cand[r])
			return (0);
		g->cnt[r] = 0;
		st[0] = r;
		st[1] = 0;
		st[2] = 0;
		gen(g, cur, st, &g->cnt[r]);
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
