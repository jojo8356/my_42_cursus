/* Rush02 : ./rush-02 [dictionnaire] nombre [-w]   ("-" lit stdin). */
#include <stdlib.h>
#include <unistd.h>
#include "rush02.h"

static void	handle_one(t_dict *d, char *num, int w)
{
	char					out[8192];
	unsigned long long		n;
	int						ok;

	n = ft_atoull(num, &ok);
	if (!ok)
	{
		ft_putstr("Error\n");
		return ;
	}
	if (!convert(d, n, w, out))
	{
		ft_putstr("Dict Error\n");
		return ;
	}
	ft_putstr(out);
	ft_putstr("\n");
}

static void	read_stdin(t_dict *d, int w)
{
	char	buf[4096];
	char	line[128];
	int		r;
	int		i;
	int		l;

	r = read(0, buf, 4095);
	while (r > 0)
	{
		buf[r] = 0;
		i = 0;
		while (i < r)
		{
			l = 0;
			while (i < r && buf[i] != '\n')
				line[l++] = buf[i++];
			line[l] = 0;
			if (l > 0 || (i < r && buf[i] == '\n'))
				handle_one(d, line, w);
			i++;
		}
		r = read(0, buf, 4095);
	}
}

static int	parse_args(int argc, char **argv, char **dict, char **num, int *w)
{
	int	i;
	int	nb;

	*dict = "numbers.dict";
	*num = 0;
	*w = 0;
	i = 1;
	nb = 0;
	while (i < argc)
	{
		if (argv[i][0] == '-' && argv[i][1] == 'w' && !argv[i][2])
			*w = 1;
		else if (!*num)
			*num = argv[i];
		else
		{
			*dict = *num;
			*num = argv[i];
		}
		nb++;
		i++;
	}
	return (*num != 0 && nb <= 2 + *w);
}

static void	free_dict(t_dict *d)
{
	int	i;

	i = 0;
	while (i < d->count)
	{
		free(d->val[i]);
		i++;
	}
}

int	main(int argc, char **argv)
{
	t_dict	d;
	char	*dict;
	char	*num;
	int		w;

	if (!parse_args(argc, argv, &dict, &num, &w))
	{
		ft_putstr("Error\n");
		return (1);
	}
	if (!dict_load(dict, &d))
	{
		ft_putstr("Dict Error\n");
		return (1);
	}
	if (num[0] == '-' && !num[1])
		read_stdin(&d, w);
	else
		handle_one(&d, num, w);
	free_dict(&d);
	return (0);
}
