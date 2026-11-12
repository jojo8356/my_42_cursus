/* ************************************************************************** */
/*                                                                          */
/*                                                        :::      ::::::::   */
/*   1_tokenizer_op.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/12 15:45:00 by jojo8356          #+#    #+#             */
/*   Updated: 2026/11/12 15:45:00 by jojo8356         ###   ########.fr       */
/*                                                                          */
/* ************************************************************************** */

# include "../includes/minishell.h"

static token_t *read_and_or(char *input, int *i)
{
    if (input[*i] == '|' && input[*i + 1] == '|')
    {
        *i = *i + 2;
        return (create_token(ft_strdup("||"), TOKEN_OR));
    }
    if (input[*i] == '&' && input[*i + 1] == '&')
    {
        *i = *i + 2;
        return (create_token(ft_strdup("&&"), TOKEN_AND));
    }
    return (NULL);
}

static token_t *read_redirect(char *input, int *i)
{
    if (input[*i] == '<' && input[*i + 1] == '<')
    {
        *i = *i + 2;
        return (create_token(ft_strdup("<<"), TOKEN_HEREDOC));
    }
    if (input[*i] == '>' && input[*i + 1] == '>')
    {
        *i = *i + 2;
        return (create_token(ft_strdup(">>"), TOKEN_APPEND));
    }
    if (input[*i] == '<')
    {
        (*i)++;
        return (create_token(ft_strdup("<"), TOKEN_INPUT));
    }
    if (input[*i] == '>')
    {
        (*i)++;
        return (create_token(ft_strdup(">"), TOKEN_OUTPUT));
    }
    return (NULL);
}

token_t *read_operator(char *input, int *i)
{
    token_t *token;

    token = read_and_or(input, i);
    if (token)
        return (token);
    token = read_redirect(input, i);
    if (token)
        return (token);
    if (input[*i] == '|')
    {
        (*i)++;
        return (create_token(ft_strdup("|"), TOKEN_PIPE));
    }
    if (input[*i] == '(')
    {
        (*i)++;
        return (create_token(ft_strdup("("), TOKEN_LPAREN));
    }
    if (input[*i] == ')')
    {
        (*i)++;
        return (create_token(ft_strdup(")"), TOKEN_RPAREN));
    }
    return (NULL);
}
