/* Petites utilitaires chaines (aucune libc autorisee). */
#include <unistd.h>
#include <stdlib.h>
#include "rush02.h"

int	ft_strlen(char *s)
{
	int	i;

	i = 0;
	while (s[i])
		i++;
	return (i);
}

void	ft_putstr(char *s)
{
	write(1, s, ft_strlen(s));
}

int	ft_strcmp(char *a, char *b)
{
	int	i;

	i = 0;
	while (a[i] && a[i] == b[i])
		i++;
	return (a[i] - b[i]);
}

char	*ft_strdup_n(char *s, int len)
{
	char	*dup;
	int		i;

	dup = malloc(len + 1);
	if (!dup)
		return (0);
	i = 0;
	while (i < len)
	{
		dup[i] = s[i];
		i++;
	}
	dup[len] = 0;
	return (dup);
}

unsigned long long	ft_atoull(char *s, int *ok)
{
	unsigned long long	n;
	int					i;

	n = 0;
	i = 0;
	*ok = 1;
	if (!s[0])
	{
		*ok = 0;
		return (n);
	}
	while (s[i])
	{
		if (s[i] < '0' || s[i] > '9' || i >= 19)
		{
			*ok = 0;
			return (n);
		}
		n = n * 10 + (s[i] - '0');
		i++;
	}
	if (n > 1000000000000000000ULL)
		*ok = 0;
	return (n);
}
