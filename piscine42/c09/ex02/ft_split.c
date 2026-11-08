/* Decoupe str en mots separes par les caracteres de charset, et renvoie
   un tableau malloc termine par 0 (pas de chaine vide dedans). */
#include <stdlib.h>

static int	is_sep(char c, char *charset)
{
	int	i;

	i = 0;
	while (charset[i])
	{
		if (charset[i] == c)
			return (1);
		i++;
	}
	return (0);
}

static int	count_words(char *str, char *charset)
{
	int	count;
	int	in_word;

	count = 0;
	in_word = 0;
	while (*str)
	{
		if (!is_sep(*str, charset) && !in_word)
		{
			in_word = 1;
			count++;
		}
		else if (is_sep(*str, charset))
			in_word = 0;
		str++;
	}
	return (count);
}

static char	*dup_word(char *str, int len)
{
	char	*word;
	int		i;

	word = malloc(len + 1);
	if (!word)
		return (0);
	i = 0;
	while (i < len)
	{
		word[i] = str[i];
		i++;
	}
	word[i] = '\0';
	return (word);
}

char	**ft_split(char *str, char *charset)
{
	char	**tab;
	int		words;
	int		w;
	int		len;

	words = count_words(str, charset);
	tab = malloc(sizeof(char *) * (words + 1));
	if (!tab)
		return (0);
	w = 0;
	while (*str && w < words)
	{
		while (*str && is_sep(*str, charset))
			str++;
		len = 0;
		while (str[len] && !is_sep(str[len], charset))
			len++;
		tab[w] = dup_word(str, len);
		if (!tab[w])
			return (0);
		str += len;
		w++;
	}
	tab[w] = 0;
	return (tab);
}
