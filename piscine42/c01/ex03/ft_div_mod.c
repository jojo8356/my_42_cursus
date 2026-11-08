/* Range le quotient a/b dans *div et le reste a%b dans *mod. */
void	ft_div_mod(int a, int b, int *div, int *mod)
{
	*div = a / b;
	*mod = a % b;
}
