/* ************************************************************************** */
/*                                                                          */
/*                                                        :::      ::::::::   */
/*   1_tokenizer_word.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/12 15:45:00 by jojo8356          #+#    #+#             */
/*   Updated: 2026/11/12 15:45:00 by jojo8356         ###   ########.fr       */
/*                                                                          */
/* ************************************************************************** */

# include "../includes/minishell.h"

static char *grow_buffer(char *buf, int *len, int *cap, char c)
{
    if (*len + 1 >= *cap)
    {
        *cap = (*cap == 0) ? 64 : *cap * 2;
        buf = (char *)realloc(buf, *cap);
    }
    buf[*len] = c;
    *len = *len + 1;
    return (buf);
}

char *read_word(char *input, int *i)
{
    char *buf = NULL;
    int len = 0;
    int cap = 0;
    int in_single = 0;
    int in_double = 0;

    while (input[*i])
    {
        char c = input[*i];
        if (in_single)
        {
            buf = grow_buffer(buf, &len, &cap, c);
            if (c == '\'')
                in_single = 0;
            (*i)++;
        }
        else if (in_double)
        {
            buf = grow_buffer(buf, &len, &cap, c);
            if (c == '"')
                in_double = 0;
            (*i)++;
        }
        else if (is_quote(c))
        {
            if (c == '\'')
                in_single = 1;
            else
                in_double = 1;
            buf = grow_buffer(buf, &len, &cap, c);
            (*i)++;
        }
        else if (c == ' ' || c == '\t' || is_operator_char(c))
            break;
        else
        {
            buf = grow_buffer(buf, &len, &cap, c);
            (*i)++;
        }
    }
    if (buf)
        buf[len] = 0;
    return (buf);
}
