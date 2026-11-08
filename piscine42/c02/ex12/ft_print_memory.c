/* Affiche size octets a l'adresse addr comme un dump hexadecimal :
   offset, octets par groupes de 2, puis la representation ASCII
   ('.' pour les non-imprimables). Renvoie addr. */
#include <unistd.h>

static void	put_hexn(unsigned long v, int width)
{
	char	*base;
	char	c;

	base = "0123456789abcdef";
	if (width > 1)
		put_hexn(v / 16, width - 1);
	c = base[v % 16];
	write(1, &c, 1);
}

static void	put_line_hex(unsigned char *p, int len)
{
	int	i;

	write(1, ": ", 2);
	i = 0;
	while (i < 16)
	{
		if (i < len)
			put_hexn(p[i], 2);
		else
			write(1, "  ", 2);
		if (i % 2 == 1)
			write(1, " ", 1);
		i++;
	}
}

static void	put_line_ascii(unsigned char *p, int len)
{
	int	i;

	i = 0;
	while (i < len)
	{
		if (p[i] >= 32 && p[i] <= 126)
			write(1, &p[i], 1);
		else
			write(1, ".", 1);
		i++;
	}
	write(1, "\n", 1);
}

void	*ft_print_memory(void *addr, unsigned int size)
{
	unsigned char	*p;
	unsigned int	off;
	int				len;

	p = (unsigned char *)addr;
	off = 0;
	while (off < size)
	{
		len = 16;
		if (size - off < 16)
			len = (int)(size - off);
		put_hexn(off, 8);
		put_line_hex(p + off, len);
		put_line_ascii(p + off, len);
		off += 16;
	}
	return (addr);
}
