/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   op_buffer.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/08 21:36:54 by jojo8356          #+#    #+#             */
/*   Updated: 2026/11/08 21:36:54 by jojo8356         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "push_swap.h"

/*
** Writing thousands of instructions one `write` at a time is the slowest part
** of push_swap: 5500 syscalls at 500 values. The buffer below turns that into
** a handful of large writes, and it is the only output path of the program.
*/

int	op_emit(t_ops *o, const char *name)
{
	int		n;

	if (!o)
		return (1);
	n = 0;
	while (name[n])
		n++;
	n++;
	if (o->len + n > o->cap)
		if (!op_flush(o))
			return (0);
	ft_memcpy(o->buf + o->len, name, (size_t)n);
	o->buf[o->len + n - 1] = '\n';
	o->len += n;
	o->count++;
	return (1);
}

int	op_flush(t_ops *o)
{
	int	written;

	if (o->len == 0)
		return (1);
	written = (int)write(1, o->buf, (size_t)o->len);
	if (written != o->len)
		return (0);
	o->len = 0;
	return (1);
}

int	op_init(t_ops *o, int cap)
{
	o->buf = (char *)malloc((size_t)cap);
	if (!o->buf)
		return (0);
	o->len = 0;
	o->cap = cap;
	o->count = 0;
	return (1);
}

void	op_free(t_ops *o)
{
	free(o->buf);
	o->buf = NULL;
	o->len = 0;
	o->cap = 0;
}
