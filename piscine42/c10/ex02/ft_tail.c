/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_tail.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/08 19:00:00 by jojo8356          #+#    #+#             */
/*   Updated: 2026/11/08 19:00:00 by jojo8356         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

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
	while (i < argc && argv[i][0] == '-' && argv[i][1] == 'c')
	{
		if (argv[i][2])
			*n = parse_n(argv[i] + 2);
		else if (i + 1 < argc)
		{
			*n = parse_n(argv[i + 1]);
			i++;
		}
		i++;
	}
	return (i);
}

static int	handle_file(char *prog, char *path, long n, int *hdr)
{
	int	fd;

	fd = open(path, O_RDONLY);
	if (fd < 0)
	{
		put_error(prog, path, strerror(errno));
		return (1);
	}
	if (hdr[1] > 1)
		print_header(path, &hdr[0]);
	tail_fd(fd, n);
	close(fd);
	return (0);
}

int	main(int argc, char **argv)
{
	char	*prog;
	long	n;
	int		i;
	int		hdr[2];
	int		status;

	prog = basename(argv[0]);
	n = 0;
	status = 0;
	hdr[0] = 1;
	i = skip_options(argc, argv, &n);
	if (i >= argc)
		return (tail_fd(0, n), 0);
	hdr[1] = argc - i;
	while (i < argc)
	{
		if (handle_file(prog, argv[i], n, hdr))
			status = 1;
		i++;
	}
	return (status);
}
