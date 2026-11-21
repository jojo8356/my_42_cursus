/* ************************************************************************** */
/*                                                                            */
/*   test_solution.c                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Tests : round-trip JSON + validation config des 10 niveaux référence.*/
/* ************************************************************************** */

# include "test_framework.h"

static int        roundtrip_level(const char *path)
{
    t_solution        sol;
    char            *json;
    t_json            *j;
    t_solution        sol2;
    char            *err = NULL;

    if (solution_parse_file(path, &sol) != 0)
        return (-1);
    /* config valide */
    if (solution_validate_config(&sol, NULL, &err) != 0)
    {
        if (err)
            fprintf(stderr, "    config invalide: %s\n", err);
        ft_free(err);
        solution_free(&sol);
        return (-1);
    }
    /* round-trip */
    json = solution_to_json(&sol);
    j = json_parse(json);
    if (!j)
    {
        ft_free(json);
        solution_free(&sol);
        return (-1);
    }
    if (solution_from_json(j, &sol2) != 0)
    {
        json_free(j);
        ft_free(json);
        solution_free(&sol);
        return (-1);
    }
    json_free(j);
    ft_free(json);
    solution_free(&sol2);
    solution_free(&sol);
    return (0);
}

TEST(test_roundtrip_all_levels)
{
    int            i;
    int            ok = 0;

    for (i = 1; i <= 10; i++)
    {
        char            path[64];
        ft_snprintf(path, sizeof(path), "levels/level%d.json", i);
        if (roundtrip_level(path) == 0)
        {
            ok++;
            fprintf(stderr, "    level%d: OK\n", i);
        }
        else
            fprintf(stderr, "    level%d: FAIL\n", i);
    }
    ASSERT_EQ_INT(ok, 10, "10 niveaux round-trip + config valide");
}

TEST(test_parse_level1_structure)
{
    t_solution        sol;
    ASSERT_EQ_INT(solution_parse_file("levels/level1.json", &sol), 0, "parse");
    ASSERT_EQ_INT(sol.n_ifs, 4, "4 interfaces");
    ASSERT_EQ_INT(sol.n_rts, 0, "pas de routes");
    /* A1 a une IP */
    {
        int            i;
        int            found = 0;
        for (i = 0; i < sol.n_ifs; i++)
        {
            if (ft_strcmp(sol.ifs[i].name, "A1") == 0)
            {
                ASSERT(sol.ifs[i].has_ip, "A1 a une IP");
                found = 1;
            }
            if (ft_strcmp(sol.ifs[i].name, "B1") == 0)
                ASSERT(!sol.ifs[i].has_ip, "B1 sans IP");
        }
        ASSERT(found, "A1 trouvé");
    }
    solution_free(&sol);
}

TEST(test_parse_level5_routes)
{
    t_solution        sol;
    ASSERT_EQ_INT(solution_parse_file("levels/level5.json", &sol), 0, "parse");
    ASSERT_EQ_INT(sol.n_rts, 2, "2 tables de routage");
    /* Ar1 a route + gate */
    {
        int            i;
        for (i = 0; i < sol.n_rts; i++)
        {
            if (ft_strcmp(sol.rts[i].name, "Ar1") == 0)
            {
                ASSERT_EQ_INT(sol.rts[i].count, 1, "1 entrée");
                ASSERT(sol.rts[i].entries[0].has_route, "route présente");
                ASSERT(sol.rts[i].entries[0].has_gate, "gate présente");
                ASSERT_EQ_INT(sol.rts[i].entries[0].dest_mask, 0, "route default");
            }
        }
    }
    solution_free(&sol);
}

TEST(test_invalid_config_rejected)
{
    /* IP = adresse réseau -> invalide */
    const char        *bad = "{\"routes\":{},\"ifs\":{\"A1\":{\"ip\":\"10.0.0.0\",\"mask\":\"255.255.255.0\"}}}";
    t_solution        sol;
    char            *err = NULL;
    ASSERT_EQ_INT(solution_parse_str(bad, &sol), 0, "parse");
    ASSERT_EQ_INT(solution_validate_config(&sol, NULL, &err), -1, "réseau rejeté");
    ft_free(err);
    solution_free(&sol);

    /* masque non contigu -> échec du parse */
    {
        const char        *bad2 = "{\"routes\":{},\"ifs\":{\"A1\":{\"ip\":\"10.0.0.1\",\"mask\":\"255.0.255.0\"}}}";
        ASSERT_EQ_INT(solution_parse_str(bad2, &sol), -1, "parse2 échoue");
    }
}

int            main(void)
{
    RUN(test_roundtrip_all_levels);
    RUN(test_parse_level1_structure);
    RUN(test_parse_level5_routes);
    RUN(test_invalid_config_rejected);
    SUMMARY();
}
