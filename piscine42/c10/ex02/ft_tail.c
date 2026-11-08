/* Comme tail : seule l'option -c N est geree (les N derniers octets).
   Plusieurs fichiers : en-tetes "==> f <==" avec ligne vide entre eux.
   Sans fichier : lit stdin. */
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <libgen.h>
#include "ft_tail.h"

static long	parse_n(char *arg)
{
	long	n;

	n = 0;
	if (!*arg)
		return (0);
	while (*arg >= '0' && *arg <= '9')
	{
		n = n * 10 + (*arg - '0');
		arg++;
	}
	return (n);
}

static void	print_header(char *file, int *first)
{
	if (!*first)
		write(1, "\n", 1);
	*first = 0;
	write(1, "==> ", 4);
	while (*file)
		write(1, file++, 1);
	write(1, " <==\n", 5);
}

static int	skip_options(int argc, char **argv, long *n)
{
	int	i;

	i = 1;
	while (i < argc)
	{
		if (argv[i][0] == '-' && argv[i][1] == 'c')
		{
			if (argv[i][2])
				*n = parse_n(argv[i] + 2);
			else if (i + 1 < argc)
			{
				*n = parse_n(argv[i + 1]);
				i++;
			}
		}
		else
			break ;
		i++;
	}
	return (i);
}

int	main(int argc, char **argv)
{
	char	*prog;
	long	n;
	int		i;
	int		fd;
	int		first;
	int		status;
	int		nfiles;

	prog = basename(argv[0]);
	n = 0;
	status = 0;
	first = 1;
	i = skip_options(argc, argv, &n);
	if (i >= argc)
		return (tail_fd(0, n), 0);
	nfiles = argc - i;
	while (i < argc)
	{
		fd = open(argv[i], O_RDONLY);
		if (fd < 0)
		{
			put_error(prog, argv[i], strerror(errno));
			status = 1;
		}
		else
		{
			if (nfiles > 1)
				print_header(argv[i], &first);
			tail_fd(fd, n);
			close(fd);
		}
		i++;
	}
	return (status);
}
