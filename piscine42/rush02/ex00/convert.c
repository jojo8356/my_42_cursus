/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   convert.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/08 19:00:00 by jojo8356          #+#    #+#             */
/*   Updated: 2026/11/08 19:00:00 by jojo8356         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdlib.h>
#include "rush02.h"

static char	*make_scale_key(int zeros)
{
	char	*key;
	int		i;

	key = malloc(zeros + 2);
	if (!key)
		return (0);
	key[0] = '1';
	i = 1;
	while (i <= zeros)
	{
		key[i] = '0';
		i++;
	}
	key[zeros + 1] = '\0';
	return (key);
}

static int	slice_int(const char *s, int len)
{
	int	val;
	int	i;

	val = 0;
	i = 0;
	while (i < len)
	{
		val = val * 10 + (s[i] - '0');
		i++;
	}
	return (val);
}

static int	emit_scale_group(t_buf *b, int val, int zeros)
{
	char	*skey;
	char	*z;

	if (!b->first && b->w && !b->d->lang)
		buf_add(b, ", ");
	else if (!b->first)
		buf_add(b, " ");
	b->first = 0;
	if (zeros == 0)
		return (emit_chunk3(b, val));
	skey = make_scale_key(zeros);
	if (!skey)
		return (0);
	z = dict_find(b->d, skey);
	free(skey);
	if (!z)
		return (0);
	if (!(b->d->lang && zeros == 3 && val == 1)
		&& (!emit_chunk3(b, val) || (buf_add(b, " "), 0)))
		return (0);
	buf_add(b, z);
	if (b->d->lang && zeros >= 6 && val > 1)
		buf_add(b, "s");
	return (1);
}

static int	convert_groups(t_buf *b, const char *norm)
{
	int	len;
	int	idx;
	int	glen;
	int	val;

	len = ft_strlen(norm);
	idx = 0;
	while (idx < len)
	{
		glen = (len - idx) % 3;
		if (glen == 0)
			glen = 3;
		val = slice_int(norm + idx, glen);
		if (val > 0 && !emit_scale_group(b, val, len - idx - glen))
			return (0);
		idx += glen;
	}
	return (1);
}

int	convert_number(t_dict *d, const char *raw, int w, char *out)
{
	char	*norm;
	t_buf	b;
	int		ok;

	norm = norm_digits(raw, ft_strlen(raw));
	if (!norm)
		return (-1);
	b.d = d;
	b.out = out;
	b.pos = 0;
	b.w = w;
	b.first = 1;
	out[0] = '\0';
	if (ft_strcmp(norm, "0") == 0)
		ok = emit_word(&b, 0);
	else
		ok = convert_groups(&b, norm);
	free(norm);
	return (ok);
}
