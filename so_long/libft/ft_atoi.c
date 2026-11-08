/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_atoi.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: student42 <student42@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/04 08:00:00 by student42         #+#    #+#             */
/*   Updated: 2026/11/04 08:00:00 by student42        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

static int	ft_parse_digits(const char *s, int sign)
{
	unsigned long	acc;
	unsigned long	cutoff;
	unsigned long	digit;

	acc = 0;
	cutoff = ~0UL >> 1;
	while (*s >= '0' && *s <= '9')
	{
		digit = (unsigned long)(*s - '0');
		if (sign == 1 && acc > (cutoff - digit) / 10)
			return ((int)cutoff);
		if (sign == -1 && acc > (cutoff + 1UL - digit) / 10)
			return ((int)(0UL - cutoff - 1UL));
		acc = acc * 10 + digit;
		s++;
	}
	if (sign == -1)
		return ((int)(0UL - acc));
	return ((int)acc);
}

int	ft_atoi(const char *nptr)
{
	int	sign;

	sign = 1;
	while (*nptr == ' ' || (*nptr >= '\t' && *nptr <= '\r'))
		nptr++;
	if (*nptr == '-' || *nptr == '+')
	{
		if (*nptr == '-')
			sign = -1;
		nptr++;
	}
	return (ft_parse_digits(nptr, sign));
}
