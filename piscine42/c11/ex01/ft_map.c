/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_map.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/08 19:00:00 by jojo8356          #+#    #+#             */
/*   Updated: 2026/11/08 19:00:00 by jojo8356         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

/* Renvoie un tableau malloc contenant f applique a chaque element. */
#include <stdlib.h>

int	*ft_map(int *tab, int length, int (*f)(int))
{
	int	*res;
	int	i;

	res = malloc(sizeof(int) * length);
	if (!res)
		return (0);
	i = 0;
	while (i < length)
	{
		res[i] = f(tab[i]);
		i++;
	}
	return (res);
}
