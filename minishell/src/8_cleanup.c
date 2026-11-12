/* ************************************************************************** */
/*                                                                          */
/*                                                        :::      ::::::::   */
/*   8_cleanup.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/12 15:45:00 by jojo8356          #+#    #+#             */
/*   Updated: 2026/11/12 15:45:00 by jojo8356         ###   ########.fr       */
/*                                                                          */
/* ************************************************************************** */

# include "../includes/minishell.h"

void free_argv(char **argv)
{
    int i;

    if (!argv)
        return;
    i = 0;
    while (argv[i])
    {
        free(argv[i]);
        i++;
    }
    free(argv);
}

void free_redirs(redir_t *redirs)
{
    redir_t *next;

    while (redirs)
    {
        next = redirs->next;
        if (redirs->filename)
            free(redirs->filename);
        if (redirs->heredoc_content)
            free(redirs->heredoc_content);
        free(redirs);
        redirs = next;
    }
}

void free_cmd(cmd_t *cmd)
{
    cmd_t *next;

    while (cmd)
    {
        next = cmd->next;
        free_argv(cmd->argv);
        free_redirs(cmd->redirs);
        free(cmd);
        cmd = next;
    }
}

void free_pipeline(pipe_t *pipeline)
{
    pipe_t *next;

    while (pipeline)
    {
        next = pipeline->next;
        free_cmd(pipeline->first);
        free(pipeline);
        pipeline = next;
    }
}

void free_tokens(token_t *tokens)
{
    token_t *next;

    while (tokens)
    {
        next = tokens->next;
        if (tokens->value)
            free(tokens->value);
        free(tokens);
        tokens = next;
    }
}
