/* Convertit la chaine str (dans la base donnee) en int. Ignore les
   espaces et les signes. Renvoie 0 si la base est invalide. */
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
	if (i < 2)
		return (0);
	return (i);
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

int	ft_atoi_base(char *str, char *base)
{
	int	len;
	int	sign;
	int	nb;
	int	d;

	len = base_len(base);
	if (!len)
		return (0);
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
