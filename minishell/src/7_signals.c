/* ************************************************************************** */
/*                                                                          */
/*                                                        :::      ::::::::   */
/*   7_signals.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/12 15:45:00 by jojo8356          #+#    #+#             */
/*   Updated: 2026/11/12 15:45:00 by jojo8356         ###   ########.fr       */
/*                                                                          */
/* ************************************************************************** */

# include "../includes/minishell.h"

volatile sig_atomic_t g_signal = 0;

void signal_handler(int sig)
{
    g_signal = sig;
}

static void setup_sigint(void)
{
    struct sigaction sa;

    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);
}

static void setup_sigquit(void)
{
    struct sigaction sa;

    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGQUIT, &sa, NULL);
}

static void setup_sigdfl(void)
{
    signal(SIGINT, SIG_DFL);
    signal(SIGQUIT, SIG_DFL);
}

void setup_signals(void)
{
    setup_sigint();
    setup_sigquit();
}

void reset_signals(void)
{
    setup_sigdfl();
}
