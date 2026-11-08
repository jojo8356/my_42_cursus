/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   get_next_line_bonus.c                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/08 19:30:00 by jojo8356          #+#    #+#             */
/*   Updated: 2026/11/08 19:30:00 by jojo8356         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "get_next_line_bonus.h"

static t_fd	*gnl_get_node(t_fd **head, int fd)
{
	t_fd	*cur;

	cur = *head;
	while (cur)
	{
		if (cur->fd == fd)
			return (cur);
		cur = cur->next;
	}
	cur = malloc(sizeof(t_fd));
	if (!cur)
		return (NULL);
	cur->fd = fd;
	cur->buf = NULL;
	cur->len = 0;
	cur->cap = 0;
	cur->scan = 0;
	cur->next = *head;
	*head = cur;
	return (cur);
}

static int	gnl_fill(t_fd *node, size_t bs, size_t *llen)
{
	ssize_t	r;

	while (!gnl_find_nl(node, llen))
	{
		if (!gnl_reserve(node, bs))
			return (0);
		r = read(node->fd, node->buf + node->len, bs);
		if (r < 0)
			return (0);
		if (r == 0)
		{
			*llen = node->len;
			return (1);
		}
		node->len += (size_t)r;
		node->buf[node->len] = '\0';
	}
	*llen += 1;
	return (1);
}

static char	*gnl_extract(t_fd **head, t_fd *node, size_t llen)
{
	char	*line;
	size_t	rem;

	line = malloc(llen + 1);
	if (!line)
	{
		gnl_del_node(head, node->fd);
		return (NULL);
	}
	gnl_memcpy(line, node->buf, llen);
	line[llen] = '\0';
	rem = node->len - llen;
	if (rem == 0)
		gnl_del_node(head, node->fd);
	else
	{
		gnl_memcpy(node->buf, node->buf + llen, rem);
		node->len = rem;
		node->buf[rem] = '\0';
		node->scan = 0;
	}
	return (line);
}

char	*get_next_line(int fd)
{
	static t_fd	*head;
	t_fd		*node;
	ssize_t		bs;
	size_t		llen;

	bs = BUFFER_SIZE;
	if (fd < 0 || bs <= 0)
		return (NULL);
	llen = 0;
	node = gnl_get_node(&head, fd);
	if (!node)
		return (NULL);
	if (!gnl_fill(node, (size_t)bs, &llen) || node->len == 0)
	{
		gnl_del_node(&head, fd);
		return (NULL);
	}
	return (gnl_extract(&head, node, llen));
}
