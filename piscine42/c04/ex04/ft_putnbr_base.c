/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_putnbr_base.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/08 19:00:00 by jojo8356          #+#    #+#             */
/*   Updated: 2026/11/08 19:00:00 by jojo8356         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

/* Affiche l'entier nbr dans la base donnee. Si la base est invalide
   (moins de 2 symboles, '+'/'-', doublons), n'affiche rien. */
#include <unistd.h>

static int	base_ok(char *base, int *len)
{
	int	i;
	int	j;

	*len = 0;
	while (base[*len])
	{
		if (base[*len] == '+' || base[*len] == '-')
			return (0);
		(*len)++;
	}
	if (*len < 2)
		return (0);
	i = 0;
	while (i < *len)
	{
		j = i + 1;
		while (j < *len)
		{
			if (base[i] == base[j])
				return (0);
			j++;
		}
		i++;
	}
	return (1);
}

static void	put_base(unsigned int nb, char *base, int len)
{
	char	c;

	if (nb >= (unsigned int)len)
		put_base(nb / len, base, len);
	c = base[nb % len];
	write(1, &c, 1);
}

void	ft_putnbr_base(int nbr, char *base)
{
	int	len;

	if (!base_ok(base, &len))
		return ;
	if (nbr < 0)
	{
		write(1, "-", 1);
		put_base(-(unsigned int)nbr, base, len);
	}
	else
		put_base((unsigned int)nbr, base, len);
}
