/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/08 19:00:00 by jojo8356          #+#    #+#             */
/*   Updated: 2026/11/08 19:00:00 by jojo8356         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

/* Binaire unique : ./rush0x <version 0-4> [x y]  (defaut 5 5). */
#include <unistd.h>

void	rush(int x, int y, int version);

static int	ft_atoi(const char *s)
{
	int	n;

	n = 0;
	while (*s >= '0' && *s <= '9')
	{
		n = n * 10 + (*s - '0');
		s++;
	}
	if (*s != '\0')
		return (-1);
	return (n);
}

int	main(int argc, char **argv)
{
	int	version;
	int	x;
	int	y;

	if (argc != 2 && argc != 4)
	{
		write(2, "usage: rush0x <version 0-4> [x y]\n", 34);
		return (1);
	}
	version = ft_atoi(argv[1]);
	x = 5;
	y = 5;
	if (argc == 4)
	{
		x = ft_atoi(argv[2]);
		y = ft_atoi(argv[3]);
	}
	if (version < 0 || version > 4 || x < 0 || y < 0)
	{
		write(2, "Error\n", 6);
		return (1);
	}
	rush(x, y, version);
	return (0);
}
