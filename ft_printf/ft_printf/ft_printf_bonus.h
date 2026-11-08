/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_printf_bonus.h                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: arena <arena@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/04 08:00:00 by arena             #+#    #+#             */
/*   Updated: 2026/11/04 08:00:00 by arena            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef FT_PRINTF_BONUS_H
# define FT_PRINTF_BONUS_H

# include "ft_printf.h"

typedef struct s_spec
{
	int		minus;
	int		zero;
	int		hash;
	int		plus;
	int		space;
	int		width;
	int		precision;
	int		start;
	int		end;
	char	conv;
}	t_spec;

typedef struct s_num
{
	const char	*prefix;
	int			plen;
	char		*digits;
	int			dlen;
}	t_num;

int	ft_parse_spec(const char *fmt, int *i, va_list *args, t_spec *s);
int	ft_render_spec(const char *fmt, t_spec *s, va_list *args);
int	ft_render_char(t_spec *s, va_list *args);
int	ft_render_str(t_spec *s, va_list *args);
int	ft_render_percent(void);
int	ft_write_string(t_spec *s, const char *str, int len);
int	ft_render_int(t_spec *s, va_list *args);
int	ft_render_uint(t_spec *s, va_list *args);
int	ft_render_hex(t_spec *s, va_list *args);
int	ft_render_ptr(t_spec *s, va_list *args);
int	ft_write_number(t_spec *s, t_num *n);

#endif
