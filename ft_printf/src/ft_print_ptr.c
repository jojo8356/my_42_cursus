/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_print_ptr.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: arena <arena@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/04 08:00:00 by arena             #+#    #+#             */
/*   Updated: 2026/11/04 08:00:00 by arena            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_printf.h"

int	ft_print_ptr(void *p)
{
	char				buf[32];
	unsigned long long	v;
	int					len;

	if (p == NULL)
		return (ft_write_mem("(nil)", 5));
	v = (unsigned long long)p;
	if (ft_write_mem("0x", 2) < 0)
		return (-1);
	len = ft_utoa(v, buf, 16, 0);
	if (ft_write_mem(buf, len) < 0)
		return (-1);
	return (2 + len);
}
