/* Renvoie 1 si tous les caracteres sont des minuscules, 0 sinon.
   Une chaine vide est consideree valide. */
int	ft_str_is_lowercase(char *str)
{
	int	i;

	i = 0;
	while (str[i])
	{
		if (!(str[i] >= 'a' && str[i] <= 'z'))
			return (0);
		i++;
	}
	return (1);
}
