/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_hexdump.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/08 19:00:00 by jojo8356          #+#    #+#             */
/*   Updated: 2026/11/08 19:00:00 by jojo8356         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

/* Comme hexdump -C : dump hexa 16 octets/ligne avec colonne ASCII,
   puis la taille totale en hexadecimal sur la derniere ligne.
   Plusieurs fichiers sont concatenes, ou stdin sans fichier. */
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <libgen.h>

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

static void	put_hex(long v, int width)
{
	char	*base;
	char	c;

	base = "0123456789abcdef";
	if (width > 1)
		put_hex(v / 16, width - 1);
	c = base[v % 16];
	write(1, &c, 1);
}

static void	dump_line(unsigned char *p, int cnt, long offset)
{
	int	i;

	put_hex(offset, 8);
	write(1, "  ", 2);
	i = -1;
	while (++i < 16)
	{
		if (i < cnt)
			put_hex(p[i], 2);
		else
			write(1, "  ", 2);
		write(1, "  ", 1 + (i == 7));
	}
	write(1, " |", 2);
	i = -1;
	while (++i < cnt)
	{
		if (p[i] >= 32 && p[i] <= 126)
			write(1, &p[i], 1);
		else
			write(1, ".", 1);
	}
	write(1, "|\n", 2);
}

static void	dump_fd(int fd, long *offset, unsigned char *line, int *cnt)
{
	unsigned char	buf[29696];
	int				r;
	int				i;

	r = read(fd, buf, sizeof(buf));
	while (r > 0)
	{
		i = 0;
		while (i < r)
		{
			line[*cnt] = buf[i++];
			(*cnt)++;
			(*offset)++;
			if (*cnt == 16)
			{
				dump_line(line, 16, *offset - 16);
				*cnt = 0;
			}
		}
		r = read(fd, buf, sizeof(buf));
	}
}

int	main(int argc, char **argv)
{
	unsigned char	line[16];
	long			off;
	int				s[4];

	off = 0;
	s[0] = 0;
	s[1] = 0;
	s[2] = (argc > 1 && argv[1][0] == '-' && argv[1][1] == 'C' && !argv[1][2]);
	if (s[2] + 1 >= argc)
		dump_fd(0, &off, line, &s[0]);
	while (++s[2] < argc)
	{
		s[3] = open(argv[s[2]], O_RDONLY);
		if (s[3] < 0 && ++s[1])
			put_error(basename(argv[0]), argv[s[2]], strerror(errno));
		if (s[3] >= 0)
			dump_fd(s[3], &off, line, &s[0]);
		if (s[3] >= 0)
			close(s[3]);
	}
	if (s[0] > 0)
		dump_line(line, s[0], off - s[0]);
	return (put_hex(off, 8), write(1, "\n", 1), s[1] > 0);
}
