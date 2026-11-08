/* Renvoie 1 si tous les caracteres sont imprimables, 0 sinon.
   Une chaine vide est consideree valide. */
int	ft_str_is_printable(char *str)
{
	int	i;

	i = 0;
	while (str[i])
	{
		if (str[i] < 32 || str[i] > 126)
			return (0);
		i++;
	}
	return (1);
}
