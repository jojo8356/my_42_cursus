/* ************************************************************************** */
/*                                                                          */
/*                                                        :::      ::::::::   */
/*   0_main.c                                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/12 15:45:00 by jojo8356          #+#    #+#             */
/*   Updated: 2026/11/12 15:45:00 by jojo8356         ###   ########.fr       */
/*                                                                          */
/* ************************************************************************** */

# include "../includes/minishell.h"

char *get_prompt(void)
{
    return ("minishell> ");
}

static void handle_signal(void)
{
    if (g_signal == SIGINT)
    {
        rl_on_new_line();
        g_signal = 0;
    }
    else if (g_signal == SIGQUIT)
    {
        g_signal = 0;
    }
}

static void collect_heredoc_content(pipe_t *pipeline)
{
    cmd_t *cmd;
    redir_t *redir;
    char *line;
    char *content;
    int len;

    cmd = pipeline->first;
    while (cmd)
    {
        redir = cmd->redirs;
        while (redir)
        {
            if (redir->type == REDIR_HEREDOC && redir->filename)
            {
                content = NULL;
                len = 0;
                while (1)
                {
                    line = readline("> ");
                    if (!line)
                        break;
                    if (ft_strcmp(line, redir->filename) == 0)
                    {
                        free(line);
                        break;
                    }
                    if (content)
                    {
                        len += ft_strlen(line) + 1;
                        content = (char *)realloc(content, sizeof(char) * (len + 1));
                        ft_strlcat(content, line, len + 1);
                        content[len - 1] = '\n';
                    }
                    else
                    {
                        len = ft_strlen(line) + 1;
                        content = (char *)malloc(sizeof(char) * (len + 1));
                        ft_strlcpy(content, line, len);
                        content[len - 1] = '\n';
                    }
                    free(line);
                }
                if (content)
                    redir->heredoc_content = content;
                redir->filename = NULL;
            }
            redir = redir->next;
        }
        cmd = cmd->next;
    }
}

static void collect_all_heredocs(pipe_t *pipeline)
{
    while (pipeline)
    {
        collect_heredoc_content(pipeline);
        pipeline = pipeline->next;
    }
}

static int process_line(char *line, int exit_code)
{
    token_t *tokens;
    pipe_t *pipeline;
    int status;

    handle_signal();
    if (!line || line[0] == 0)
        return (exit_code);
    tokens = tokenize_input(line);
    pipeline = parse_tokens(tokens);
    free_tokens(tokens);
    if (!pipeline)
        return (exit_code);
    collect_all_heredocs(pipeline);
    expand_pipeline(pipeline, exit_code);
    status = execute_pipeline(pipeline);
    free_pipeline(pipeline);
    if (should_exit())
        return (get_exit_code());
    return (status);
}

int run_shell(void)
{
    char *line;
    int exit_code;

    exit_code = 0;
    while (1)
    {
        setup_signals();
        line = readline(get_prompt());
        reset_signals();
        if (!line)
        {
            write(1, "exit\n", 5);
            break;
        }
        if (line[0])
            add_history(line);
        exit_code = process_line(line, exit_code);
        free(line);
        if (should_exit())
            break;
    }
    rl_clear_history();
    return (exit_code);
}

int main(int ac, char **av)
{
    (void)ac;
    (void)av;
    return (run_shell());
}
