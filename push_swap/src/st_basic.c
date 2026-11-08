/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   st_basic.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/08 21:36:54 by jojo8356          #+#    #+#             */
/*   Updated: 2026/11/08 21:36:54 by jojo8356         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "push_swap.h"

int	st_init(t_stack *s, int capacity)
{
	s->data = NULL;
	s->size = 0;
	s->capacity = 0;
	if (capacity <= 0)
		capacity = 1;
	s->data = (int *)malloc(sizeof(int) * (size_t)capacity);
	if (!s->data)
		return (0);
	s->capacity = capacity;
	return (1);
}

void	st_free(t_stack *s)
{
	if (!s)
		return ;
	free(s->data);
	s->data = NULL;
	s->size = 0;
	s->capacity = 0;
}

void	st_copy(t_stack *dst, const t_stack *src)
{
	int	i;

	i = 0;
	while (i < src->size)
	{
		dst->data[i] = src->data[i];
		i++;
	}
	dst->size = src->size;
}

int	value_at(const t_stack *s, int i)
{
	return (s->data[i]);
}
