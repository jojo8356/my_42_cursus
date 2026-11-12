/* ************************************************************************** */
/*                                                                          */
/*                                                        :::      ::::::::   */
/*   3_expander_main.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/12 15:45:00 by jojo8356          #+#    #+#             */
/*   Updated: 2026/11/12 15:45:00 by jojo8356         ###   ########.fr       */
/*                                                                          */
/* ************************************************************************** */

# include "../includes/minishell.h"

static void expand_cmd_argv(cmd_t *cmd, int exit_code)
{
    int i;
    char *expanded;
    char *no_quotes;

    if (!cmd->argv)
        return;
    i = 0;
    while (cmd->argv[i])
    {
        expanded = expand_word(cmd->argv[i], exit_code);
        if (!expanded)
        {
            i++;
            continue;
        }
        no_quotes = remove_quotes(expanded);
        free(cmd->argv[i]);
        if (no_quotes)
        {
            cmd->argv[i] = no_quotes;
            free(expanded);
        }
        else
            cmd->argv[i] = expanded;
        i++;
    }
}

static void expand_cmd_redirs(cmd_t *cmd, int exit_code)
{
    redir_t *redir;
    char *expanded;
    char *no_quotes;

    redir = cmd->redirs;
    while (redir)
    {
        if (redir->filename)
        {
            expanded = expand_word(redir->filename, exit_code);
            no_quotes = remove_quotes(expanded);
            free(redir->filename);
            if (no_quotes)
            {
                redir->filename = no_quotes;
                free(expanded);
            }
            else
                redir->filename = expanded;
        }
        redir = redir->next;
    }
}

void expand_pipeline(pipe_t *pipeline, int exit_code)
{
    cmd_t *cmd;

    while (pipeline)
    {
        cmd = pipeline->first;
        while (cmd)
        {
            expand_cmd_argv(cmd, exit_code);
            expand_cmd_redirs(cmd, exit_code);
            if (cmd->argv)
            {
                char **expanded = expand_wildcards(cmd->argv);
                if (expanded)
                {
                    free_argv(cmd->argv);
                    cmd->argv = expanded;
                }
            }
            cmd = cmd->next;
        }
        pipeline = pipeline->next;
    }
}
