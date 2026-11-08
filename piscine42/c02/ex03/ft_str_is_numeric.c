/* Renvoie 1 si tous les caracteres sont des chiffres, 0 sinon.
   Une chaine vide est consideree valide. */
int	ft_str_is_numeric(char *str)
{
	int	i;

	i = 0;
	while (str[i])
	{
		if (!(str[i] >= '0' && str[i] <= '9'))
			return (0);
		i++;
	}
	return (1);
}
