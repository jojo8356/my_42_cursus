/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   get_next_line_utils_bonus.c                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/08 19:30:00 by jojo8356          #+#    #+#             */
/*   Updated: 2026/11/08 19:30:00 by jojo8356         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "get_next_line_bonus.h"

void	gnl_memcpy(char *dst, const char *src, size_t n)
{
	size_t	i;

	i = 0;
	while (i < n)
	{
		dst[i] = src[i];
		i++;
	}
}

int	gnl_find_nl(t_fd *node, size_t *nl_idx)
{
	size_t	i;

	if (!node->buf)
		return (0);
	i = node->scan;
	while (i < node->len)
	{
		if (node->buf[i] == '\n')
		{
			*nl_idx = i;
			node->scan = i;
			return (1);
		}
		i++;
	}
	node->scan = node->len;
	return (0);
}

int	gnl_reserve(t_fd *node, size_t add)
{
	char	*nbuf;
	size_t	need;
	size_t	ncap;

	if (add > (size_t)-1 - node->len - 1)
		return (0);
	need = node->len + add + 1;
	if (need <= node->cap)
		return (1);
	ncap = 64;
	if (node->cap > 0 && node->cap <= ((size_t)-1) / 2)
		ncap = node->cap * 2;
	if (ncap < need)
		ncap = need;
	nbuf = malloc(ncap);
	if (!nbuf)
		return (0);
	if (node->buf && node->len > 0)
		gnl_memcpy(nbuf, node->buf, node->len);
	if (node->buf)
		free(node->buf);
	node->buf = nbuf;
	node->cap = ncap;
	return (1);
}

void	gnl_del_node(t_fd **head, int fd)
{
	t_fd	*cur;
	t_fd	*prev;

	prev = NULL;
	cur = *head;
	while (cur)
	{
		if (cur->fd == fd)
		{
			if (prev)
				prev->next = cur->next;
			else
				*head = cur->next;
			if (cur->buf)
				free(cur->buf);
			free(cur);
			return ;
		}
		prev = cur;
		cur = cur->next;
	}
}
