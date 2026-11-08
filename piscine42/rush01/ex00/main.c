/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/08 19:00:00 by jojo8356          #+#    #+#             */
/*   Updated: 2026/11/08 19:00:00 by jojo8356         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

/* Rush01 : ./rush01 "col_top col_bottom row_left row_right".
   Affiche la 1re solution, ou Error si entree invalide / insoluble.
   Bonus : tailles de 1x1 a 9x9. */
#include "rush01.h"

static void	init(t_ctx *g)
{
	int	i;

	i = 0;
	while (i < 9)
	{
		g->col_used[i] = 0;
		g->cand[i] = 0;
		g->cnt[i] = 0;
		i++;
	}
}

int	main(int argc, char **argv)
{
	t_ctx	g;
	int		ok;

	if (argc != 2)
	{
		print_error();
		return (1);
	}
	init(&g);
	if (!parse_clues(argv[1], &g) || !build_rows(&g))
	{
		print_error();
		return (1);
	}
	filter_rows(&g);
	ok = solve(&g, 0);
	free_rows(&g);
	if (!ok)
	{
		print_error();
		return (1);
	}
	print_grid(&g);
	return (0);
}
