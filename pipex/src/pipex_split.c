/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   pipex_split.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/08 12:00:00 by jojo8356          #+#    #+#             */
/*   Updated: 2026/11/08 12:00:00 by jojo8356         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "pipex.h"

/*
** Splits a command line on spaces, ignoring runs of spaces and leading or
** trailing ones. Unlike ft_split() the empty fields are dropped, so that
** "  ls   -l  " becomes {"ls", "-l", NULL}: this is what execve() wants and
** what a shell produces for an unquoted command line.
*/

static size_t	px_count_fields(const char *line, char sep)
{
	size_t	n;
	int		in_field;

	n = 0;
	in_field = 0;
	while (*line)
	{
		if (*line == sep)
			in_field = 0;
		else if (!in_field)
		{
			in_field = 1;
			n++;
		}
		line++;
	}
	return (n);
}

static char	*px_dup_field(const char *start, size_t len)
{
	char	*field;
	size_t	i;

	field = malloc(len + 1);
	if (!field)
		return (NULL);
	i = 0;
	while (i < len)
	{
		field[i] = start[i];
		i++;
	}
	field[len] = '\0';
	return (field);
}

void	px_free_tab(char **tab)
{
	size_t	i;

	if (!tab)
		return ;
	i = 0;
	while (tab[i])
	{
		free(tab[i]);
		i++;
	}
	free(tab);
}

size_t	px_tablen(char **tab)
{
	size_t	n;

	n = 0;
	while (tab && tab[n])
		n++;
	return (n);
}

char	**px_split(const char *line, char sep)
{
	char	**tab;
	size_t	i;
	size_t	len;

	if (!line)
		return (NULL);
	tab = malloc(sizeof(char *) * (px_count_fields(line, sep) + 1));
	if (!tab)
		return (NULL);
	i = 0;
	while (*line)
	{
		while (*line == sep)
			line++;
		len = 0;
		while (line[len] && line[len] != sep)
			len++;
		if (len)
		{
			tab[i] = px_dup_field(line, len);
			if (!tab[i])
				return (px_free_tab(tab), NULL);
			i++;
		}
		line += len;
	}
	tab[i] = NULL;
	return (tab);
}
