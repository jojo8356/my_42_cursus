/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_render_num_bonus.c                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: arena <arena@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/04 08:00:00 by arena             #+#    #+#             */
/*   Updated: 2026/11/04 08:00:00 by arena            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_printf_bonus.h"

int	ft_render_int(t_spec *s, va_list *args)
{
	char		buf[32];
	long long	v;
	t_num		n;

	v = va_arg(*args, int);
	n.prefix = "";
	n.plen = 0;
	if (v < 0)
	{
		n.prefix = "-";
		n.plen = 1;
		v = -(v + 1) + 1;
	}
	else if (s->plus)
		n.prefix = "+";
	else if (s->space)
		n.prefix = " ";
	if (n.prefix[0] != '\0')
		n.plen = 1;
	n.dlen = ft_utoa((unsigned long long)v, buf, 10, 0);
	n.digits = buf;
	if (s->precision == 0 && v == 0)
		n.dlen = 0;
	return (ft_write_number(s, &n));
}

int	ft_render_uint(t_spec *s, va_list *args)
{
	char			buf[32];
	unsigned int	v;
	t_num			n;

	v = va_arg(*args, unsigned int);
	n.dlen = ft_utoa(v, buf, 10, 0);
	n.digits = buf;
	n.prefix = "";
	n.plen = 0;
	if (s->precision == 0 && v == 0)
		n.dlen = 0;
	return (ft_write_number(s, &n));
}

int	ft_render_hex(t_spec *s, va_list *args)
{
	char			buf[32];
	unsigned int	v;
	t_num			n;
	int				upper;

	upper = (s->conv == 'X');
	v = va_arg(*args, unsigned int);
	n.dlen = ft_utoa(v, buf, 16, upper);
	n.digits = buf;
	n.prefix = "";
	n.plen = 0;
	if (s->hash && v != 0)
	{
		if (upper)
			n.prefix = "0X";
		else
			n.prefix = "0x";
		n.plen = 2;
	}
	if (s->precision == 0 && v == 0)
		n.dlen = 0;
	return (ft_write_number(s, &n));
}

int	ft_render_ptr(t_spec *s, va_list *args)
{
	char				buf[32];
	char				pref[4];
	unsigned long long	v;
	t_num				n;

	v = (unsigned long long)va_arg(*args, void *);
	if (v == 0)
		return (ft_write_string(s, "(nil)", 5));
	n.dlen = ft_utoa(v, buf, 16, 0);
	n.digits = buf;
	n.plen = 0;
	if (s->plus)
		pref[n.plen++] = '+';
	else if (s->space)
		pref[n.plen++] = ' ';
	pref[n.plen++] = '0';
	pref[n.plen++] = 'x';
	pref[n.plen] = '\0';
	n.prefix = pref;
	return (ft_write_number(s, &n));
}
