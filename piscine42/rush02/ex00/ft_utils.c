/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_utils.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/08 19:00:00 by jojo8356          #+#    #+#             */
/*   Updated: 2026/11/08 19:00:00 by jojo8356         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdlib.h>
#include <unistd.h>
#include "rush02.h"

int	ft_strlen(const char *s)
{
	int	i;

	i = 0;
	while (s[i])
		i++;
	return (i);
}

void	ft_putstr(const char *s)
{
	write(1, s, ft_strlen(s));
}

int	ft_strcmp(const char *a, const char *b)
{
	int	i;

	i = 0;
	while (a[i] && a[i] == b[i])
		i++;
	return ((unsigned char)a[i] - (unsigned char)b[i]);
}

char	*ft_strdup_n(const char *s, int len)
{
	char	*dup;
	int		i;

	dup = malloc(len + 1);
	if (!dup)
		return (0);
	i = 0;
	while (i < len)
	{
		dup[i] = s[i];
		i++;
	}
	dup[len] = '\0';
	return (dup);
}

char	*norm_digits(const char *s, int len)
{
	int	i;
	int	start;

	if (len <= 0)
		return (0);
	i = 0;
	while (i < len)
	{
		if (s[i] < '0' || s[i] > '9')
			return (0);
		i++;
	}
	start = 0;
	while (start < len - 1 && s[start] == '0')
		start++;
	return (ft_strdup_n(s + start, len - start));
}
