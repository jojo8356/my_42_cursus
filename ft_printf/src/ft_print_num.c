/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_print_num.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: arena <arena@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/04 08:00:00 by arena             #+#    #+#             */
/*   Updated: 2026/11/04 08:00:00 by arena            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_printf.h"

int	ft_print_int(int n)
{
	char				buf[32];
	long long			v;
	int					len;
	int					total;

	v = n;
	total = 0;
	if (v < 0)
	{
		if (ft_write_char('-') < 0)
			return (-1);
		total = 1;
		v = -(v + 1) + 1;
	}
	len = ft_utoa((unsigned long long)v, buf, 10, 0);
	if (ft_write_mem(buf, len) < 0)
		return (-1);
	return (total + len);
}

int	ft_print_uint(unsigned int n)
{
	char	buf[32];
	int		len;

	len = ft_utoa(n, buf, 10, 0);
	return (ft_write_mem(buf, len));
}

int	ft_print_hex(unsigned int n, int upper)
{
	char	buf[32];
	int		len;

	len = ft_utoa(n, buf, 16, upper);
	return (ft_write_mem(buf, len));
}
