/* Affichage de la grille et des erreurs (autorise : write). */
#include <unistd.h>
#include "rush01.h"

void	print_grid(t_ctx *g)
{
	char	line[20];
	int		i;
	int		j;

	i = 0;
	while (i < g->n)
	{
		j = 0;
		while (j < g->n)
		{
			line[j * 2] = (char)('0' + g->grid[i * g->n + j]);
			line[j * 2 + 1] = ' ';
			j++;
		}
		line[g->n * 2 - 1] = '\n';
		write(1, line, g->n * 2);
		i++;
	}
}

void	print_error(void)
{
	write(1, "Error\n", 6);
}
