/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/08 21:36:54 by jojo8356          #+#    #+#             */
/*   Updated: 2026/11/08 21:36:54 by jojo8356         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "push_swap.h"

int	main(int ac, char **av)
{
	t_stack	a;
	t_stack	b;
	t_ops	o;
	int		res;

	if (ac < 2)
		return (0);
	res = ps_parse(ac, av, &a);
	if (res <= 0)
		return (ps_error(), 1);
	if (!st_init(&b, a.size) || !op_init(&o, 1 << 16))
		return (st_free(&a), 1);
	sort_adaptive(&a, &b, &o);
	op_flush(&o);
	op_free(&o);
	st_free(&a);
	st_free(&b);
	return (0);
}
