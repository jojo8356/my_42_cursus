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
	i = 0;
	while (i < 16)
	{
		if (i < cnt)
		{
			put_hex(p[i], 2);
			write(1, " ", 1);
		}
		else
			write(1, "   ", 3);
		if (i == 7)
			write(1, " ", 1);
		i++;
	}
	write(1, " |", 2);
	i = 0;
	while (i < cnt)
	{
		if (p[i] >= 32 && p[i] <= 126)
			write(1, &p[i], 1);
		else
			write(1, ".", 1);
		i++;
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
			line[*cnt] = buf[i];
			(*cnt)++;
			(*offset)++;
			if (*cnt == 16)
			{
				dump_line(line, 16, *offset - 16);
				*cnt = 0;
			}
			i++;
		}
		r = read(fd, buf, sizeof(buf));
	}
}

int	main(int argc, char **argv)
{
	unsigned char	line[16];
	char			*prog;
	long			offset;
	int				cnt;
	int				i;
	int				fd;
	int				status;

	prog = basename(argv[0]);
	offset = 0;
	cnt = 0;
	status = 0;
	i = 1;
	if (i < argc && !strcmp(argv[i], "-C"))
		i++;
	if (i >= argc)
		dump_fd(0, &offset, line, &cnt);
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
			dump_fd(fd, &offset, line, &cnt);
			close(fd);
		}
		i++;
	}
	if (cnt > 0)
		dump_line(line, cnt, offset - cnt);
	put_hex(offset, 8);
	write(1, "\n", 1);
	return (status);
}
