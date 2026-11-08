/* Composition du nombre en mots a partir des entrees du dictionnaire.
   w = 1 : bonus style ecrit (tirets, virgules, and). */
#include "rush02.h"

static void	add(char *out, int *pos, char *s)
{
	int	i;

	i = 0;
	while (s[i])
	{
		out[*pos] = s[i];
		(*pos)++;
		i++;
	}
	out[*pos] = 0;
}

static int	word_of(t_dict *d, unsigned long long m, char *out, int *pos, int w)
{
	char	*z;
	int		i;
	int		sp;

	z = dict_find(d, m);
	if (!z && m > 20 && m % 10)
	{
		if (!word_of(d, m - m % 10, out, pos, w))
			return (0);
		add(out, pos, (w && !d->lang) ? "-" : " ");
		return (word_of(d, m % 10, out, pos, w));
	}
	if (!z)
		return (0);
	if (w && !d->lang)
	{
		sp = -1;
		i = -1;
		while (z[++i])
			if (z[i] == ' ' && sp >= 0)
				sp = -2;
			else if (z[i] == ' ')
				sp = i;
			else if (!((z[i] >= 'a' && z[i] <= 'z')))
				sp = -2;
		if (sp > 0)
		{
			z[sp] = '-';
			add(out, pos, z);
			z[sp] = ' ';
			return (1);
		}
	}
	add(out, pos, z);
	return (1);
}

static int	chunk3(t_dict *d, unsigned long long m, char *out, int *pos, int w)
{
	unsigned long long	h;
	unsigned long long	r;
	char				*z;

	h = m / 100;
	r = m % 100;
	if (h && !(d->lang && h == 1))
	{
		if (!word_of(d, h, out, pos, w))
			return (0);
		add(out, pos, " ");
	}
	if (h)
	{
		z = dict_find(d, 100);
		if (!z)
			return (0);
		add(out, pos, z);
		if (d->lang && h > 1 && !r)
			add(out, pos, "s");
		if (r)
			add(out, pos, (w && !d->lang) ? " and " : " ");
	}
	if (r || !h)
		return (word_of(d, r, out, pos, w));
	return (1);
}

static int	add_group(t_dict *d, unsigned long long s, unsigned long long mult,
		char *out, int *pos, int w)
{
	char	*z;

	z = dict_find(d, s);
	if (!z)
		return (0);
	if (!(d->lang && s == 1000 && mult == 1))
	{
		if (!chunk3(d, mult, out, pos, w))
			return (0);
		add(out, pos, " ");
	}
	add(out, pos, z);
	if (d->lang && s >= 1000000 && mult > 1)
		add(out, pos, "s");
	return (1);
}

int	convert(t_dict *d, unsigned long long n, int w, char *out)
{
	unsigned long long	sc[6];
	int					p[2];
	int					i;

	sc[0] = 1000000000000000000ULL;
	sc[1] = 1000000000000000ULL;
	sc[2] = 1000000000000ULL;
	sc[3] = 1000000000ULL;
	sc[4] = 1000000ULL;
	sc[5] = 1000ULL;
	p[0] = 0;
	p[1] = 1;
	out[0] = 0;
	if (n == 0)
		return (word_of(d, 0, out, &p[0], w));
	i = -1;
	while (++i < 6)
	{
		if (n / sc[i])
		{
			if (!p[1])
				add(out, &p[0], (w && !d->lang) ? ", " : " ");
			if (!add_group(d, sc[i], n / sc[i], out, &p[0], w))
				return (0);
			p[1] = 0;
		}
		n %= sc[i];
	}
	if (n)
	{
		if (!p[1])
			add(out, &p[0], (w && !d->lang) ? ", " : " ");
		return (chunk3(d, n, out, &p[0], w));
	}
	return (1);
}
