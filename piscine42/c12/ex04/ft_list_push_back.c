/* Ajoute un nouvel element a la fin de la liste. */
#include "ft_list.h"

void	ft_list_push_back(t_list **begin_list, void *data)
{
	t_list	*elem;
	t_list	*cur;

	elem = ft_create_elem(data);
	if (!elem)
		return ;
	if (!*begin_list)
	{
		*begin_list = elem;
		return ;
	}
	cur = *begin_list;
	while (cur->next)
		cur = cur->next;
	cur->next = elem;
}
