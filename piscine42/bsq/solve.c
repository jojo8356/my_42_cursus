/* Resolution : programmation dynamique O(rows*cols), puis trace
   du carre maximal (en cas d'exaequo : le plus haut, puis le plus
   a gauche, obtenu en ne remplacant que sur taille STRICTEMENT
   superieure). */
#include <stdlib.h>
#include "bsq.h"

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

void	solve(t_map *m)
{
	int	*prev;
	int	*cur;
	int	best[3];
	int	p[2];

	prev = malloc(sizeof(int) * (m->cols + 1));
	cur = malloc(sizeof(int) * (m->cols + 1));
	best[0] = 0;
	p[0] = -1;
	if (!prev || !cur)
		return ;
	while (++p[0] < m->cols)
		prev[p[0]] = 0;
	p[0] = -1;
	while (++p[0] < m->rows)
	{
		p[1] = 0;
		while (p[1] < m->cols)
		{
			cur[p[1]] = 0;
			if (m->grid[p[0]][p[1]] == m->empty)
			{
				if (p[1] == 0)
					cur[p[1]] = 1;
				else
				{
					cur[p[1]] = 1 + prev[p[1]];
					if (cur[p[1] - 1] + 1 < cur[p[1]])
						cur[p[1]] = cur[p[1] - 1] + 1;
					if (prev[p[1] - 1] + 1 < cur[p[1]])
						cur[p[1]] = prev[p[1] - 1] + 1;
				}
				if (cur[p[1]] > best[0])
				{
					best[0] = cur[p[1]];
					best[1] = p[0];
					best[2] = p[1];
				}
			}
			p[1]++;
		}
		p[1] = -1;
		while (++p[1] < m->cols)
			prev[p[1]] = cur[p[1]];
	}
	if (best[0] > 0)
		fill_square(m, best[1], best[2], best[0]);
	free(prev);
	free(cur);
}
