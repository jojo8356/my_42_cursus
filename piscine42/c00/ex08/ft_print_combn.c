/* Affiche toutes les combinaisons de n chiffres distincts en ordre
   croissant, separees par ", ". n doit etre entre 1 et 9. */
#include <unistd.h>

static int	is_last(int *d, int n)
{
	int	i;

	i = 0;
	while (i < n)
	{
		if (d[i] != 10 - n + i)
			return (0);
		i++;
	}
	return (1);
}

static void	put_comb(int *d, int n)
{
	char	c;
	int		i;

	i = 0;
	while (i < n)
	{
		c = d[i] + '0';
		write(1, &c, 1);
		i++;
	}
	if (!is_last(d, n))
		write(1, ", ", 2);
}

static void	next_comb(int *d, int n)
{
	int	i;

	i = n - 1;
	while (i >= 0 && d[i] == 10 - n + i)
		i--;
	if (i >= 0)
	{
		d[i]++;
		i++;
		while (i < n)
		{
			d[i] = d[i - 1] + 1;
			i++;
		}
	}
}

void	ft_print_combn(int n)
{
	int	d[9];
	int	i;

	if (n <= 0 || n > 9)
		return ;
	i = 0;
	while (i < n)
	{
		d[i] = i;
		i++;
	}
	while (1)
	{
		put_comb(d, n);
		if (is_last(d, n))
			break ;
		next_comb(d, n);
	}
}
