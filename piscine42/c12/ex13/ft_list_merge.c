/* Concatene begin_list2 a la fin de la liste begin_list1. */
#include "ft_list.h"

void	ft_list_merge(t_list **begin_list1, t_list *begin_list2)
{
	t_list	*cur;

	if (!*begin_list1)
	{
		*begin_list1 = begin_list2;
		return ;
	}
	cur = *begin_list1;
	while (cur->next)
		cur = cur->next;
	cur->next = begin_list2;
}
