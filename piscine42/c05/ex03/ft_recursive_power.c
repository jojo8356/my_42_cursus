/* nb eleve a la puissance power, en recursif.
   Renvoie 1 si power vaut 0, 0 si power est negatif. */
int	ft_recursive_power(int nb, int power)
{
	if (power < 0)
		return (0);
	if (power == 0)
		return (1);
	return (nb * ft_recursive_power(nb, power - 1));
}
