/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_ten_queens_puzzle.c                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/08 19:00:00 by jojo8356          #+#    #+#             */
/*   Updated: 2026/11/08 19:00:00 by jojo8356         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

/* Le probleme des dix dames : affiche toutes les solutions du placement
   de 10 reines sur un echiquier 10x10 (une colonne par chiffre), une
   solution par ligne. Renvoie le nombre de solutions (724). */
#include <unistd.h>

static int	safe(int *pos, int col, int row)
{
	int	i;

	i = 0;
	while (i < col)
	{
		if (pos[i] == row)
			return (0);
		if (pos[i] - i == row - col || pos[i] + i == row + col)
			return (0);
		i++;
	}
	return (1);
}

static void	print_sol(int *pos)
{
	int		i;
	char	c;

	i = 0;
	while (i < 10)
	{
		c = pos[i] + '0';
		write(1, &c, 1);
		i++;
	}
	write(1, "\n", 1);
}

static void	solve(int *pos, int col, int *count)
{
	int	row;

	if (col == 10)
	{
		print_sol(pos);
		(*count)++;
		return ;
	}
	row = 0;
	while (row < 10)
	{
		if (safe(pos, col, row))
		{
			pos[col] = row;
			solve(pos, col + 1, count);
		}
		row++;
	}
}

int	ft_ten_queens_puzzle(void)
{
	int	pos[10];
	int	count;

	count = 0;
	solve(pos, 0, &count);
	return (count);
}
