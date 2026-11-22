/* ************************************************************************** */
/*                                                                            */
/*   microshell.c                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Exam Rank 04 — microshell : exécution, pipelines, cd, bonus &&/||.   */
/*   Fonctions autorisées : malloc, free, write, close, fork, waitpid,    */
/*   exit, chdir, execve, dup2, pipe, strcmp (+ variable environ).        */
/* ************************************************************************** */

# include "microshell.h"
# include <stdlib.h>
# include <string.h>
# include <unistd.h>
# include <sys/wait.h>

extern char        **environ;

/* ---- helpers string (sans strlen/strtok — non autorisés) ---- */

static int        my_strlen(const char *s)
{
    int            n = 0;

    while (s[n] != '\0')
        n++;
    return (n);
}

static void        write_err(const char *s)
{
    if (write(2, s, my_strlen(s)) == -1)
    {
        write(2, "error: fatal\n", 14);
        exit(1);
    }
}

/* erreur fatale : système d'appel (sauf execve/chdir) a échoué */
static void        fatal(void)
{
    write(2, "error: fatal\n", 14);
    exit(1);
}

/* execve a échoué (lancé dans le fils) */
static void        cannot_execute(const char *path)
{
    write(2, "error: cannot execute ", 22);
    write(2, path, my_strlen(path));
    write(2, "\n", 1);
    exit(127);
}

/* ---- builtin cd ---- */

static int        execute_cd(const t_cmd *cmd)
{
    /* cd attend exactement 1 argument (le chemin) */
    if (cmd->argc != 2)
    {
        write_err("error: cd: bad arguments\n");
        return (1);
    }
    if (chdir(cmd->argv[1]) == -1)
    {
        write_err("error: cd: cannot change directory to ");
        write_err(cmd->argv[1]);
        write_err("\n");
        return (1);
    }
    return (0);
}

/* ---- exécution d'un pipeline (concurrent, faibles descripteurs) ---- */

/* Exécute les ncmds commandes du groupe en pipeline.
   Un seul pipe est ouvert à la fois dans le parent -> gère des centaines
   de "|" sans épuiser les descripteurs. Retourne le status du dernier fils. */
static int        execute_pipeline(const t_group *grp)
{
    const int        n = grp->ncmds;
    int            prev_read = -1;    /* extrémité lecture du pipe précédent */
    int            *pids;
    int            last_status = 0;
    int            i;

    pids = (int *)malloc(n * sizeof(int));
    if (pids == NULL)
        fatal();
    for (i = 0; i < n; i++)
    {
        int            pfd[2];
        int            pid;

        if (i < n - 1)
        {
            if (pipe(pfd) == -1)
                fatal();
        }
        pid = fork();
        if (pid == -1)
            fatal();
        if (pid == 0)
        {
            /* ---- fils ---- */
            /* stdin : depuis le pipe précédent (sauf pour la 1ère cmd) */
            if (prev_read >= 0)
            {
                if (dup2(prev_read, 0) == -1)
                    fatal();
                if (close(prev_read) == -1)
                    fatal();
            }
            /* stdout : vers le pipe suivant (sauf pour la dernière cmd) */
            if (i < n - 1)
            {
                if (dup2(pfd[1], 1) == -1)
                    fatal();
                if (close(pfd[0]) == -1)
                    fatal();
                if (close(pfd[1]) == -1)
                    fatal();
            }
            /* exécuter */
            if (execve(grp->cmds[i].argv[0], grp->cmds[i].argv, environ) == -1)
                cannot_execute(grp->cmds[i].argv[0]);
            exit(127);    /* inaccessible : execve ne revient pas en succès */
        }
        /* ---- parent ---- */
        if (prev_read >= 0)
        {
            if (close(prev_read) == -1)
                fatal();
            prev_read = -1;
        }
        if (i < n - 1)
        {
            if (close(pfd[1]) == -1)
                fatal();
            prev_read = pfd[0];
        }
        pids[i] = pid;
    }
    if (prev_read >= 0)
    {
        if (close(prev_read) == -1)
            fatal();
    }
    /* attendre tous les fils (le dernier détermine le status du groupe) */
    for (i = 0; i < n; i++)
    {
        int            status;

        if (waitpid(pids[i], &status, 0) == -1)
            fatal();
        if (WIFEXITED(status))
            last_status = WEXITSTATUS(status);
        else if (WIFSIGNALED(status))
            last_status = 128 + WTERMSIG(status);
    }
    free(pids);
    return (last_status);
}

/* ---- exécution d'un groupe ---- */

static int        execute_group(const t_group *grp)
{
    /* cd est un builtin, jamais dans un pipeline (énoncé) */
    if (grp->ncmds == 1 && grp->cmds[0].argc >= 1
        && strcmp(grp->cmds[0].argv[0], "cd") == 0)
        return (execute_cd(&grp->cmds[0]));
    return (execute_pipeline(grp));
}

/* ---- exécution de tous les groupes (;, &&, || avec short-circuit) ---- */

int                execute_groups(t_groups *g)
{
    int            i;
    int            last_status = 0;

    for (i = 0; i < g->ngroups; i++)
    {
        int            should_run = 1;

        /* le séparateur du groupe précédent décide si on exécute */
        if (i > 0)
        {
            t_sep        ps = g->groups[i - 1].sep;

            if (ps == SEP_AND && last_status != 0)
                should_run = 0;
            if (ps == SEP_OR && last_status == 0)
                should_run = 0;
        }
        if (should_run)
            last_status = execute_group(&g->groups[i]);
    }
    return (last_status);
}

/* ---- point d'entrée ---- */

int                main(int argc, char **argv)
{
    t_groups        *g;
    int             status;

    if (argc < 2)
        return (0);
    if (parse_command_line(argv, &g) == -1)
    {
        write_err("error: fatal\n");
        return (1);
    }
    status = execute_groups(g);
    groups_free(g);
    return (status);
}
