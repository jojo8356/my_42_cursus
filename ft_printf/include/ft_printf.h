/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_printf.h                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: arena <arena@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/04 08:00:00 by arena             #+#    #+#             */
/*   Updated: 2026/11/04 08:00:00 by arena            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef FT_PRINTF_H
# define FT_PRINTF_H

# include <stdarg.h>
# include <unistd.h>

int	ft_printf(const char *format, ...);

int	ft_write_char(char c);
int	ft_write_mem(const char *s, int len);
int	ft_write_pad(char c, int n);
int	ft_strlen(const char *s);
int	ft_numlen(unsigned long long n, int base);
int	ft_utoa(unsigned long long n, char *buf, int base, int upper);

int	ft_print_char(int c);
int	ft_print_str(const char *s);
int	ft_print_percent(void);
int	ft_print_int(int n);
int	ft_print_uint(unsigned int n);
int	ft_print_hex(unsigned int n, int upper);
int	ft_print_ptr(void *p);

#endif
