/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_putnbr.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/08 19:00:00 by jojo8356          #+#    #+#             */
/*   Updated: 2026/11/08 19:00:00 by jojo8356         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

/* Affiche l'entier passe en parametre (INT_MIN compris). */
#include <unistd.h>

void	ft_putnbr(int n)
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
