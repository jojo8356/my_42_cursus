/* ************************************************************************** */
/*                                                                          */
/*                                                        :::      ::::::::   */
/*   1_tokenizer_main.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/12 15:45:00 by jojo8356          #+#    #+#             */
/*   Updated: 2026/11/12 15:45:00 by jojo8356         ###   ########.fr       */
/*                                                                          */
/* ************************************************************************** */

# include "../includes/minishell.h"

token_t *tokenize_input(char *input)
{
    token_t *tokens = NULL;
    token_t *token;
    int i = 0;

    while (input[i])
    {
        while (input[i] == ' ' || input[i] == '\t')
            i++;
        if (input[i] == 0)
            break;
        if (is_operator_char(input[i]) ||
            (input[i] == '&' && input[i + 1] == '&'))
        {
            token = read_operator(input, &i);
            if (token)
                append_token(&tokens, token);
            else
            {
                token = create_token(ft_strdup("&"), TOKEN_WORD);
                i++;
                append_token(&tokens, token);
            }
        }
        else
        {
            token = create_token(read_word(input, &i), TOKEN_WORD);
            append_token(&tokens, token);
        }
    }
    append_token(&tokens, create_token(NULL, TOKEN_END));
    return (tokens);
}
