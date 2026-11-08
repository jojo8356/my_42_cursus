/* Affiche le nom du programme (argv[0]), suivi d'un retour a la ligne. */
#include <unistd.h>

int	main(int argc, char **argv)
{
	char	*p;

	(void)argc;
	p = argv[0];
	while (*p)
	{
		write(1, p, 1);
		p++;
	}
	write(1, "\n", 1);
	return (0);
}
