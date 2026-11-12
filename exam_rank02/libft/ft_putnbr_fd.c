/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_putnbr_fd.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: student42 <student42@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/04 08:00:00 by student42         #+#    #+#             */
/*   Updated: 2026/11/04 08:00:00 by student42        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

static void	ft_putuint_fd(unsigned int u, int fd)
{
	char	c;
	ssize_t	ret;

	if (u >= 10)
		ft_putuint_fd(u / 10, fd);
	c = (char)('0' + (u % 10));
	ret = write(fd, &c, 1);
	(void)ret;
}

void	ft_putnbr_fd(int n, int fd)
{
	unsigned int	u;
	ssize_t			ret;

	if (n < 0)
	{
		ret = write(fd, "-", 1);
		(void)ret;
		u = (unsigned int)(0U - (unsigned int)n);
	}
	else
		u = (unsigned int)n;
	ft_putuint_fd(u, fd);
}
