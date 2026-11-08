/* Variante rush01 : rectangle avec coins /\/ et \*\/  (car. / \ *). */
void	ft_putchar(char c);

static char	cell(int r, int c, int x, int y)
{
	if (r == 1 && c == 1)
		return ('/');
	if (r == 1 && c == x)
		return ('\\');
	if (r == y && c == 1)
		return ('\\');
	if (r == y && c == x)
		return ('/');
	if (r == 1 || r == y)
		return ('*');
	if (c == 1 || c == x)
		return ('*');
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
