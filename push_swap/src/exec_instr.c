/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   exec_instr.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/08 21:36:54 by jojo8356          #+#    #+#             */
/*   Updated: 2026/11/08 21:36:54 by jojo8356         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "push_swap.h"

static int	same(const char *a, const char *b, int n)
{
	int	i;

	i = 0;
	while (i < n)
	{
		if (a[i] != b[i])
			return (0);
		i++;
	}
	return (a[n] == '\0');
}

/*
** One dispatch table shared by the mandatory binary, the algorithms and the
** bonus checker: an instruction name means exactly the same thing everywhere.
*/

int	exec_instr(t_stack *a, t_stack *b, const char *name, t_ops *o)
{
	int			ok;
	int			len;
	static void	(*const fns[12])(t_stack *, t_stack *, t_ops *) = {
		do_sa, do_sb, do_ss, do_pa, do_pb, do_ra,
		do_rb, do_rr, do_rra, do_rrb, do_rrr, NULL};
	static const char *const names[11] = {
		"sa", "sb", "ss", "pa", "pb", "ra", "rb", "rr", "rra", "rrb", "rrr"};

	ok = 0;
	while (ok < 11)
	{
		len = ps_strlen((char *)names[ok]);
		if (same(name, names[ok], len) && !name[len])
			return (fns[ok](a, b, o), 1);
		ok++;
	}
	return (0);
}
