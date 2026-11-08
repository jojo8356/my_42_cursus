/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_show_tab.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/08 19:00:00 by jojo8356          #+#    #+#             */
/*   Updated: 2026/11/08 19:00:00 by jojo8356         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

/* Affiche le tableau de t_stock_str cree par ft_strs_to_tab : pour
   chaque element, la chaine, sa taille, puis la copie. */
#include <unistd.h>
#include "ft_stock_str.h"

static void	put_nbr(int n)
{
	char			buf[12];
	int				i;
	unsigned int	nb;

	nb = 0;
	if (n < 0)
	{
		write(1, "-", 1);
		nb = -(unsigned int)n;
	}
	else
		nb = (unsigned int)n;
	i = 0;
	if (nb == 0)
		buf[i++] = '0';
	while (nb > 0)
	{
		buf[i++] = '0' + nb % 10;
		nb /= 10;
	}
	while (i > 0)
		write(1, &buf[--i], 1);
}

static void	put_str(char *str)
{
	while (*str)
	{
		write(1, str, 1);
		str++;
	}
	write(1, "\n", 1);
}

void	ft_show_tab(struct s_stock_str *par)
{
	int	i;

	i = 0;
	while (par[i].str)
	{
		put_str(par[i].str);
		put_nbr(par[i].size);
		write(1, "\n", 1);
		put_str(par[i].copy);
		i++;
	}
}
