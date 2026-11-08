/* Concatene les chaines du tableau sep[0..size-1] avec le separateur
   sep entre chaque, dans un nouveau bloc malloc. */
#include <stdlib.h>

static int	total_len(int size, char **strs, char *sep)
{
	int	total;
	int	i;
	int	j;

	total = 0;
	i = 0;
	while (i < size)
	{
		j = 0;
		while (strs[i][j])
		{
			total++;
			j++;
		}
		if (i < size - 1)
		{
			j = 0;
			while (sep[j++])
				total++;
		}
		i++;
	}
	return (total);
}

static char	*join_all(char *out, int size, char **strs, char *sep)
{
	int	k;
	int	i;
	int	j;

	k = 0;
	i = 0;
	while (i < size)
	{
		j = 0;
		while (strs[i][j])
			out[k++] = strs[i][j++];
		if (i < size - 1)
		{
			j = 0;
			while (sep[j])
				out[k++] = sep[j++];
		}
		i++;
	}
	out[k] = '\0';
	return (out);
}

char	*ft_strjoin(int size, char **strs, char *sep)
{
	char	*out;

	if (size <= 0)
	{
		out = malloc(1);
		if (out)
			out[0] = '\0';
		return (out);
	}
	out = malloc(total_len(size, strs, sep) + 1);
	if (!out)
		return (0);
	return (join_all(out, size, strs, sep));
}
