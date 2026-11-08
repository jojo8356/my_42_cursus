/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   checker_main.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/08 21:36:54 by jojo8356          #+#    #+#             */
/*   Updated: 2026/11/08 21:36:54 by jojo8356         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "checker.h"

/*
** The subject's checker: read instructions on stdin, apply them, then print OK
** when a is sorted and b is empty, KO otherwise. An instruction that does not
** exist, or is not terminated by a newline, is an error.
*/

static int	run(t_stack *a, t_stack *b)
{
	t_reader	r;
	char		line[16];
	int			n;

	r.pos = 0;
	r.len = 0;
	r.seen = 0;
	while (1)
	{
		n = ch_next_line(&r, line, (int)sizeof(line));
		if (n == 0)
			return (1);
		if (n < 0)
			return (0);
		if (!exec_instr(a, b, line, NULL))
			return (0);
		r.seen = 1;
	}
	return (1);
}

int	main(int ac, char **av)
{
	t_stack	a;
	t_stack	b;
	int		res;

	if (ac < 2)
		return (0);
	res = ps_parse(ac, av, &a);
	if (res <= 0)
		return (ps_error(), 1);
	if (!st_init(&b, a.size))
		return (st_free(&a), 1);
	if (!run(&a, &b))
		return (ps_error(), st_free(&a), st_free(&b), 1);
	if (is_sorted(&a) && b.size == 0)
		write(1, "OK\n", 3);
	else
		write(1, "KO\n", 3);
	return (st_free(&a), st_free(&b), 0);
}
