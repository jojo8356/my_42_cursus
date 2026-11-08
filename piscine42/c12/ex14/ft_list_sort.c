/* Trie la liste par ordre croissant selon cmp (bulles sur les data). */
#include "ft_list.h"

void	ft_list_sort(t_list **begin_list, int (*cmp)())
{
	t_list	*cur;
	void	*tmp;
	int		swapped;

	swapped = 1;
	while (swapped)
	{
		swapped = 0;
		cur = *begin_list;
		while (cur && cur->next)
		{
			if (cmp(cur->data, cur->next->data) > 0)
			{
				tmp = cur->data;
				cur->data = cur->next->data;
				cur->next->data = tmp;
				swapped = 1;
			}
			cur = cur->next;
		}
	}
}
