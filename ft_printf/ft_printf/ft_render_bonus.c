/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_render_bonus.c                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: arena <arena@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/04 08:00:00 by arena             #+#    #+#             */
/*   Updated: 2026/11/04 08:00:00 by arena            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_printf_bonus.h"

int	ft_render_percent(void)
{
	return (ft_write_char('%'));
}

int	ft_write_string(t_spec *s, const char *str, int len)
{
	int	pad;

	pad = s->width - len;
	if (pad < 0)
		pad = 0;
	if (s->minus)
	{
		if (ft_write_mem(str, len) < 0 || ft_write_pad(' ', pad) < 0)
			return (-1);
		return (len + pad);
	}
	if (ft_write_pad(' ', pad) < 0 || ft_write_mem(str, len) < 0)
		return (-1);
	return (len + pad);
}

int	ft_render_char(t_spec *s, va_list *args)
{
	char	c;
	int		pad;

	c = (char)va_arg(*args, int);
	pad = s->width - 1;
	if (pad < 0)
		pad = 0;
	if (s->minus)
	{
		if (ft_write_char(c) < 0 || ft_write_pad(' ', pad) < 0)
			return (-1);
		return (1 + pad);
	}
	if (ft_write_pad(' ', pad) < 0 || ft_write_char(c) < 0)
		return (-1);
	return (1 + pad);
}

int	ft_render_str(t_spec *s, va_list *args)
{
	const char	*str;
	int			len;

	str = va_arg(*args, const char *);
	if (str == NULL)
	{
		if (s->precision >= 0 && s->precision < 6)
			str = "";
		else
			str = "(null)";
	}
	len = ft_strlen(str);
	if (s->precision >= 0 && s->precision < len)
		len = s->precision;
	return (ft_write_string(s, str, len));
}

int	ft_render_spec(const char *fmt, t_spec *s, va_list *args)
{
	char	c;

	c = s->conv;
	if (c == 'c')
		return (ft_render_char(s, args));
	if (c == 's')
		return (ft_render_str(s, args));
	if (c == 'p')
		return (ft_render_ptr(s, args));
	if (c == 'd' || c == 'i')
		return (ft_render_int(s, args));
	if (c == 'u')
		return (ft_render_uint(s, args));
	if (c == 'x' || c == 'X')
		return (ft_render_hex(s, args));
	if (c == '%')
		return (ft_render_percent());
	return (ft_write_mem(fmt + s->start, s->end - s->start));
}
