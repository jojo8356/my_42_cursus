/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   convert_group.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/08 19:00:00 by jojo8356          #+#    #+#             */
/*   Updated: 2026/11/08 19:00:00 by jojo8356         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "rush02.h"

char	*dict_find(t_dict *d, const char *key)
{
	int	i;

	i = 0;
	while (i < d->count)
	{
		if (ft_strcmp(d->key[i], key) == 0)
			return (d->val[i]);
		i++;
	}
	return (0);
}

void	buf_add(t_buf *b, const char *s)
{
	int	i;

	i = 0;
	while (s[i])
	{
		b->out[b->pos] = s[i];
		b->pos++;
		i++;
	}
	b->out[b->pos] = '\0';
}

static void	emit_hyphen_word(t_buf *b, char *z)
{
	int	i;
	int	sp;

	sp = -1;
	i = -1;
	while (z[++i])
	{
		if (z[i] == ' ' && sp >= 0)
			sp = -2;
		else if (z[i] == ' ')
			sp = i;
		else if (z[i] < 'a' || z[i] > 'z')
			sp = -2;
	}
	if (sp > 0)
	{
		z[sp] = '-';
		buf_add(b, z);
		z[sp] = ' ';
		return ;
	}
	buf_add(b, z);
}

int	emit_word(t_buf *b, int m)
{
	char	key[4];
	char	*z;

	key[0] = '0' + m;
	key[1] = '\0';
	if (m >= 10 && (key[0] = '0' + (m / 10)))
		key[1] = '0' + (m % 10);
	key[2] = '\0';
	z = dict_find(b->d, key);
	if (!z && m > 20 && (m % 10) != 0)
	{
		if (!emit_word(b, m - (m % 10)))
			return (0);
		if (b->w && !b->d->lang)
			buf_add(b, "-");
		else
			buf_add(b, " ");
		return (emit_word(b, m % 10));
	}
	if (!z)
		return (0);
	if (b->w && !b->d->lang)
		return (emit_hyphen_word(b, z), 1);
	return (buf_add(b, z), 1);
}

int	emit_chunk3(t_buf *b, int val)
{
	int		h[2];
	char	*z;

	h[0] = val / 100;
	h[1] = val % 100;
	if (h[0] > 0 && !(b->d->lang && h[0] == 1) && !emit_word(b, h[0]))
		return (0);
	if (h[0] > 0 && !(b->d->lang && h[0] == 1))
		buf_add(b, " ");
	if (h[0] > 0)
	{
		z = dict_find(b->d, "100");
		if (!z)
			return (0);
		buf_add(b, z);
		if (b->d->lang && h[0] > 1 && h[1] == 0)
			buf_add(b, "s");
		if (h[1] > 0 && b->w && !b->d->lang)
			buf_add(b, " and ");
		else if (h[1] > 0)
			buf_add(b, " ");
	}
	if (h[1] > 0 || h[0] == 0)
		return (emit_word(b, h[1]));
	return (1);
}
