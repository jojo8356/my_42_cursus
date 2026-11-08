/* Renvoie le nieme terme de Fibonacci (F0 = 0, F1 = 1), recursif.
   Renvoie -1 si index est negatif. */
int	ft_fibonacci(int index)
{
	if (index < 0)
		return (-1);
	if (index < 2)
		return (index);
	return (ft_fibonacci(index - 1) + ft_fibonacci(index - 2));
}
