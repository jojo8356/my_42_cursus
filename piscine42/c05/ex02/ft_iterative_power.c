/* nb eleve a la puissance power, en iteratif.
   Renvoie 1 si power vaut 0, 0 si power est negatif. */
int	ft_iterative_power(int nb, int power)
{
	int	res;

	if (power < 0)
		return (0);
	res = 1;
	while (power > 0)
	{
		res *= nb;
		power--;
	}
	return (res);
}
