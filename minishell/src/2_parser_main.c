/* ************************************************************************** */
/*                                                                          */
/*                                                        :::      ::::::::   */
/*   2_parser_main.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/12 15:45:00 by jojo8356          #+#    #+#             */
/*   Updated: 2026/11/12 15:45:00 by jojo8356         ###   ########.fr       */
/*                                                                          */
/* ************************************************************************** */

# include "../includes/minishell.h"

static int token_to_redir_type(int token_type)
{
    if (token_type == TOKEN_INPUT)
        return (REDIR_INPUT);
    if (token_type == TOKEN_OUTPUT)
        return (REDIR_OUTPUT);
    if (token_type == TOKEN_APPEND)
        return (REDIR_APPEND);
    if (token_type == TOKEN_HEREDOC)
        return (REDIR_HEREDOC);
    return (-1);
}

static void handle_redir_token(cmd_t *cmd, token_t **tokens)
{
    redir_t *redir;
    token_t *next;

    next = (*tokens)->next;
    if (next && next->type == TOKEN_WORD && next->value)
    {
        redir = create_redir(token_to_redir_type((*tokens)->type), next->value);
        if (redir)
            redirs_append(&cmd->redirs, redir);
        *tokens = next;
    }
}

static void handle_pipe_token(cmd_t **cmd)
{
    (*cmd)->next = create_cmd();
    *cmd = (*cmd)->next;
}

static void handle_logical_token(pipe_t **pipeline, cmd_t **cmd, int type)
{
    (*pipeline)->logical = (type == TOKEN_AND) ? LOGICAL_AND : LOGICAL_OR;
    (*pipeline)->next = create_pipeline();
    *pipeline = (*pipeline)->next;
    (*pipeline)->first = create_cmd();
    *cmd = (*pipeline)->first;
}

pipe_t *parse_tokens(token_t *tokens)
{
    pipe_t *pipelines;
    pipe_t *current_pipeline;
    cmd_t *current_cmd;
    token_t *current;

    if (!tokens || tokens->type == TOKEN_END)
        return (NULL);
    pipelines = create_pipeline();
    if (!pipelines)
        return (NULL);
    pipelines->first = create_cmd();
    if (!pipelines->first)
    {
        free(pipelines);
        return (NULL);
    }
    current_pipeline = pipelines;
    current_cmd = pipelines->first;
    current = tokens;
    while (current && current->type != TOKEN_END)
    {
        if (current->type == TOKEN_PIPE)
        {
            handle_pipe_token(&current_cmd);
            current = current->next;
        }
        else if (current->type == TOKEN_AND)
        {
            handle_logical_token(&current_pipeline, &current_cmd, TOKEN_AND);
            current = current->next;
        }
        else if (current->type == TOKEN_OR)
        {
            handle_logical_token(&current_pipeline, &current_cmd, TOKEN_OR);
            current = current->next;
        }
        else if (current->type == TOKEN_LPAREN ||
                 current->type == TOKEN_RPAREN)
            current = current->next;
        else if (is_redir_token(current->type))
        {
            handle_redir_token(current_cmd, &current);
            current = current->next;
        }
        else if (current->type == TOKEN_WORD && current->value)
        {
            argv_append(&current_cmd->argv, ft_strdup(current->value));
            current = current->next;
        }
        else
            current = current->next;
    }
    return (pipelines);
}
