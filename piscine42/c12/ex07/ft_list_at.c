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
