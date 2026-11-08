/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/08 19:00:00 by jojo8356          #+#    #+#             */
/*   Updated: 2026/11/08 19:00:00 by jojo8356         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <unistd.h>
#include "rush02.h"

static void	handle_one(t_dict *d, char *num, int w)
{
	char	out[8192];
	int		res;

	res = convert_number(d, num, w, out);
	if (res < 0)
		ft_putstr("Error\n");
	else if (res == 0)
		ft_putstr("Dict Error\n");
	else
	{
		ft_putstr(out);
		ft_putstr("\n");
	}
}

static void	feed_char(t_dict *d, char c, char *line, int *st)
{
	if (c == '\n')
	{
		line[st[0]] = '\0';
		handle_one(d, line, st[1]);
		st[0] = 0;
	}
	else if (st[0] < 510)
	{
		line[st[0]] = c;
		st[0]++;
	}
}

static int	read_stdin(t_dict *d, int w)
{
	char	buf[4096];
	char	line[512];
	int		st[2];
	int		r;
	int		i;

	st[0] = 0;
	st[1] = w;
	i = -1;
	r = read(0, buf, 4096);
	if (r <= 0)
		return (0);
	while (r > 0)
	{
		i = -1;
		while (++i < r)
			feed_char(d, buf[i], line, st);
		r = read(0, buf, 4096);
	}
	if (st[0] > 0)
		feed_char(d, '\n', line, st);
	return (1);
}

static int	parse_args(int argc, char **argv, char **dict, char **num)
{
	int	i;
	int	w;
	int	pos;

	*dict = "numbers.dict";
	*num = 0;
	w = 0;
	pos = 0;
	i = 0;
	while (++i < argc)
	{
		if (ft_strcmp(argv[i], "-w") == 0)
			w = 1;
		else
		{
			if (pos == 1)
				*dict = *num;
			*num = argv[i];
			pos++;
		}
	}
	if (pos > 2)
		return (-1);
	return (w);
}

int	main(int argc, char **argv)
{
	t_dict	d;
	char	*dict;
	char	*num;
	int		w;

	w = parse_args(argc, argv, &dict, &num);
	if (w < 0)
		return (ft_putstr("Error\n"), 1);
	if (!dict_load(dict, &d))
		return (ft_putstr("Dict Error\n"), 1);
	if (!num || ft_strcmp(num, "-") == 0)
	{
		if (!read_stdin(&d, w) && !num)
			return (dict_free(&d), ft_putstr("Error\n"), 1);
	}
	else
		handle_one(&d, num, w);
	dict_free(&d);
	return (0);
}
