/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_parse_bonus.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: arena <arena@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/04 08:00:00 by arena             #+#    #+#             */
/*   Updated: 2026/11/04 08:00:00 by arena            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_printf_bonus.h"

static void	ft_init_spec(t_spec *s)
{
	s->minus = 0;
	s->zero = 0;
	s->hash = 0;
	s->plus = 0;
	s->space = 0;
	s->width = 0;
	s->precision = -1;
}

static void	ft_parse_flags(const char *fmt, int *i, t_spec *s)
{
	while (fmt[*i] == '-' || fmt[*i] == '0' || fmt[*i] == '#'
		|| fmt[*i] == '+' || fmt[*i] == ' ')
	{
		if (fmt[*i] == '-')
			s->minus = 1;
		else if (fmt[*i] == '0')
			s->zero = 1;
		else if (fmt[*i] == '#')
			s->hash = 1;
		else if (fmt[*i] == '+')
			s->plus = 1;
		else
			s->space = 1;
		(*i)++;
	}
}

static void	ft_parse_width(const char *fmt, int *i, va_list *args, t_spec *s)
{
	int	w;

	if (fmt[*i] == '*')
	{
		w = va_arg(*args, int);
		if (w < 0)
		{
			s->minus = 1;
			w = -w;
		}
		s->width = w;
		(*i)++;
		return ;
	}
	while (fmt[*i] >= '0' && fmt[*i] <= '9')
	{
		s->width = s->width * 10 + (fmt[*i] - '0');
		(*i)++;
	}
}

static void	ft_parse_prec(const char *fmt, int *i, va_list *ap, t_spec *s)
{
	int	p;

	if (fmt[*i] != '.')
		return ;
	(*i)++;
	if (fmt[*i] == '*')
	{
		p = va_arg(*ap, int);
		if (p < 0)
			p = -1;
		s->precision = p;
		(*i)++;
		return ;
	}
	s->precision = 0;
	while (fmt[*i] >= '0' && fmt[*i] <= '9')
	{
		s->precision = s->precision * 10 + (fmt[*i] - '0');
		(*i)++;
	}
}

int	ft_parse_spec(const char *fmt, int *i, va_list *args, t_spec *s)
{
	s->start = *i;
	(*i)++;
	ft_init_spec(s);
	ft_parse_flags(fmt, i, s);
	ft_parse_width(fmt, i, args, s);
	ft_parse_prec(fmt, i, args, s);
	s->conv = fmt[*i];
	if (s->conv == '\0')
		return (-1);
	(*i)++;
	s->end = *i;
	return (0);
}
