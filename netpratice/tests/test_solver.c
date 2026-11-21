/* ************************************************************************** */
/*                                                                            */
/*   test_solver.c                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Tests : solveur sur diverses topologies (chaîne, étoile, multi).    */
/* ************************************************************************** */

# include "test_framework.h"

static int        solve_and_validate(const char *topo)
{
    t_level            lvl;
    t_solution        sol;
    t_topo            topo_s;
    char            *err = NULL;
    int            ret;

    if (level_parse_str(topo, &lvl) != 0)
        return (-1);
    ft_bzero(&topo_s, sizeof(topo_s));
    if (solver_solve(&lvl, &topo_s, &sol) != 0)
    {
        level_free(&lvl);
        return (-1);
    }
    ret = solution_validate_goals(&sol, &lvl, &err);
    if (err)
        fprintf(stderr, "    err: %s\n", err);
    ft_free(err);
    solution_free(&sol);
    level_free(&lvl);
    return (ret);
}

TEST(test_two_hosts)
{
    const char        *t =
    "{\"name\":\"t\",\"devices\":[{\"name\":\"A\",\"ifs\":[\"A1\"]},{\"name\":\"B\",\"ifs\":[\"B1\"]}],"
    "\"links\":[[\"A1\",\"B1\"]],\"goals\":[{\"src\":\"A\",\"dst\":\"B\"}]}";
    ASSERT_EQ_INT(solve_and_validate(t), 0, "2 hôtes");
}

TEST(test_chain_3_routers)
{
    /* A - R1 - R2 - R3 - B */
    const char        *t =
    "{\"name\":\"c3\",\"devices\":["
    "{\"name\":\"A\",\"ifs\":[\"A1\"]},"
    "{\"name\":\"R1\",\"ifs\":[\"R1a\",\"R1b\"],\"rt\":\"R1\"},"
    "{\"name\":\"R2\",\"ifs\":[\"R2a\",\"R2b\"],\"rt\":\"R2\"},"
    "{\"name\":\"R3\",\"ifs\":[\"R3a\",\"R3b\"],\"rt\":\"R3\"},"
    "{\"name\":\"B\",\"ifs\":[\"B1\"]}"
    "],\"links\":[[\"A1\",\"R1a\"],[\"R1b\",\"R2a\"],[\"R2b\",\"R3a\"],[\"R3b\",\"B1\"]],"
    "\"goals\":[{\"src\":\"A\",\"dst\":\"B\"}]}";
    ASSERT_EQ_INT(solve_and_validate(t), 0, "chaîne 3 routeurs");
}

TEST(test_star_topology)
{
    /* hub R0 relié à 4 hôtes sur 4 sous-réseaux distincts */
    const char        *t =
    "{\"name\":\"star\",\"devices\":["
    "{\"name\":\"R0\",\"ifs\":[\"R0a\",\"R0b\",\"R0c\",\"R0d\"],\"rt\":\"R0\"},"
    "{\"name\":\"A\",\"ifs\":[\"A1\"]},"
    "{\"name\":\"B\",\"ifs\":[\"B1\"]},"
    "{\"name\":\"C\",\"ifs\":[\"C1\"]},"
    "{\"name\":\"D\",\"ifs\":[\"D1\"]}"
    "],\"links\":[[\"R0a\",\"A1\"],[\"R0b\",\"B1\"],[\"R0c\",\"C1\"],[\"R0d\",\"D1\"]],"
    "\"goals\":[{\"src\":\"A\",\"dst\":\"B\"},{\"src\":\"A\",\"dst\":\"C\"},{\"src\":\"A\",\"dst\":\"D\"}]}";
    ASSERT_EQ_INT(solve_and_validate(t), 0, "étoile 4 hôtes");
}

TEST(test_mesh_topology)
{
    /* 3 routeurs en triangle + hôtes */
    const char        *t =
    "{\"name\":\"mesh\",\"devices\":["
    "{\"name\":\"A\",\"ifs\":[\"A1\"]},"
    "{\"name\":\"R1\",\"ifs\":[\"R1a\",\"R1b\",\"R1c\"],\"rt\":\"R1\"},"
    "{\"name\":\"R2\",\"ifs\":[\"R2a\",\"R2b\",\"R2c\"],\"rt\":\"R2\"},"
    "{\"name\":\"R3\",\"ifs\":[\"R3a\",\"R3b\",\"R3c\"],\"rt\":\"R3\"},"
    "{\"name\":\"B\",\"ifs\":[\"B1\"]}"
    "],\"links\":[[\"A1\",\"R1a\"],[\"R1b\",\"R2a\"],[\"R2b\",\"R3a\"],[\"R3b\",\"R1c\"],[\"R3c\",\"B1\"]],"
    "\"goals\":[{\"src\":\"A\",\"dst\":\"B\"}]}";
    ASSERT_EQ_INT(solve_and_validate(t), 0, "maillage triangle");
}

TEST(test_subnet_size_scaling)
{
    /* un switch avec 10 hôtes (nécessite /28 ou plus grand) */
    const char        *t =
    "{\"name\":\"sw\",\"devices\":["
    "{\"name\":\"H1\",\"ifs\":[\"H11\"]},{\"name\":\"H2\",\"ifs\":[\"H21\"]},{\"name\":\"H3\",\"ifs\":[\"H31\"]},"
    "{\"name\":\"H4\",\"ifs\":[\"H41\"]},{\"name\":\"H5\",\"ifs\":[\"H51\"]},{\"name\":\"H6\",\"ifs\":[\"H61\"]},"
    "{\"name\":\"H7\",\"ifs\":[\"H71\"]},{\"name\":\"H8\",\"ifs\":[\"H81\"]},"
    "{\"name\":\"H9\",\"ifs\":[\"H91\"]},{\"name\":\"H10\",\"ifs\":[\"H101\"]}"
    "],\"links\":[[\"H11\",\"H21\",\"H31\",\"H41\",\"H51\",\"H61\",\"H71\",\"H81\",\"H91\",\"H101\"]],"
    "\"goals\":[{\"src\":\"H1\",\"dst\":\"H10\"}]}";
    ASSERT_EQ_INT(solve_and_validate(t), 0, "switch 10 hôtes");
}

int            main(void)
{
    RUN(test_two_hosts);
    RUN(test_chain_3_routers);
    RUN(test_star_topology);
    RUN(test_mesh_topology);
    RUN(test_subnet_size_scaling);
    SUMMARY();
}
