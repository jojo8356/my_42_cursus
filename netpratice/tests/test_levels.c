/* ************************************************************************** */
/*                                                                            */
/*   test_levels.c                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Tests : parseur de définition de niveau (devices/links/goals).       */
/* ************************************************************************** */

# include "test_framework.h"

TEST(test_parse_full_level)
{
    const char        *s =
    "{\"name\":\"L1\",\"devices\":["
    "{\"name\":\"A\",\"ifs\":[\"A1\"]},"
    "{\"name\":\"R\",\"ifs\":[\"R1\",\"R2\"],\"rt\":\"R\"}"
    "],\"links\":[[\"A1\",\"R1\"],[\"R2\",\"B1\"]],"
    "\"goals\":[{\"src\":\"A\",\"dst\":\"B\"}]}";
    t_level            lvl;

    ASSERT_EQ_INT(level_parse_str(s, &lvl), 0, "parsé");
    ASSERT_EQ_STR(lvl.name, "L1", "nom");
    ASSERT_EQ_INT(lvl.n_devices, 2, "2 devices");
    ASSERT_EQ_INT(lvl.n_links, 2, "2 liens");
    ASSERT_EQ_INT(lvl.n_goals, 1, "1 objectif");
    ASSERT_EQ_STR(lvl.devices[1].name, "R", "device R");
    ASSERT_EQ_INT(lvl.devices[1].n_ifs, 2, "R a 2 interfaces");
    ASSERT_EQ_STR(lvl.devices[1].rt_name, "R", "rt R");
    ASSERT_EQ_STR(lvl.links[0].ifs[0], "A1", "lien A1");
    ASSERT_EQ_STR(lvl.goals[0].src, "A", "src A");
    ASSERT_EQ_STR(lvl.goals[0].dst, "B", "dst B");
    level_free(&lvl);
}

TEST(test_parse_minimal_level)
{
    const char        *s = "{\"name\":\"m\",\"devices\":[],\"links\":[],\"goals\":[]}";
    t_level            lvl;
    ASSERT_EQ_INT(level_parse_str(s, &lvl), 0, "parsé minimal");
    ASSERT_EQ_INT(lvl.n_devices, 0, "0 devices");
    level_free(&lvl);
}

TEST(test_parse_invalid_level)
{
    t_level            lvl;
    ASSERT_EQ_INT(level_parse_str("{", &lvl), -1, "json invalide");
    ASSERT_EQ_INT(level_parse_str("{\"devices\":[{\"name\":1}]}", &lvl), -1,
        "device invalide");
}

TEST(test_free_no_crash)
{
    const char        *s =
    "{\"name\":\"x\",\"devices\":[{\"name\":\"A\",\"ifs\":[\"A1\"]}],"
    "\"links\":[[\"A1\",\"B1\"]],\"goals\":[{\"src\":\"A\",\"dst\":\"B\"}]}";
    t_level            lvl;
    ASSERT_EQ_INT(level_parse_str(s, &lvl), 0, "parsé");
    level_free(&lvl);
    /* double free sécurisé (bzero après free) */
    level_free(&lvl);
    ASSERT(1, "pas de crash");
}

int            main(void)
{
    RUN(test_parse_full_level);
    RUN(test_parse_minimal_level);
    RUN(test_parse_invalid_level);
    RUN(test_free_no_crash);
    SUMMARY();
}
