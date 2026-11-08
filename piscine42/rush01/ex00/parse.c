/* Analyse la chaine d'indices : 4N nombres 1..N separes par espaces. */
#include "rush01.h"

static int	parse_all(char *s, int *clues, int total)
{
	int	i;

	i = 0;
	while (i < total)
	{
		if (s[i * 2] < '1' || s[i * 2] > '9')
			return (0);
		clues[i] = s[i * 2] - '0';
		if (i < total - 1 && s[i * 2 + 1] != ' ')
			return (0);
		i++;
	}
	return (s[(total - 1) * 2 + 1] == '\0');
}

int	parse_clues(char *s, t_ctx *g)
{
	int	total;
	int	i;

	total = 0;
	while (s[total])
		total++;
	if (total < 1 || total > 71 || total % 2 == 0)
		return (0);
	total = (total + 1) / 2;
	if (total % 4 != 0)
		return (0);
	g->n = total / 4;
	if (g->n < 1 || g->n > 9)
		return (0);
	if (!parse_all(s, g->clues, total))
		return (0);
	i = 0;
	while (i < total)
	{
		if (g->clues[i] > g->n)
			return (0);
		i++;
	}
	return (1);
}
