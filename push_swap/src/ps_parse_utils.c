/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ps_parse_utils.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/08 21:36:54 by jojo8356          #+#    #+#             */
/*   Updated: 2026/11/08 21:36:54 by jojo8356         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "push_swap.h"

int	ps_is_blank(char c)
{
	return (c == ' ' || (c >= 9 && c <= 13));
}

int	ps_fill(t_stack *a, char *s, int *k)
{
	char	*start;
	char	tmp[12];
	int		n;

	while (*s)
	{
		while (ps_is_blank(*s))
			s++;
		if (!*s)
			break ;
		start = s;
		while (*s && !ps_is_blank(*s))
			s++;
		n = (int)(s - start);
		if (n == 0 || n > 11)
			return (0);
		ft_memcpy(tmp, start, n);
		tmp[n] = '\0';
		if (!ps_str_to_int(tmp, &a->data[*k]))
			return (0);
		(*k)++;
	}
	return (1);
}

int	ps_has_duplicate(const t_stack *a)
{
	int	i;
	int	j;

	i = 0;
	while (i < a->size)
	{
		j = i + 1;
		while (j < a->size)
		{
			if (a->data[i] == a->data[j])
				return (1);
			j++;
		}
		i++;
	}
	return (0);
}

int	ps_strlen(const char *s)
{
	int	n;

	n = 0;
	while (s[n])
		n++;
	return (n);
}
