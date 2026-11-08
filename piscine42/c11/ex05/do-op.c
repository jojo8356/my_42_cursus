/* do-op : calcule "valeur1 operateur valeur2" (operateurs + - * / %).
   Operateur invalide -> 0. Division ou modulo par zero -> "Stop : ...".
   Nombre d arguments invalide -> affiche rien. */
#include <unistd.h>
#include "do-op.h"

int	ft_atoi(char *str)
{
	int	sign;
	int	nb;

	while (*str == ' ' || (*str >= 9 && *str <= 13))
		str++;
	sign = 1;
	while (*str == '+' || *str == '-')
	{
		if (*str == '-')
			sign = -sign;
		str++;
	}
	nb = 0;
	while (*str >= '0' && *str <= '9')
	{
		nb = nb * 10 + (*str - '0');
		str++;
	}
	return (sign * nb);
}

static void	put_nbr(int n)
{
	char			buf[12];
	int				i;
	unsigned int	nb;

	nb = 0;
	if (n < 0)
	{
		write(1, "-", 1);
		nb = -(unsigned int)n;
	}
	else
		nb = (unsigned int)n;
	i = 0;
	if (nb == 0)
		buf[i++] = '0';
	while (nb > 0)
	{
		buf[i++] = '0' + nb % 10;
		nb /= 10;
	}
	while (i > 0)
		write(1, &buf[--i], 1);
}

static int	find_op(char *op, int (*f[5])(int, int))
{
	char	*ops;
	int		i;

	ops = "+-*/%";
	i = 0;
	while (i < 5)
	{
		if (op[0] == ops[i] && !op[1])
			return (i);
		i++;
	}
	(void)f;
	return (-1);
}

int	main(int argc, char **argv)
{
	int	(*f[5])(int, int);
	int	idx;
	int	b;

	if (argc != 4)
		return (0);
	f[0] = op_add;
	f[1] = op_sub;
	f[2] = op_mul;
	f[3] = op_div;
	f[4] = op_mod;
	idx = find_op(argv[2], f);
	if (idx < 0)
		return (put_nbr(0), write(1, "\n", 1), 0);
	b = ft_atoi(argv[3]);
	if (b == 0 && idx == 3)
		return (write(1, "Stop : division by zero\n", 24), 0);
	if (b == 0 && idx == 4)
		return (write(1, "Stop : modulo by zero\n", 22), 0);
	put_nbr(f[idx](ft_atoi(argv[1]), b));
	write(1, "\n", 1);
	return (0);
}
