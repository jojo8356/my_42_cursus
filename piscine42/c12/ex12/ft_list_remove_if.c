/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_list_remove_if.c                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/08 19:00:00 by jojo8356          #+#    #+#             */
/*   Updated: 2026/11/08 19:00:00 by jojo8356         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

/* Supprime de la liste tous les elements dont data compare a 0 avec
   data_ref ; libere les data via free_fct et les maillons via free. */
#include <stdlib.h>
#include "ft_list.h"

void	ft_list_remove_if(t_list **begin_list, void *data_ref,
		int (*cmp)(), void (*free_fct)(void *))
{
	t_list	*cur;
	t_list	*prev;
	t_list	*next;

	cur = *begin_list;
	prev = 0;
	while (cur)
	{
		next = cur->next;
		if (cmp(cur->data, data_ref) == 0)
		{
			if (prev)
				prev->next = next;
			else
				*begin_list = next;
			if (free_fct)
				free_fct(cur->data);
			free(cur);
		}
		else
			prev = cur;
		cur = next;
	}
}
