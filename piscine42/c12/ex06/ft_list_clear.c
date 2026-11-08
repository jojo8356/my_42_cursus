/* Vide et libere toute la liste : les data via free_fct, les maillons
   via free. */
#include <stdlib.h>
#include "ft_list.h"

void	ft_list_clear(t_list *begin_list, void (*free_fct)(void *))
{
	t_list	*next;

	while (begin_list)
	{
		next = begin_list->next;
		if (free_fct)
			free_fct(begin_list->data);
		free(begin_list);
		begin_list = next;
	}
}
