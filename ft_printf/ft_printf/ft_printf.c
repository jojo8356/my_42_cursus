/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_printf.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: arena <arena@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/04 08:00:00 by arena          #+#    #+#             */
/*   Updated: 2026/11/04 08:00:00 by arena         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_printf.h"

static int	ft_convert(char c, va_list *args)
{
	if (c == 'c')
		return (ft_print_char(va_arg(*args, int)));
	if (c == 's')
		return (ft_print_str(va_arg(*args, const char *)));
	if (c == 'p')
		return (ft_print_ptr(va_arg(*args, void *)));
	if (c == 'd' || c == 'i')
		return (ft_print_int(va_arg(*args, int)));
	if (c == 'u')
		return (ft_print_uint(va_arg(*args, unsigned int)));
	if (c == 'x')
		return (ft_print_hex(va_arg(*args, unsigned int), 0));
	if (c == 'X')
		return (ft_print_hex(va_arg(*args, unsigned int), 1));
	if (c == '%')
		return (ft_print_percent());
	return (-2);
}

static int	ft_emit_unknown(char c)
{
	if (ft_write_char('%') < 0)
		return (-1);
	if (ft_write_char(c) < 0)
		return (-1);
	return (2);
}

static int	ft_percent(const char *fmt, int *i, va_list *args)
{
	int	r;

	(*i)++;
	if (fmt[*i] == '\0')
		return (-1);
	r = ft_convert(fmt[*i], args);
	if (r == -2)
		r = ft_emit_unknown(fmt[*i]);
	(*i)++;
	return (r);
}

static int	ft_process(const char *fmt, va_list *args)
{
	int	i;
	int	count;
	int	r;

	i = 0;
	count = 0;
	while (fmt[i])
	{
		if (fmt[i] == '%')
			r = ft_percent(fmt, &i, args);
		else
		{
			r = ft_write_char(fmt[i]);
			i++;
		}
		if (r < 0)
			return (-1);
		count += r;
	}
	return (count);
}

int	ft_printf(const char *fmt, ...)
{
	va_list	args;
	int		r;

	if (fmt == NULL)
		return (-1);
	va_start(args, fmt);
	r = ft_process(fmt, &args);
	va_end(args);
	return (r);
}
