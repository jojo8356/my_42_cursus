/* ************************************************************************** */
/*                                                                          */
/*                                                        :::      ::::::::   */
/*   shell_state.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/12 15:45:00 by jojo8356          #+#    #+#             */
/*   Updated: 2026/11/12 15:45:00 by jojo8356         ###   ########.fr       */
/*                                                                          */
/* ************************************************************************** */

# include "../includes/minishell.h"

static int g_should_exit = 0;
static int g_exit_code = 0;
static int g_last_status = 0;

void request_exit(int code)
{
    g_should_exit = 1;
    g_exit_code = code;
}

int should_exit(void)
{
    return (g_should_exit);
}

int get_exit_code(void)
{
    return (g_exit_code);
}

void set_last_status(int status)
{
    g_last_status = status;
}

int get_last_status(void)
{
    return (g_last_status);
}
