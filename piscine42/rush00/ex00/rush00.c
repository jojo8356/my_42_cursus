/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   rush00.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/08 19:00:00 by jojo8356          #+#    #+#             */
/*   Updated: 2026/11/08 19:00:00 by jojo8356         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

/* Variante rush00 : rectangle avec coins o/o et o/o. */
void	ft_putchar(char c);

static char	cell(int r, int c, int x, int y)
{
	if (r == 1 && c == 1)
		return ('o');
	if (r == 1 && c == x)
		return ('o');
	if (r == y && c == 1)
		return ('o');
	if (r == y && c == x)
		return ('o');
	if (r == 1 || r == y)
		return ('-');
	if (c == 1 || c == x)
		return ('|');
	return (' ');
}

void	rush(int x, int y)
{
	int	r;
	int	c;

	if (x <= 0 || y <= 0)
		return ;
	r = 1;
	while (r <= y)
	{
		c = 1;
		while (c <= x)
		{
			ft_putchar(cell(r, c, x, y));
			c++;
		}
		ft_putchar('\n');
		r++;
	}
}
