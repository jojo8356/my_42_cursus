/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   do_op_ops.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/08 19:00:00 by jojo8356          #+#    #+#             */
/*   Updated: 2026/11/08 19:00:00 by jojo8356         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

/* Les cinq operateurs arithmetiques de do-op. */
int	op_add(int a, int b)
{
	return (a + b);
}

int	op_sub(int a, int b)
{
	return (a - b);
}

int	op_mul(int a, int b)
{
	return (a * b);
}

int	op_div(int a, int b)
{
	return (a / b);
}

int	op_mod(int a, int b)
{
	return (a % b);
}
