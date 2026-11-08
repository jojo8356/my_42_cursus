/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_write_number_bonus.c                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: arena <arena@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/04 08:00:00 by arena             #+#    #+#             */
/*   Updated: 2026/11/04 08:00:00 by arena            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_printf_bonus.h"

static int	ft_num_body(t_num *n, int zeros)
{
	if (ft_write_mem(n->prefix, n->plen) < 0)
		return (-1);
	if (ft_write_pad('0', zeros) < 0)
		return (-1);
	if (ft_write_mem(n->digits, n->dlen) < 0)
		return (-1);
	return (n->plen + zeros + n->dlen);
}

static int	ft_num_left(t_num *n, int pz, int pad)
{
	int	body;

	body = ft_num_body(n, pz);
	if (body < 0)
		return (-1);
	if (ft_write_pad(' ', pad) < 0)
		return (-1);
	return (body + pad);
}

static int	ft_num_zero(t_num *n, int pad)
{
	return (ft_num_body(n, pad));
}

static int	ft_num_right(t_num *n, int pz, int pad)
{
	int	body;

	if (ft_write_pad(' ', pad) < 0)
		return (-1);
	body = ft_num_body(n, pz);
	if (body < 0)
		return (-1);
	return (body + pad);
}

int	ft_write_number(t_spec *s, t_num *n)
{
	int	digit_len;
	int	content;
	int	pz;
	int	pad;

	if (s->precision >= 0 && n->dlen == 0)
		digit_len = 0;
	else if (s->precision > n->dlen)
		digit_len = s->precision;
	else
		digit_len = n->dlen;
	pz = digit_len - n->dlen;
	content = n->plen + digit_len;
	pad = s->width - content;
	if (pad < 0)
		pad = 0;
	if (s->minus)
		return (ft_num_left(n, pz, pad));
	if (s->zero && s->precision < 0)
		return (ft_num_zero(n, pad));
	return (ft_num_right(n, pz, pad));
}
