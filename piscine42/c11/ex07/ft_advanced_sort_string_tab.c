/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_advanced_sort_string_tab.c                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/08 19:00:00 by jojo8356          #+#    #+#             */
/*   Updated: 2026/11/08 19:00:00 by jojo8356         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

/* Trie le tableau de chaines (termine par NULL) selon la fonction de
   comparaison cmp (echanges de pointeurs). */
void	ft_advanced_sort_string_tab(char **tab, int (*cmp)(char *, char *))
{
	int		j;
	int		swapped;
	char	*tmp;

	swapped = 1;
	while (swapped)
	{
		swapped = 0;
		j = 0;
		while (tab[j] && tab[j + 1])
		{
			if (cmp(tab[j], tab[j + 1]) > 0)
			{
				tmp = tab[j];
				tab[j] = tab[j + 1];
				tab[j + 1] = tmp;
				swapped = 1;
			}
			j++;
		}
	}
}
