/* Affiche la chaine en remplacant chaque caractere non imprimable par
   son code hexadecimal en minuscules, precede d'une antislash. */
#include <unistd.h>

static void	put_hex(unsigned char c)
{
	char	*base;

	base = "0123456789abcdef";
	write(1, "\\", 1);
	write(1, &base[c / 16], 1);
	write(1, &base[c % 16], 1);
}

void	ft_putstr_non_printable(char *str)
{
	while (str && *str)
	{
		if (*str >= 32 && *str <= 126)
			write(1, str, 1);
		else
			put_hex((unsigned char)*str);
		str++;
	}
}
