/* ************************************************************************** */
/*                                                                          */
/*                                                        :::      ::::::::   */
/*   3_expander_var.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/12 15:45:00 by jojo8356          #+#    #+#             */
/*   Updated: 2026/11/12 15:45:00 by jojo8356         ###   ########.fr       */
/*                                                                          */
/* ************************************************************************** */

# include "../includes/minishell.h"

static int is_var_char(char c)
{
    return (ft_isalnum(c) || c == '_');
}

static void expand_dollar_var(char *word, int *i, int exit_code,
    char *result, int *r)
{
    char var_name[256];
    char *value;
    int j = *i + 1;
    int start;
    int name_len;
    int val_len;

    if (word[j] == '?')
    {
        char *code = ft_itoa(exit_code);
        val_len = ft_strlen(code);
        ft_strlcpy(result + *r, code, val_len + 1);
        *r += val_len;
        free(code);
        *i = j + 1;
        return;
    }
    start = j;
    while (word[j] && is_var_char(word[j]))
        j++;
    if (j > start)
    {
        name_len = j - start;
        ft_strlcpy(var_name, word + start, name_len + 1);
        value = getenv(var_name);
        if (value)
        {
            val_len = ft_strlen(value);
            ft_strlcpy(result + *r, value, val_len + 1);
            *r += val_len;
        }
        *i = j;
    }
    else
    {
        result[*r] = '$';
        (*r)++;
        (*i)++;
    }
}

char *expand_word(char *word, int exit_code)
{
    int word_len;
    int cap;
    char *result;
    int r = 0;
    int i = 0;
    int in_single = 0;
    int in_double = 0;

    word_len = ft_strlen(word);
    cap = word_len * 8 + 1;
    result = (char *)malloc(sizeof(char) * cap);
    if (!result)
        return (NULL);
    while (word[i])
    {
        char c = word[i];
        if (c == '\'' && !in_double)
        {
            in_single = !in_single;
            result[r++] = c;
            i++;
        }
        else if (c == '"' && !in_single)
        {
            in_double = !in_double;
            result[r++] = c;
            i++;
        }
        else if (c == '$' && !in_single)
            expand_dollar_var(word, &i, exit_code, result, &r);
        else
        {
            result[r++] = c;
            i++;
        }
    }
    result[r] = 0;
    return (result);
}
