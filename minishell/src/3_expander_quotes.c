/* ************************************************************************** */
/*                                                                          */
/*                                                        :::      ::::::::   */
/*   3_expander_quotes.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/12 15:45:00 by jojo8356          #+#    #+#             */
/*   Updated: 2026/11/12 15:45:00 by jojo8356         ###   ########.fr       */
/*                                                                          */
/* ************************************************************************** */

# include "../includes/minishell.h"

static void check_closed_quotes(char *word, int *closed_single,
    int *closed_double)
{
    int in_single = 0;
    int in_double = 0;
    int i = 0;

    while (word[i])
    {
        if (word[i] == '\'' && !in_double)
            in_single = !in_single;
        else if (word[i] == '"' && !in_single)
            in_double = !in_double;
        i++;
    }
    *closed_single = !in_single;
    *closed_double = !in_double;
}

char *remove_quotes(char *word)
{
    char *result;
    int r = 0;
    int i = 0;
    int in_single = 0;
    int in_double = 0;
    int closed_single;
    int closed_double;
    int len = ft_strlen(word);

    check_closed_quotes(word, &closed_single, &closed_double);
    result = (char *)malloc(sizeof(char) * (len + 1));
    if (!result)
        return (NULL);
    while (word[i])
    {
        char c = word[i];
        if (c == '\'' && !in_double)
        {
            if (in_single)
                in_single = 0;
            else
            {
                if (closed_single)
                    in_single = 1;
                else
                    result[r++] = c;
            }
        }
        else if (c == '"' && !in_single)
        {
            if (in_double)
                in_double = 0;
            else
            {
                if (closed_double)
                    in_double = 1;
                else
                    result[r++] = c;
            }
        }
        else if (c == '\'' || c == '"')
            result[r++] = c;
        else
            result[r++] = c;
        i++;
    }
    result[r] = 0;
    return (result);
}
