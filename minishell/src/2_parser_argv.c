/* ************************************************************************** */
/*                                                                          */
/*                                                        :::      ::::::::   */
/*   2_parser_argv.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/12 15:45:00 by jojo8356          #+#    #+#             */
/*   Updated: 2026/11/12 15:45:00 by jojo8356         ###   ########.fr       */
/*                                                                          */
/* ************************************************************************** */

# include "../includes/minishell.h"

int argv_count(char **argv)
{
    int count;

    count = 0;
    while (argv && argv[count])
        count++;
    return (count);
}

void argv_append(char ***argv, char *arg)
{
    char **new_argv;
    int count;
    int i;

    count = argv_count(*argv);
    new_argv = (char **)malloc(sizeof(char *) * (count + 2));
    if (!new_argv)
        return;
    i = 0;
    while (i < count)
    {
        new_argv[i] = (*argv)[i];
        i++;
    }
    new_argv[count] = arg;
    new_argv[count + 1] = NULL;
    free(*argv);
    *argv = new_argv;
}

void redirs_append(redir_t **redirs, redir_t *redir)
{
    redir_t *last;

    if (!*redirs)
    {
        *redirs = redir;
        return;
    }
    last = *redirs;
    while (last->next)
        last = last->next;
    last->next = redir;
}
