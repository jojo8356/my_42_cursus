/* Comme cat sans option : lit les fichiers donnes (ou stdin si aucun),
   avec un tableau de taille fixe (~29 ko, aucun malloc). */
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <libgen.h>

static char	g_buf[29696];

static void	put_error(char *prog, char *file, char *reason)
{
	while (*prog)
		write(2, prog++, 1);
	write(2, ": ", 2);
	while (*file)
		write(2, file++, 1);
	write(2, ": ", 2);
	while (*reason)
		write(2, reason++, 1);
	write(2, "\n", 1);
}

static void	cat_fd(int fd)
{
	int	n;

	n = read(fd, g_buf, sizeof(g_buf));
	while (n > 0)
	{
		write(1, g_buf, n);
		n = read(fd, g_buf, sizeof(g_buf));
	}
}

int	main(int argc, char **argv)
{
	char	*prog;
	int		i;
	int		fd;
	int		status;

	prog = basename(argv[0]);
	status = 0;
	if (argc < 2)
		return (cat_fd(0), 0);
	i = 1;
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
			cat_fd(fd);
			close(fd);
		}
		i++;
	}
	return (status);
}
