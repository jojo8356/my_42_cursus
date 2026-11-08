/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_list_at.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/08 19:00:00 by jojo8356          #+#    #+#             */
/*   Updated: 2026/11/08 19:00:00 by jojo8356         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

/* Renvoie l'element a l'indice idx (0 = premier), ou 0. */
#include "ft_list.h"

t_list	*ft_list_at(t_list *begin_list, unsigned int idx)
{
	unsigned int	i;

	i = 0;
	while (begin_list)
	{
		if (i == idx)
			return (begin_list);
		begin_list = begin_list->next;
		i++;
	}
	return (0);
}
