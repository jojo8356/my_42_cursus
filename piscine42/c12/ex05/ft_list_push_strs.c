/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_list_push_strs.c                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/08 19:00:00 by jojo8356          #+#    #+#             */
/*   Updated: 2026/11/08 19:00:00 by jojo8356         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

/* Cree une liste depuis les size chaines de strs : chaque chaine est
   ajoutee au debut, donc strs[0] se retrouve en FIN de liste.
   Renvoie le premier element de la liste creee. */
#include "ft_list.h"

static void	push_front(t_list **begin_list, void *data)
{
	t_list	*elem;

	elem = ft_create_elem(data);
	if (!elem)
		return ;
	elem->next = *begin_list;
	*begin_list = elem;
}

t_list	*ft_list_push_strs(int size, char **strs)
{
	t_list	*begin;
	int		i;

	begin = 0;
	i = 0;
	while (i < size)
	{
		push_front(&begin, strs[i]);
		i++;
	}
	return (begin);
}
