/* ************************************************************************** */
/*                                                                          */
/*                                                        :::      ::::::::   */
/*   2_parser_utils.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/12 15:45:00 by jojo8356          #+#    #+#             */
/*   Updated: 2026/11/12 15:45:00 by jojo8356         ###   ########.fr       */
/*                                                                          */
/* ************************************************************************** */

# include "../includes/minishell.h"

pipe_t *create_pipeline(void)
{
    pipe_t *pipeline;

    pipeline = (pipe_t *)malloc(sizeof(pipe_t));
    if (!pipeline)
        return (NULL);
    pipeline->first = NULL;
    pipeline->logical = LOGICAL_NONE;
    pipeline->next = NULL;
    return (pipeline);
}

cmd_t *create_cmd(void)
{
    cmd_t *cmd;

    cmd = (cmd_t *)malloc(sizeof(cmd_t));
    if (!cmd)
        return (NULL);
    cmd->argv = NULL;
    cmd->redirs = NULL;
    cmd->next = NULL;
    return (cmd);
}

redir_t *create_redir(int type, char *filename)
{
    redir_t *redir;

    redir = (redir_t *)malloc(sizeof(redir_t));
    if (!redir)
        return (NULL);
    redir->type = type;
    redir->filename = filename ? ft_strdup(filename) : NULL;
    redir->fd = -1;
    redir->heredoc_content = NULL;
    redir->next = NULL;
    return (redir);
}

int is_redir_token(int type)
{
    if (type == TOKEN_INPUT || type == TOKEN_OUTPUT ||
        type == TOKEN_APPEND || type == TOKEN_HEREDOC)
        return (1);
    return (0);
}
