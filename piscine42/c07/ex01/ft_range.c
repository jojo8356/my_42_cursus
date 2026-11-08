/* Renvoie un tableau malloc contenant les entiers de min a max,
   min inclus et max exclu. Renvoie NULL si min >= max. */
#include <stdlib.h>

int	*ft_range(int min, int max)
{
	int	*tab;
	int	size;
	int	i;

	if (min >= max)
		return (0);
	size = max - min;
	tab = malloc(sizeof(int) * size);
	if (!tab)
		return (0);
	i = 0;
	while (i < size)
	{
		tab[i] = min + i;
		i++;
	}
	return (tab);
}
