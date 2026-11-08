/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_sorted_list_merge.c                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/08 19:00:00 by jojo8356          #+#    #+#             */
/*   Updated: 2026/11/08 19:00:00 by jojo8356         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

/* Fusionne begin_list2 (triee) dans begin_list1 (triee) en gardant
   l'ordre croissant global. */
#include "ft_list.h"

static void	sorted_insert(t_list **begin, t_list *node, int (*cmp)())
{
	t_list	*cur;

	if (!*begin || cmp((*begin)->data, node->data) > 0)
	{
		node->next = *begin;
		*begin = node;
		return ;
	}
	cur = *begin;
	while (cur->next && cmp(cur->next->data, node->data) < 0)
		cur = cur->next;
	node->next = cur->next;
	cur->next = node;
}

void	ft_sorted_list_merge(t_list **begin_list1, t_list *begin_list2,
		int (*cmp)())
{
	t_list	*next;

	while (begin_list2)
	{
		next = begin_list2->next;
		sorted_insert(begin_list1, begin_list2, cmp);
		begin_list2 = next;
	}
}
