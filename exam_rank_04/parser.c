/* ************************************************************************** */
/*                                                                            */
/*   parser.c                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Exam Rank 04 — microshell : découpage de la ligne de commande.       */
/*   Fonctions autorisées uniquement : malloc, free, strcmp.             */
/* ************************************************************************** */

# include "microshell.h"
# include <stdlib.h>
# include <string.h>

/* reconnaît un séparateur et retourne son type */
static int        is_sep(const char *s, t_sep *out)
{
    if (strcmp(s, "|") == 0)
    {
        *out = SEP_PIPE;
        return (1);
    }
    if (strcmp(s, ";") == 0)
    {
        *out = SEP_SEMI;
        return (1);
    }
    if (strcmp(s, "&&") == 0)
    {
        *out = SEP_AND;
        return (1);
    }
    if (strcmp(s, "||") == 0)
    {
        *out = SEP_OR;
        return (1);
    }
    return (0);
}

/* alloue la table argv d'une commande (capacité fixe, pas de realloc) */
static char        **alloc_argv(void)
{
    return ((char **)malloc(MAX_ARGS * sizeof(char *)));
}

int                parse_command_line(char **argv, t_groups **out)
{
    t_groups        *g;
    char            **cur_argv;    /* argv de la commande en cours (NULL = pas alloué) */
    int             na;            /* nombre d'args de la cmd en cours                */
    int             ng;            /* index du groupe courant                         */
    int             nc;            /* index de la cmd dans le groupe                 */
    int             i;
    t_sep           sep;

    if (!argv || !out)
        return (-1);
    *out = NULL;
    g = (t_groups *)malloc(sizeof(t_groups));
    if (!g)
        return (-1);
    g->groups = (t_group *)malloc(MAX_GROUPS * sizeof(t_group));
    if (!g->groups)
    {
        free(g);
        return (-1);
    }
    for (i = 0; i < MAX_GROUPS; i++)
    {
        g->groups[i].cmds = NULL;
        g->groups[i].ncmds = 0;
        g->groups[i].sep = SEP_NONE;
    }
    ng = 0;
    nc = 0;
    na = 0;
    cur_argv = NULL;
    g->groups[ng].cmds = (t_cmd *)malloc(MAX_CMDS * sizeof(t_cmd));
    if (!g->groups[ng].cmds)
    {
        free(g->groups);
        free(g);
        return (-1);
    }
    g->groups[ng].ncmds = 0;
    g->groups[ng].sep = SEP_NONE;

    for (i = 1; argv[i] != NULL; i++)
    {
        if (is_sep(argv[i], &sep))
        {
            /* valider la commande en cours (si elle a des args) */
            if (cur_argv != NULL)
            {
                cur_argv[na] = NULL;
                g->groups[ng].cmds[nc].argv = cur_argv;
                g->groups[ng].cmds[nc].argc = na;
                nc++;
                g->groups[ng].ncmds = nc;
                cur_argv = NULL;
                na = 0;
            }
            if (sep == SEP_PIPE)
            {
                /* pipeline : on reste dans le même groupe,
                   la cmd suivante allouera son argv laziment */
                if (nc >= MAX_CMDS)
                {
                    groups_free(g);
                    return (-1);
                }
            }
            else
            {
                /* ; && || : on clôture le groupe et on en commence un nouveau */
                g->groups[ng].sep = sep;
                ng++;
                if (ng >= MAX_GROUPS)
                {
                    groups_free(g);
                    return (-1);
                }
                g->groups[ng].cmds = (t_cmd *)malloc(MAX_CMDS * sizeof(t_cmd));
                if (!g->groups[ng].cmds)
                {
                    groups_free(g);
                    return (-1);
                }
                g->groups[ng].ncmds = 0;
                g->groups[ng].sep = SEP_NONE;
                nc = 0;
            }
        }
        else
        {
            /* argument : allouer argv laziment au premier arg */
            if (cur_argv == NULL)
            {
                cur_argv = alloc_argv();
                if (cur_argv == NULL)
                {
                    groups_free(g);
                    return (-1);
                }
            }
            if (na >= MAX_ARGS - 1)
            {
                groups_free(g);
                return (-1);
            }
            cur_argv[na] = argv[i];
            na++;
        }
    }
    /* valider la dernière commande */
    if (cur_argv != NULL)
    {
        cur_argv[na] = NULL;
        g->groups[ng].cmds[nc].argv = cur_argv;
        g->groups[ng].cmds[nc].argc = na;
        nc++;
        g->groups[ng].ncmds = nc;
    }
    g->ngroups = ng + 1;
    *out = g;
    return (0);
}

void            groups_free(t_groups *g)
{
    int            i;
    int            j;

    if (!g)
        return;
    for (i = 0; i < g->ngroups; i++)
    {
        for (j = 0; j < g->groups[i].ncmds; j++)
            free(g->groups[i].cmds[j].argv);
        free(g->groups[i].cmds);
    }
    free(g->groups);
    free(g);
}
