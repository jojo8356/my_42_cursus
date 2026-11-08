/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   rush02.h                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/08 19:00:00 by jojo8356          #+#    #+#             */
/*   Updated: 2026/11/08 19:00:00 by jojo8356         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef RUSH02_H
# define RUSH02_H

# define MAX_ENTRIES 512

typedef struct s_dict
{
	char	*key[MAX_ENTRIES];
	char	*val[MAX_ENTRIES];
	int		count;
	int		lang;
}	t_dict;

typedef struct s_buf
{
	t_dict	*d;
	char	*out;
	int		pos;
	int		w;
	int		first;
}	t_buf;

int		ft_strlen(const char *s);
void	ft_putstr(const char *s);
int		ft_strcmp(const char *a, const char *b);
char	*ft_strdup_n(const char *s, int len);
char	*norm_digits(const char *s, int len);

int		dict_load(char *path, t_dict *d);
void	dict_free(t_dict *d);
char	*dict_find(t_dict *d, const char *key);

void	buf_add(t_buf *b, const char *s);
int		emit_word(t_buf *b, int m);
int		emit_chunk3(t_buf *b, int val);
int		convert_number(t_dict *d, const char *raw, int w, char *out);

#endif
