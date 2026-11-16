/* ************************************************************************** */
/*                                                                          */
/*                                                        :::      ::::::::   */
/*   6_redirections.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/12 15:45:00 by jojo8356          #+#    #+#             */
/*   Updated: 2026/11/12 15:45:00 by jojo8356         ###   ########.fr       */
/*                                                                          */
/* ************************************************************************** */

# include "../includes/minishell.h"

int apply_input_redir(redir_t *redir)
{
    int fd;

    fd = open(redir->filename, O_RDONLY);
    if (fd == -1)
    {
        ft_putstr_fd("minishell: ", 2);
        ft_putstr_fd(redir->filename, 2);
        ft_putendl_fd(": No such file or directory", 2);
        return (-1);
    }
    dup2(fd, STDIN_FILENO);
    close(fd);
    return (0);
}

int apply_output_redir(redir_t *redir)
{
    int fd;
    int flags;

    flags = O_WRONLY | O_CREAT | O_TRUNC;
    if (redir->type == REDIR_APPEND)
        flags = O_WRONLY | O_CREAT | O_APPEND;
    fd = open(redir->filename, flags, 0644);
    if (fd == -1)
    {
        ft_putstr_fd("minishell: ", 2);
        ft_putstr_fd(redir->filename, 2);
        ft_putendl_fd(": Permission denied", 2);
        return (-1);
    }
    dup2(fd, STDOUT_FILENO);
    close(fd);
    return (0);
}

int apply_heredoc_redir(redir_t *redir)
{
    int pipefd[2];
    char *content;
    int len;

    if (!redir->heredoc_content)
        return (0);
    pipe(pipefd);
    content = redir->heredoc_content;
    len = ft_strlen(content);
    write(pipefd[1], content, len);
    close(pipefd[1]);
    dup2(pipefd[0], STDIN_FILENO);
    close(pipefd[0]);
    return (0);
}

int apply_redirections(cmd_t *cmd)
{
    redir_t *redir;

    redir = cmd->redirs;
    while (redir)
    {
        if (redir->type == REDIR_INPUT)
        {
            if (apply_input_redir(redir) == -1)
                return (-1);
        }
        else if (redir->type == REDIR_OUTPUT || redir->type == REDIR_APPEND)
        {
            if (apply_output_redir(redir) == -1)
                return (-1);
        }
        else if (redir->type == REDIR_HEREDOC)
        {
            apply_heredoc_redir(redir);
        }
        redir = redir->next;
    }
    return (0);
}
