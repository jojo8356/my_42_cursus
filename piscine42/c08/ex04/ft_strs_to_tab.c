/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_strs_to_tab.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/08 19:00:00 by jojo8356          #+#    #+#             */
/*   Updated: 2026/11/08 19:00:00 by jojo8356         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

/* Transforme un tableau de chaines en tableau de t_stock_str (copie
   incluse). L'array se termine par un element dont str vaut 0.
   Renvoie NULL en cas d'erreur d'allocation. */
#include <stdlib.h>
#include "ft_stock_str.h"

static int	ft_strlen(char *str)
{
	int	len;

	len = 0;
	while (str[len])
		len++;
	return (len);
}

static char	*ft_strdup(char *src)
{
	char	*dup;
	int		len;
	int		i;

	len = ft_strlen(src);
	dup = malloc(len + 1);
	if (!dup)
		return (0);
	i = 0;
	while (i <= len)
	{
		dup[i] = src[i];
		i++;
	}
	return (dup);
}

struct s_stock_str	*ft_strs_to_tab(int ac, char **av)
{
	t_stock_str	*tab;
	int			i;

	tab = malloc(sizeof(t_stock_str) * (ac + 1));
	if (!tab)
		return (0);
	i = 0;
	while (i < ac)
	{
		tab[i].size = ft_strlen(av[i]);
		tab[i].str = av[i];
		tab[i].copy = ft_strdup(av[i]);
		if (!tab[i].copy)
			return (0);
		i++;
	}
	tab[i].str = 0;
	tab[i].copy = 0;
	tab[i].size = 0;
	return (tab);
}
