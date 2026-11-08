/* Affiche 'N', 'P' ou 'Z' selon le signe de n, avec retour a la ligne. */
#include <unistd.h>

void	ft_is_negative(int n)
{
	if (n < 0)
		write(1, "N\n", 2);
	else if (n > 0)
		write(1, "P\n", 2);
	else
		write(1, "Z\n", 2);
}
