/* Convertit le nombre nb_ecrit_dans_base_from en base_to et renvoie
   la nouvelle chaine malloc. Renvoie NULL si une base est invalide. */
#include <stdlib.h>

static int	base_len(char *base)
{
	int	i;
	int	j;

	i = 0;
	while (base[i])
	{
		if (base[i] == '+' || base[i] == '-')
			return (0);
		j = i + 1;
		while (base[j])
		{
			if (base[i] == base[j])
				return (0);
			j++;
		}
		i++;
	}
	return (i * (i >= 2));
}

static int	base_index(char c, char *base)
{
	int	i;

	i = 0;
	while (base[i])
	{
		if (base[i] == c)
			return (i);
		i++;
	}
	return (-1);
}

static long	conv_value(char *str, char *base, int len)
{
	long	nb;
	int		sign;
	int		d;

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
	d = base_index(*str, base);
	while (d >= 0)
	{
		nb = nb * len + d;
		str++;
		d = base_index(*str, base);
	}
	return (sign * nb);
}

static char	*conv_out(long nb, char *base, int len)
{
	char	buf[65];
	char	*out;
	int		i;
	int		j;
	int		neg;

	neg = 0;
	if (nb < 0)
	{
		neg = 1;
		nb = -nb;
	}
	i = 0;
	if (nb == 0)
		buf[i++] = base[0];
	while (nb > 0)
	{
		buf[i++] = base[nb % len];
		nb /= len;
	}
	out = malloc(i + neg + 1);
	if (!out)
		return (0);
	j = 0;
	if (neg)
		out[j++] = '-';
	while (i > 0)
		out[j++] = buf[--i];
	out[j] = '\0';
	return (out);
}

char	*ft_convert_base(char *nbr, char *base_from, char *base_to)
{
	long	value;
	int		len_from;
	int		len_to;

	len_from = base_len(base_from);
	len_to = base_len(base_to);
	if (len_from == 0 || len_to == 0)
		return (0);
	value = conv_value(nbr, base_from, len_from);
	return (conv_out(value, base_to, len_to));
}
