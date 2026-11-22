/* ************************************************************************** */
/*                                                                            */
/*   test_parser.c                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Tests : parseur de la ligne de commande (| ; && ||).                */
/* ************************************************************************** */

# include "test_framework.h"
# include "microshell.h"
# include <string.h>

TEST(test_parse_simple_command)
{
    char            *argv[] = {"microshell", "/bin/echo", "hello", NULL};
    t_groups        *g;

    ASSERT_EQ_INT(parse_command_line(argv, &g), 0, "parsé");
    ASSERT_EQ_INT(g->ngroups, 1, "1 groupe");
    ASSERT_EQ_INT(g->groups[0].ncmds, 1, "1 commande");
    ASSERT_EQ_INT(g->groups[0].sep, SEP_NONE, "sep NONE");
    ASSERT_EQ_INT(g->groups[0].cmds[0].argc, 2, "2 args");
    ASSERT(strcmp(g->groups[0].cmds[0].argv[0], "/bin/echo") == 0, "argv0");
    ASSERT(strcmp(g->groups[0].cmds[0].argv[1], "hello") == 0, "argv1");
    ASSERT(g->groups[0].cmds[0].argv[2] == NULL, "argv NULL-terminé");
    groups_free(g);
}

TEST(test_parse_semi)
{
    char            *argv[] = {"m", "/bin/echo", "a", ";", "/bin/echo", "b", NULL};
    t_groups        *g;
    ASSERT_EQ_INT(parse_command_line(argv, &g), 0, "parsé");
    ASSERT_EQ_INT(g->ngroups, 2, "2 groupes");
    ASSERT_EQ_INT(g->groups[0].sep, SEP_SEMI, "sep SEMI grp0");
    ASSERT_EQ_INT(g->groups[1].sep, SEP_NONE, "sep NONE grp1");
    ASSERT_EQ_INT(g->groups[0].cmds[0].argc, 2, "grp0 cmd argc");
    ASSERT_EQ_INT(g->groups[1].cmds[0].argc, 2, "grp1 cmd argc");
    groups_free(g);
}

TEST(test_parse_pipe)
{
    char            *argv[] = {"m", "/bin/echo", "x", "|", "/bin/cat", NULL};
    t_groups        *g;
    ASSERT_EQ_INT(parse_command_line(argv, &g), 0, "parsé");
    ASSERT_EQ_INT(g->ngroups, 1, "1 groupe (pipeline)");
    ASSERT_EQ_INT(g->groups[0].ncmds, 2, "2 commandes");
    ASSERT_EQ_INT(g->groups[0].sep, SEP_NONE, "sep NONE");
    ASSERT_EQ_INT(g->groups[0].cmds[0].argc, 2, "cmd0 argc");
    ASSERT_EQ_INT(g->groups[0].cmds[1].argc, 1, "cmd1 argc");
    groups_free(g);
}

TEST(test_parse_pipe_then_semi)
{
    char            *argv[] = {"m", "/bin/echo", "x", "|", "/bin/cat", ";",
        "/bin/echo", "y", NULL};
    t_groups        *g;
    ASSERT_EQ_INT(parse_command_line(argv, &g), 0, "parsé");
    ASSERT_EQ_INT(g->ngroups, 2, "2 groupes");
    ASSERT_EQ_INT(g->groups[0].ncmds, 2, "grp0 = pipeline 2 cmds");
    ASSERT_EQ_INT(g->groups[0].sep, SEP_SEMI, "grp0 sep SEMI");
    ASSERT_EQ_INT(g->groups[1].ncmds, 1, "grp1 = 1 cmd");
    ASSERT_EQ_INT(g->groups[1].sep, SEP_NONE, "grp1 sep NONE");
    groups_free(g);
}

TEST(test_parse_and)
{
    char            *argv[] = {"m", "/bin/true", "&&", "/bin/echo", "ok", NULL};
    t_groups        *g;
    ASSERT_EQ_INT(parse_command_line(argv, &g), 0, "parsé");
    ASSERT_EQ_INT(g->ngroups, 2, "2 groupes");
    ASSERT_EQ_INT(g->groups[0].sep, SEP_AND, "sep AND grp0");
    ASSERT_EQ_INT(g->groups[1].sep, SEP_NONE, "sep NONE grp1");
    groups_free(g);
}

TEST(test_parse_or)
{
    char            *argv[] = {"m", "/bin/false", "||", "/bin/echo", "ok", NULL};
    t_groups        *g;
    ASSERT_EQ_INT(parse_command_line(argv, &g), 0, "parsé");
    ASSERT_EQ_INT(g->ngroups, 2, "2 groupes");
    ASSERT_EQ_INT(g->groups[0].sep, SEP_OR, "sep OR grp0");
    groups_free(g);
}

TEST(test_parse_multiple_args)
{
    char            *argv[] = {"m", "/bin/echo", "a", "b", "c", "d", NULL};
    t_groups        *g;
    ASSERT_EQ_INT(parse_command_line(argv, &g), 0, "parsé");
    ASSERT_EQ_INT(g->groups[0].cmds[0].argc, 5, "5 args");
    ASSERT(strcmp(g->groups[0].cmds[0].argv[4], "d") == 0, "argv4");
    groups_free(g);
}

TEST(test_parse_cd_detection)
{
    char            *argv[] = {"m", "cd", "/tmp", NULL};
    t_groups        *g;
    ASSERT_EQ_INT(parse_command_line(argv, &g), 0, "parsé");
    ASSERT_EQ_INT(g->groups[0].cmds[0].argc, 2, "cd + chemin");
    ASSERT(strcmp(g->groups[0].cmds[0].argv[0], "cd") == 0, "c'est cd");
    groups_free(g);
}

TEST(test_parse_chain_and_or)
{
    /* false && echo a || echo b */
    char            *argv[] = {"m", "/bin/false", "&&", "/bin/echo", "a",
        "||", "/bin/echo", "b", NULL};
    t_groups        *g;
    ASSERT_EQ_INT(parse_command_line(argv, &g), 0, "parsé");
    ASSERT_EQ_INT(g->ngroups, 3, "3 groupes");
    ASSERT_EQ_INT(g->groups[0].sep, SEP_AND, "sep AND");
    ASSERT_EQ_INT(g->groups[1].sep, SEP_OR, "sep OR");
    ASSERT_EQ_INT(g->groups[2].sep, SEP_NONE, "sep NONE");
    groups_free(g);
}

TEST(test_parse_long_pipeline)
{
    /* 10 commandes en pipeline */
    char            *argv[] = {"m", "/bin/echo", "x",
        "|", "/bin/cat", "|", "/bin/cat", "|", "/bin/cat", "|", "/bin/cat",
        "|", "/bin/cat", "|", "/bin/cat", "|", "/bin/cat", "|", "/bin/cat",
        "|", "/bin/cat", NULL};
    t_groups        *g;
    ASSERT_EQ_INT(parse_command_line(argv, &g), 0, "parsé");
    ASSERT_EQ_INT(g->ngroups, 1, "1 groupe");
    ASSERT_EQ_INT(g->groups[0].ncmds, 10, "10 commandes");
    groups_free(g);
}

TEST(test_groups_free_null)
{
    groups_free(NULL);
    ASSERT(1, "groups_free(NULL) ne crashe pas");
}

int            main(void)
{
    RUN(test_parse_simple_command);
    RUN(test_parse_semi);
    RUN(test_parse_pipe);
    RUN(test_parse_pipe_then_semi);
    RUN(test_parse_and);
    RUN(test_parse_or);
    RUN(test_parse_multiple_args);
    RUN(test_parse_cd_detection);
    RUN(test_parse_chain_and_or);
    RUN(test_parse_long_pipeline);
    RUN(test_groups_free_null);
    SUMMARY();
}
