/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_output.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: arena <arena@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/04 08:00:00 by arena             #+#    #+#             */
/*   Updated: 2026/11/04 08:00:00 by arena            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_printf.h"

int	ft_write_char(char c)
{
	if (write(1, &c, 1) <= 0)
		return (-1);
	return (1);
}

int	ft_write_mem(const char *s, int len)
{
	int	total;
	int	r;

	if (len <= 0)
		return (0);
	total = 0;
	while (total < len)
	{
		r = write(1, s + total, len - total);
		if (r <= 0)
			return (-1);
		total += r;
	}
	return (total);
}

int	ft_write_pad(char c, int n)
{
	char	buf[64];
	int		i;
	int		total;
	int		chunk;

	if (n <= 0)
		return (0);
	i = 0;
	while (i < 64)
		buf[i++] = c;
	total = 0;
	while (total < n)
	{
		chunk = n - total;
		if (chunk > 64)
			chunk = 64;
		if (ft_write_mem(buf, chunk) < 0)
			return (-1);
		total += chunk;
	}
	return (total);
}

int	ft_strlen(const char *s)
{
	int	i;

	i = 0;
	while (s[i])
		i++;
	return (i);
}
