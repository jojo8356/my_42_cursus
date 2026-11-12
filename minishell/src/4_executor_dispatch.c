/* ************************************************************************** */
/*                                                                          */
/*                                                        :::      ::::::::   */
/*   4_executor_dispatch.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/12 15:45:00 by jojo8356          #+#    #+#             */
/*   Updated: 2026/11/12 15:45:00 by jojo8356         ###   ########.fr       */
/*                                                                          */
/* ************************************************************************** */

# include "../includes/minishell.h"

int execute_pipeline(pipe_t *pipeline)
{
    int exit_code;
    int status;

    exit_code = 0;
    while (pipeline)
    {
        if (count_cmds(pipeline->first) == 1)
            status = run_single_cmd(pipeline->first, 0, 1);
        else
            status = run_pipeline_multi(pipeline->first, 0, 1);
        exit_code = status;
        if (pipeline->logical == LOGICAL_AND && status != 0)
            break;
        if (pipeline->logical == LOGICAL_OR && status == 0)
            break;
        pipeline = pipeline->next;
    }
    set_last_status(exit_code);
    return (exit_code);
}
