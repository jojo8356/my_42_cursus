/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_split.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: student42 <student42@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/04 08:00:00 by student42         #+#    #+#             */
/*   Updated: 2026/11/04 08:00:00 by student42        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

static size_t	ft_count_words(char const *s, char c)
{
	size_t	count;
	size_t	i;

	count = 0;
	i = 0;
	while (s[i] != '\0')
	{
		while (s[i] != '\0' && s[i] == c)
			i++;
		if (s[i] != '\0')
		{
			count++;
			while (s[i] != '\0' && s[i] != c)
				i++;
		}
	}
	return (count);
}

static char	**ft_free_split(char **arr, size_t used)
{
	while (used > 0)
	{
		used--;
		free(arr[used]);
	}
	free(arr);
	return (NULL);
}

static char	*ft_extract_word(char const *s, size_t *idx, char c)
{
	size_t	start;
	size_t	len;
	char	*word;

	while (s[*idx] != '\0' && s[*idx] == c)
		(*idx)++;
	start = *idx;
	while (s[*idx] != '\0' && s[*idx] != c)
		(*idx)++;
	len = *idx - start;
	word = (char *)malloc(len + 1);
	if (!word)
		return (NULL);
	ft_memcpy(word, s + start, len);
	word[len] = '\0';
	return (word);
}

char	**ft_split(char const *s, char c)
{
	char	**arr;
	size_t	words;
	size_t	i;
	size_t	w;

	if (!s)
		return (NULL);
	words = ft_count_words(s, c);
	arr = (char **)malloc(sizeof(char *) * (words + 1));
	if (!arr)
		return (NULL);
	i = 0;
	w = 0;
	while (w < words)
	{
		arr[w] = ft_extract_word(s, &i, c);
		if (!arr[w])
			return (ft_free_split(arr, w));
		w++;
	}
	arr[words] = NULL;
	return (arr);
}
