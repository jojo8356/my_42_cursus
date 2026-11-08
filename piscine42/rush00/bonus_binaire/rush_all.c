/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   rush_all.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/08 19:00:00 by jojo8356          #+#    #+#             */
/*   Updated: 2026/11/08 19:00:00 by jojo8356         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

/* Bonus : binaire unique regroupant les 5 variantes de rush.
   rush(x, y, version) affiche le rectangle de la version demandee
   (0 = rush00, 1 = rush01, 2 = rush02, 3 = rush03, 4 = rush04). */
void	ft_putchar(char c);

static char	cell(int r, int c, int x, int y, const char *ch)
{
	if (r == 1 && c == 1)
		return (ch[0]);
	if (r == 1 && c == x)
		return (ch[1]);
	if (r == y && c == 1)
		return (ch[2]);
	if (r == y && c == x)
		return (ch[3]);
	if (r == 1 || r == y)
		return (ch[4]);
	if (c == 1 || c == x)
		return (ch[5]);
	return (' ');
}

void	rush(int x, int y, int version)
{
	const char	*ch[5] = {"oooo-|", "/\\\\/**", "AACCBB",
		"ACACBB", "ACCABB"};
	int			r;
	int			c;

	if (x <= 0 || y <= 0 || version < 0 || version > 4)
		return ;
	r = 1;
	while (r <= y)
	{
		c = 1;
		while (c <= x)
		{
			ft_putchar(cell(r, c, x, y, ch[version]));
			c++;
		}
		ft_putchar('\n');
		r++;
	}
}
