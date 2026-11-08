/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_range.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/08 19:00:00 by jojo8356          #+#    #+#             */
/*   Updated: 2026/11/08 19:00:00 by jojo8356         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

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
