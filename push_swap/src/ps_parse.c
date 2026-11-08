/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ps_parse.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/08 21:36:54 by jojo8356          #+#    #+#             */
/*   Updated: 2026/11/08 21:36:54 by jojo8356         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "push_swap.h"

/*
** A push_swap argument may be "42", "-42", "+42", or a whole string of them
** ("42 -7 9") when the shell did not split it. Anything else is an error.
*/

static int	count_numbers(int ac, char **av)
{
	int		i;
	int		total;
	char	*c;

	total = 0;
	i = 1;
	while (i < ac)
	{
		c = av[i];
		while (*c)
		{
			while (ps_is_blank(*c))
				c++;
			if (!*c)
				break ;
			total++;
			while (*c && !ps_is_blank(*c))
				c++;
		}
		i++;
	}
	return (total);
}

/*
** Integer parsing with an explicit range check: strtol is on the allowed list
** but its errno dance is longer than this, and the overflow boundary is exactly
** what the subject tests with INT_MIN and INT_MAX.
*/

int	ps_str_to_int(const char *s, int *out)
{
	long	value;
	int		sign;

	value = 0;
	sign = 1;
	if (*s == '-' || *s == '+')
		if (*s++ == '-')
			sign = -1;
	if (!*s)
		return (0);
	while (*s >= '0' && *s <= '9')
	{
		value = value * 10 + (*s++ - '0');
		if (value > 2147483648L)
			return (0);
	}
	if (*s)
		return (0);
	value *= sign;
	if (value < -2147483648L || value > 2147483647L)
		return (0);
	*out = (int)value;
	return (1);
}

/*
** An empty argument is an error, not an absence of argument.
** `./checker "" 1` must print Error: the subject lists it explicitly, and the
** two cases cannot be told apart any other way, since ac == 2 either way.
**
** A whole argument made only of blanks ("   ") is the same mistake and is
** refused by the same rule: ps_fill adds nothing, so k does not move.
*/

int	ps_parse(int ac, char **av, t_stack *a)
{
	int	i;
	int	k;
	int	before;

	if (ac < 2)
		return (0);
	if (!st_init(a, count_numbers(ac, av)))
		return (-1);
	k = 0;
	i = 1;
	while (i < ac)
	{
		before = k;
		if (!ps_fill(a, av[i], &k))
			return (st_free(a), 0);
		if (k == before)
			return (st_free(a), 0);
		i++;
	}
	a->size = k;
	if (ps_has_duplicate(a))
		return (st_free(a), 0);
	return (1);
}

void	ps_error(void)
{
	write(2, "Error\n", 6);
}
