/* ************************************************************************** */
/*                                                                          */
/*                                                        :::      ::::::::   */
/*   4_executor_main.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/12 15:45:00 by jojo8356          #+#    #+#             */
/*   Updated: 2026/11/12 15:45:00 by jojo8356         ###   ########.fr       */
/*                                                                          */
/* ************************************************************************** */

# include "../includes/minishell.h"

extern char **environ;

int count_cmds(cmd_t *cmd)
{
    int count;

    count = 0;
    while (cmd)
    {
        count++;
        cmd = cmd->next;
    }
    return (count);
}

int run_single_cmd(cmd_t *cmd, int fd_in, int fd_out)
{
    int code;
    int old_in;
    int old_out;

    if (is_builtin(cmd->argv[0]))
    {
        old_in = dup(0);
        old_out = dup(1);
        if (fd_in != 0)
            dup2(fd_in, 0);
        if (fd_out != 1)
            dup2(fd_out, 1);
        apply_redirections(cmd);
        code = exec_builtin(cmd, &code);
        dup2(old_in, 0);
        dup2(old_out, 1);
        close(old_in);
        close(old_out);
        return (code);
    }
    else
    {
        pid_t pid;
        int status;

        pid = fork();
        if (pid == 0)
        {
            dup2(fd_in, 0);
            dup2(fd_out, 1);
            apply_redirections(cmd);
            execute_external(cmd);
            exit(127);
        }
        waitpid(pid, &status, 0);
        return (WEXITSTATUS(status));
    }
}

int run_pipeline_multi(cmd_t *first, int fd_in, int fd_out)
{
    int pipefd[2];
    int prev_fd;
    pid_t last_pid;
    int status;
    cmd_t *cmd;

    prev_fd = fd_in;
    cmd = first;
    while (cmd)
    {
        if (cmd->next)
            pipe(pipefd);
        else
            pipefd[1] = fd_out;
        pid_t pid = fork();
        if (pid == 0)
        {
            dup2(prev_fd, 0);
            dup2(pipefd[1], 1);
            if (prev_fd > 2)
                close(prev_fd);
            if (pipefd[1] > 2)
                close(pipefd[1]);
            if (pipefd[0] > 2 && cmd->next)
                close(pipefd[0]);
            apply_redirections(cmd);
            if (is_builtin(cmd->argv[0]))
            {
                int code = 0;
                exec_builtin(cmd, &code);
                exit(code);
            }
            else
            {
                execute_external(cmd);
                exit(127);
            }
        }
        if (cmd->next)
        {
            if (prev_fd > 2)
                close(prev_fd);
            close(pipefd[1]);
            prev_fd = pipefd[0];
        }
        else
        {
            if (prev_fd > 2)
                close(prev_fd);
        }
        last_pid = pid;
        cmd = cmd->next;
    }
    waitpid(last_pid, &status, 0);
    return (WEXITSTATUS(status));
}
