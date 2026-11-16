/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_itoa.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: student42 <student42@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/04 08:00:00 by student42         #+#    #+#             */
/*   Updated: 2026/11/04 08:00:00 by student42        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

static size_t	ft_num_len(int n)
{
	size_t			len;
	unsigned int	u;

	if (n == 0)
		return (1);
	len = 0;
	if (n < 0)
	{
		len = 1;
		u = (unsigned int)(0U - (unsigned int)n);
	}
	else
		u = (unsigned int)n;
	while (u > 0)
	{
		u /= 10;
		len++;
	}
	return (len);
}

char	*ft_itoa(int n)
{
	char			*str;
	size_t			len;
	unsigned int	u;

	len = ft_num_len(n);
	str = (char *)malloc(len + 1);
	if (!str)
		return (NULL);
	str[len] = '\0';
	if (n < 0)
	{
		str[0] = '-';
		u = (unsigned int)(0U - (unsigned int)n);
	}
	else
		u = (unsigned int)n;
	if (u == 0)
		str[0] = '0';
	while (u > 0)
	{
		len--;
		str[len] = (char)('0' + (u % 10));
		u /= 10;
	}
	return (str);
}
