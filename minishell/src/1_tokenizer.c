/* ************************************************************************** */
/*                                                                          */
/*                                                        :::      ::::::::   */
/*   1_tokenizer.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/12 15:45:00 by jojo8356          #+#    #+#             */
/*   Updated: 2026/11/12 15:45:00 by jojo8356         ###   ########.fr       */
/*                                                                          */
/* ************************************************************************** */

# include "../includes/minishell.h"

int is_operator_char(char c)
{
    if (c == '|' || c == '<' || c == '>' || c == '(' || c == ')')
        return (1);
    return (0);
}

int is_quote(char c)
{
    if (c == '\'' || c == '"')
        return (1);
    return (0);
}

token_t *create_token(char *value, int type)
{
    token_t *token;

    token = (token_t *)malloc(sizeof(token_t));
    if (!token)
        return (NULL);
    token->value = value;
    token->type = type;
    token->next = NULL;
    return (token);
}

void append_token(token_t **tokens, token_t *token)
{
    token_t *last;

    if (!*tokens)
    {
        *tokens = token;
        return;
    }
    last = *tokens;
    while (last->next)
        last = last->next;
    last->next = token;
}
