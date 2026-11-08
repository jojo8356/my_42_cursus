/* Renvoie la racine carree entiere de nb si elle est exacte, 0 sinon.
   Version iterative (la recursive casserait la pile sur les grands nb). */
int	ft_sqrt(int nb)
{
	long	i;

	if (nb <= 0)
		return (0);
	i = 1;
	while (i * i < nb)
		i++;
	if (i * i == nb)
		return ((int)i);
	return (0);
}
