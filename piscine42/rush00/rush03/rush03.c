/* Variante rush03 : rectangle avec coins A/C et A/C. */
void	ft_putchar(char c);

static char	cell(int r, int c, int x, int y)
{
	if (r == 1 && c == 1)
		return ('A');
	if (r == 1 && c == x)
		return ('C');
	if (r == y && c == 1)
		return ('A');
	if (r == y && c == x)
		return ('C');
	if (r == 1 || r == y)
		return ('B');
	if (c == 1 || c == x)
		return ('B');
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
