/* Renvoie le nombre de caracteres de la chaine (hors '\0'). */
int	ft_strlen(char *str)
{
	int	len;

	len = 0;
	while (str[len])
		len++;
	return (len);
}
