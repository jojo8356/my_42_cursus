/* ************************************************************************** */
/*                                                                            */
/*   microshell.h                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Exam Rank 04 — microshell : structure du parsing.                    */
/* ************************************************************************** */

#ifndef MICROSHELL_H
# define MICROSHELL_H

/* capacités maximales (pas de realloc autorisé : allocations fixes) */
# define MAX_ARGS   1024
# define MAX_CMDS   1024
# define MAX_GROUPS 1024

/* séparateurs reconnus sur la ligne de commande */
typedef enum
{
    SEP_NONE = 0,    /* fin de la ligne                                   */
    SEP_PIPE,        /* |  (pipeline)                                     */
    SEP_SEMI,        /* ;  (séquence)                                      */
    SEP_AND,         /* && (bonus : et logique, short-circuit)           */
    SEP_OR           /* || (bonus : ou logique, short-circuit)           */
}                   t_sep;

/* une commande : argv NULL-terminé */
typedef struct s_cmd
{
    char            **argv;
    int             argc;
}                   t_cmd;

/* un groupe = un pipeline (commandes séparées par |)
   `sep` = séparateur qui suit le groupe (;, &&, || ou NONE) */
typedef struct s_group
{
    t_cmd           *cmds;
    int             ncmds;
    t_sep           sep;
}                   t_group;

/* liste des groupes (un pipeline par groupe) */
typedef struct s_groups
{
    t_group         *groups;
    int             ngroups;
}                   t_groups;

/* ---- parsing ---- */
int                parse_command_line(char **argv, t_groups **out);
void               groups_free(t_groups *g);

/* ---- exécution ---- */
int                execute_groups(t_groups *g);

#endif
