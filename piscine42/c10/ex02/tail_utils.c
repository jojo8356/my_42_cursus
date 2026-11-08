/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   tail_utils.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/08 19:00:00 by jojo8356          #+#    #+#             */
/*   Updated: 2026/11/08 19:00:00 by jojo8356         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

/* Outils de ft_tail : message d'erreur et lecture des n derniers
   octets d'un flux (tampon circulaire malloc). */
#include <unistd.h>
#include <stdlib.h>
#include "ft_tail.h"

void	put_error(char *prog, char *file, char *reason)
{
	while (*prog)
		write(2, prog++, 1);
	write(2, ": ", 2);
	while (*file)
		write(2, file++, 1);
	write(2, ": ", 2);
	while (*reason)
		write(2, reason++, 1);
	write(2, "\n", 1);
}

static void	flush_ring(char *ring, long n, long total)
{
	long	start;
	long	i;

	start = 0;
	if (total > n)
	{
		start = total % n;
		total = n;
	}
	i = 0;
	while (i < total)
	{
		write(1, &ring[(start + i) % n], 1);
		i++;
	}
}

void	tail_fd(int fd, long n)
{
	char	*ring;
	char	buf[29696];
	long	total;
	long	i;
	int		r;

	if (n <= 0)
		return ;
	ring = malloc(n);
	if (!ring)
		return ;
	total = 0;
	r = read(fd, buf, sizeof(buf));
	while (r > 0)
	{
		i = -1;
		while (++i < r)
			ring[(total + i) % n] = buf[i];
		total += r;
		r = read(fd, buf, sizeof(buf));
	}
	flush_ring(ring, n, total);
	free(ring);
}
