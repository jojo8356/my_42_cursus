/* ************************************************************************** */
/*                                                                          */
/*                                                        :::      ::::::::   */
/*   readline_mock.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/12 15:45:00 by jojo8356          #+#    #+#             */
/*   Updated: 2026/11/12 15:45:00 by jojo8356         ###   ########.fr       */
/*                                                                          */
/* ************************************************************************** */

# include "readline_mock.h"

static char *g_history[MOCK_RL_MAX_HISTORY];
static int g_history_count = 0;
static int g_history_index = 0;

char *readline(const char *prompt)
{
    char *buf = NULL;
    size_t buf_size = 0;
    ssize_t len;

    if (prompt)
        write(1, prompt, strlen(prompt));
    len = getline(&buf, &buf_size, stdin);
    if (len <= 0)
    {
        free(buf);
        return NULL;
    }
    if (buf[len - 1] == '\n')
        buf[len - 1] = 0;
    return buf;
}

void add_history(char *line)
{
    if (!line || g_history_count >= MOCK_RL_MAX_HISTORY)
        return;
    g_history[g_history_count] = strdup(line);
    g_history_count++;
}

void rl_clear_history(void)
{
    int i;

    i = 0;
    while (i < g_history_count)
    {
        free(g_history[i]);
        i++;
    }
    g_history_count = 0;
    g_history_index = 0;
}

void rl_on_new_line(void)
{
    write(1, "\n", 1);
}

void rl_replace_line(char *line, int len)
{
    (void)line;
    (void)len;
}

void rl_redisplay(void)
{
}

int mock_get_history_count(void)
{
    return (g_history_count);
}

char *mock_get_history_line(int index)
{
    if (index < 0 || index >= g_history_count)
        return (NULL);
    return (g_history[index]);
}
