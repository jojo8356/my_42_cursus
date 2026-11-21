/* ************************************************************************** */
/*                                                                            */
/*   test_validate.c                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Tests : simulation de routage (can_reach) sur topologie construite.  */
/* ************************************************************************** */

# include "test_framework.h"

/* topologie : A -- R1 -- R2 -- B (3 sous-réseaux /30) */
static const char    *TOPO =
"{"
"\"name\":\"topo3\","
"\"devices\":["
"{\"name\":\"A\",\"ifs\":[\"A1\"]},"
"{\"name\":\"R1\",\"ifs\":[\"R1a\",\"R1b\"],\"rt\":\"R1\"},"
"{\"name\":\"R2\",\"ifs\":[\"R2a\",\"R2b\"],\"rt\":\"R2\"},"
"{\"name\":\"B\",\"ifs\":[\"B1\"]}"
"],"
"\"links\":["
"[\"A1\",\"R1a\"],"
"[\"R1b\",\"R2a\"],"
"[\"R2b\",\"B1\"]"
"],"
"\"goals\":[{\"src\":\"A\",\"dst\":\"B\"}]"
"}";

TEST(test_reachable_with_routes)
{
    t_level            lvl;
    t_solution        sol;
    t_topo            topo;
    char            *err = NULL;

    ASSERT_EQ_INT(level_parse_str(TOPO, &lvl), 0, "level parsé");
    ft_bzero(&topo, sizeof(topo));
    ASSERT_EQ_INT(solver_solve(&lvl, &topo, &sol), 0, "solveur");
    ASSERT(can_reach(&sol, &lvl, "A", "B"), "A->B reachable");
    ASSERT(can_reach(&sol, &lvl, "B", "A"), "B->A reachable");
    ASSERT_EQ_INT(solution_validate_goals(&sol, &lvl, &err), 0, "objectifs OK");
    ft_free(err);
    solution_free(&sol);
    level_free(&lvl);
}

TEST(test_unreachable_without_routes)
{
    t_level            lvl;
    t_solution        sol;
    t_topo            topo;

    ASSERT_EQ_INT(level_parse_str(TOPO, &lvl), 0, "level parsé");
    ft_bzero(&topo, sizeof(topo));
    ASSERT_EQ_INT(solver_solve(&lvl, &topo, &sol), 0, "solveur");
    /* on supprime toutes les routes : A et B ne sont plus reliés */
    {
        int            i;
        for (i = 0; i < sol.n_rts; i++)
            sol.rts[i].count = 0;
    }
    ASSERT(!can_reach(&sol, &lvl, "A", "B"), "A->B non reachable sans routes");
    solution_free(&sol);
    level_free(&lvl);
}

TEST(test_direct_reachable_same_subnet)
{
    /* deux hôtes sur le même sous-réseau, pas de routeur */
    t_level            lvl;
    t_solution        sol;
    const char        *topo2 =
    "{\"name\":\"t2\",\"devices\":["
    "{\"name\":\"A\",\"ifs\":[\"A1\"]},"
    "{\"name\":\"B\",\"ifs\":[\"B1\"]}"
    "],\"links\":[[\"A1\",\"B1\"]],\"goals\":[{\"src\":\"A\",\"dst\":\"B\"}]}";

    ASSERT_EQ_INT(level_parse_str(topo2, &lvl), 0, "level2 parsé");
    ft_bzero(&sol, sizeof(sol));
    /* solution manuelle : même sous-réseau /30 */
    {
        t_iface        *ifs;
        t_ip            base = 0;
        ip_parse("10.0.0.0", &base);
        ifs = (t_iface *)xcalloc(2, sizeof(t_iface));
        ifs[0].name = ft_strdup("A1");
        ifs[0].ip = base + 1;
        ifs[0].has_ip = 1;
        ifs[0].mask = cidr_to_mask(30);
        ifs[0].has_mask = 1;
        ifs[1].name = ft_strdup("B1");
        ifs[1].ip = base + 2;
        ifs[1].has_ip = 1;
        ifs[1].mask = cidr_to_mask(30);
        ifs[1].has_mask = 1;
        sol.ifs = ifs;
        sol.n_ifs = 2;
        sol.cap_ifs = 2;
    }
    ASSERT(can_reach(&sol, &lvl, "A", "B"), "direct reachable");
    solution_free(&sol);
    level_free(&lvl);
}

TEST(test_gateway_must_exist)
{
    /* route avec gateway hors sous-réseau connecté -> config invalide */
    const char        *bad =
    "{\"routes\":{\"R1\":{\"route\":\"0.0.0.0/0\",\"gate\":\"9.9.9.9\"}},"
    "\"ifs\":{\"A1\":{\"ip\":\"10.0.0.1\",\"mask\":\"255.255.255.0\"}}}";
    const char        *lvl_s =
    "{\"name\":\"g\",\"devices\":[{\"name\":\"A\",\"ifs\":[\"A1\"]},"
    "{\"name\":\"R1\",\"ifs\":[\"R1a\"],\"rt\":\"R1\"}],"
    "\"links\":[[\"A1\",\"R1a\"]],\"goals\":[]}";
    t_solution        sol;
    t_level            lvl;
    char            *err = NULL;
    ASSERT_EQ_INT(solution_parse_str(bad, &sol), 0, "parse");
    ASSERT_EQ_INT(level_parse_str(lvl_s, &lvl), 0, "level parsé");
    /* passerelle 9.9.9.9 n'est pas sur 10.0.0.0/24 de R1a */
    ASSERT_EQ_INT(solution_validate_config(&sol, &lvl, &err), -1, "gate rejeté");
    ft_free(err);
    solution_free(&sol);
    level_free(&lvl);
}

int            main(void)
{
    RUN(test_reachable_with_routes);
    RUN(test_unreachable_without_routes);
    RUN(test_direct_reachable_same_subnet);
    RUN(test_gateway_must_exist);
    SUMMARY();
}
