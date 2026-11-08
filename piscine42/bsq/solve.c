/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   solve.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/08 19:00:00 by jojo8356          #+#    #+#             */
/*   Updated: 2026/11/08 19:00:00 by jojo8356         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

/* Resolution : programmation dynamique O(rows*cols), puis trace
   du carre maximal (en cas d'exaequo : le plus haut, puis le plus
   a gauche, obtenu en ne remplacant que sur taille STRICTEMENT
   superieure). */
#include <stdlib.h>
#include "bsq.h"

void	free_map(t_map *m)
{
	int	r;

	if (!m->grid)
		return ;
	r = 0;
	while (r < m->rows)
	{
		if (m->grid[r])
			free(m->grid[r]);
		r++;
	}
	free(m->grid);
	m->grid = 0;
}

static void	fill_square(t_map *m, int r, int c, int size)
{
	int	i;
	int	j;

	i = r - size + 1;
	while (i <= r)
	{
		j = c - size + 1;
		while (j <= c)
		{
			m->grid[i][j] = m->full;
			j++;
		}
		i++;
	}
}

static int	cell_dp(int *prev, int *cur, int c)
{
	int	val;

	if (c == 0)
		return (1);
	val = 1 + prev[c];
	if (cur[c - 1] + 1 < val)
		val = cur[c - 1] + 1;
	if (prev[c - 1] + 1 < val)
		val = prev[c - 1] + 1;
	return (val);
}

static void	step_row(t_map *m, int r, int *dp[2], int *best)
{
	int	c;

	c = 0;
	while (c < m->cols)
	{
		dp[1][c] = 0;
		if (m->grid[r][c] == m->empty)
		{
			dp[1][c] = cell_dp(dp[0], dp[1], c);
			if (dp[1][c] > best[0])
			{
				best[0] = dp[1][c];
				best[1] = r;
				best[2] = c;
			}
		}
		c++;
	}
	c = -1;
	while (++c < m->cols)
		dp[0][c] = dp[1][c];
}

void	solve(t_map *m)
{
	int	*dp[2];
	int	best[3];
	int	i;

	dp[0] = malloc(sizeof(int) * (m->cols + 1));
	dp[1] = malloc(sizeof(int) * (m->cols + 1));
	if (!dp[0] || !dp[1])
	{
		free(dp[0]);
		free(dp[1]);
		return ;
	}
	best[0] = 0;
	i = -1;
	while (++i < m->cols)
		dp[0][i] = 0;
	i = -1;
	while (++i < m->rows)
		step_row(m, i, dp, best);
	if (best[0] > 0)
		fill_square(m, best[1], best[2], best[0]);
	free(dp[0]);
	free(dp[1]);
}
