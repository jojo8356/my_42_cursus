/* Affiche un caractere sur la sortie standard. */
#include <unistd.h>

void	ft_putchar(char c)
{
	write(1, &c, 1);
}
