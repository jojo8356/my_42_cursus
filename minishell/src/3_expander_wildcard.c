/* ************************************************************************** */
/*                                                                          */
/*                                                        :::      ::::::::   */
/*   3_expander_wildcard.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/12 15:45:00 by jojo8356          #+#    #+#             */
/*   Updated: 2026/11/12 15:45:00 by jojo8356         ###   ########.fr       */
/*                                                                          */
/* ************************************************************************** */

# include "../includes/minishell.h"

static int matches_wildcard(char *pattern, char *filename)
{
    int i;
    int j;
    int len;
    int flen;

    i = 0;
    j = 0;
    len = ft_strlen(pattern);
    flen = (int)ft_strlen(filename);
    while (i < len && j < flen)
    {
        if (pattern[i] == '*')
        {
            if (i == len - 1)
                return (1);
            while (j < flen)
            {
                if (matches_wildcard(pattern + i + 1, filename + j))
                    return (1);
                j++;
            }
            return (0);
        }
        if (pattern[i] != filename[j])
            return (0);
        i++;
        j++;
    }
    return (i == len && j == flen);
}

static void append_to_result(char ***result, char *str, int *count, int *cap)
{
    if (*count >= *cap)
    {
        *cap = (*cap == 0) ? 8 : *cap * 2;
        *result = (char **)realloc(*result, sizeof(char *) * *cap);
    }
    (*result)[*count] = str;
    (*count)++;
}

char **expand_wildcards(char **argv)
{
    char **result;
    int count;
    int cap;
    int i;
    int j;
    char *pattern;
    DIR *dir;
    struct dirent *entry;
    char **matches;
    int mcount;

    if (!argv)
        return (NULL);
    result = NULL;
    count = 0;
    cap = 0;
    i = 0;
    while (argv[i])
    {
        pattern = argv[i];
        if (ft_strchr(pattern, '*'))
        {
            dir = opendir(".");
            matches = NULL;
            mcount = 0;
            if (dir)
            {
                while ((entry = readdir(dir)) != NULL)
                {
                    if (ft_strcmp(entry->d_name, ".") == 0 ||
                        ft_strcmp(entry->d_name, "..") == 0)
                        continue;
                    if (matches_wildcard(pattern, entry->d_name))
                    {
                        mcount++;
                        matches = (char **)realloc(matches,
                            sizeof(char *) * (mcount + 1));
                        matches[mcount - 1] = ft_strdup(entry->d_name);
                        matches[mcount] = NULL;
                    }
                }
                closedir(dir);
            }
            if (matches && matches[0])
            {
                for (j = 0; matches[j]; j++)
                    append_to_result(&result, matches[j], &count, &cap);
            }
            else
                append_to_result(&result, ft_strdup(pattern), &count, &cap);
        }
        else
            append_to_result(&result, ft_strdup(pattern), &count, &cap);
        i++;
    }
    if (result)
        result[count] = NULL;
    return (result);
}
