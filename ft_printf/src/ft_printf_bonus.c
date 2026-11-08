/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_printf_bonus.c                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: arena <arena@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/04 08:00:00 by arena             #+#    #+#             */
/*   Updated: 2026/11/04 08:00:00 by arena            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_printf_bonus.h"

static int	ft_process(const char *fmt, va_list *args)
{
	t_spec	s;
	int		i;
	int		count;
	int		r;

	i = 0;
	count = 0;
	while (fmt[i])
	{
		if (fmt[i] != '%')
		{
			if (ft_write_char(fmt[i++]) < 0)
				return (-1);
			count++;
			continue ;
		}
		if (ft_parse_spec(fmt, &i, args, &s) < 0)
			return (-1);
		r = ft_render_spec(fmt, &s, args);
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
