/* Inverse l'ordre des elements de la liste en echangeant les data
   (comme un tableau : premier <-> dernier, etc.). */
#include "ft_list.h"

static t_list	*node_at(t_list *begin, int idx)
{
	while (idx > 0)
	{
		begin = begin->next;
		idx--;
	}
	return (begin);
}

void	ft_list_reverse_fun(t_list *begin_list)
{
	t_list	*a;
	t_list	*b;
	void	*tmp;
	int		len;
	int		i;

	len = 0;
	a = begin_list;
	while (a && ++len)
		a = a->next;
	i = 0;
	while (i < len / 2)
	{
		a = node_at(begin_list, i);
		b = node_at(begin_list, len - 1 - i);
		tmp = a->data;
		a->data = b->data;
		b->data = tmp;
		i++;
	}
}
