/* Affiche la chaine passee en parametre sur la sortie standard. */
#include <unistd.h>

void	ft_putstr(char *str)
{
	while (str && *str)
	{
		write(1, str, 1);
		str++;
	}
}
