/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_find_next_prime.c                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/08 19:00:00 by jojo8356          #+#    #+#             */
/*   Updated: 2026/11/08 19:00:00 by jojo8356         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

/* Renvoie le premier nombre premier >= nb. */
static int	is_prime_local(long n)
{
	long	i;

	if (n < 2)
		return (0);
	i = 2;
	while (i * i <= n)
	{
		if (n % i == 0)
			return (0);
		i++;
	}
	return (1);
}

int	ft_find_next_prime(int nb)
{
	long	n;

	if (nb <= 2)
		return (2);
	n = nb;
	while (1)
	{
		if (is_prime_local(n))
			return ((int)n);
		n++;
	}
}
