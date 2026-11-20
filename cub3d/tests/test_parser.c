/* ************************************************************************** */
/*                                                                            */
/*   test_parser.c                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Tests du parseur .cub et de la validation de la carte.                */
/* ************************************************************************** */

#include "test_framework.h"
#include "cub3d.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* Écrire un fichier temporaire avec le contenu donné. */
static char    *write_tmp(const char *content)
{
    char    path[] = "/tmp/cub3d_test_XXXXXX";
    int        fd = mkstemp(path);
    FILE    *f;

    if (fd < 0)
        return (NULL);
    f = fdopen(fd, "w");
    if (!f)
    {
        close(fd);
        return (NULL);
    }
    fputs(content, f);
    fclose(f);
    return (strdup(path));
}

int    main(void)
{
    TEST_BEGIN();

    TEST_SUITE("Parser .cub — lecture & validation");

    /* --- map valide mandatory --- */
    {
        char    *p = write_tmp("NO textures/north.xpm\nSO textures/south.xpm\n"
            "WE textures/west.xpm\nEA textures/east.xpm\nF 10,20,30\nC 40,50,60\n\n"
            "1111\n1N01\n1001\n1111\n");
        t_map    map;
        TEST_ASSERT(p != NULL);
        TEST_ASSERT(parse_cub(p, &map) == 1);
        TEST_ASSERT(map_validate(&map) == 1);
        TEST_ASSERT_EQ(map.rows, 4);
        TEST_ASSERT_EQ(map.cols, 4);
        TEST_ASSERT(map.has_no && map.has_so && map.has_we && map.has_ea);
        TEST_ASSERT_STR(map.tex_no, "textures/north.xpm");
        TEST_ASSERT_EQ(map.player_x, 1);
        TEST_ASSERT_EQ(map.player_y, 1);
        TEST_ASSERT_EQ(map.dir, DIR_N);
        TEST_ASSERT(map.floor != 0);
        TEST_ASSERT(map.ceiling != 0);
        TEST_ASSERT_EQ(color_r(map.floor), 10);
        TEST_ASSERT_EQ(color_g(map.floor), 20);
        TEST_ASSERT_EQ(color_b(map.floor), 30);
        TEST_ASSERT_EQ(color_r(map.ceiling), 40);
        map_free(&map);
        free(p);
    }

    /* --- orientations S/E/W --- */
    {
        char    *p = write_tmp("NO t/n.xpm\nSO t/s.xpm\nWE t/w.xpm\nEA t/e.xpm\n"
            "F 0,0,0\nC 0,0,0\n11111\n10E01\n10001\n11111\n");
        t_map    map;
        TEST_ASSERT(parse_cub(p, &map) == 1);
        TEST_ASSERT(map_validate(&map) == 1);
        TEST_ASSERT_EQ(map.dir, DIR_E);
        TEST_ASSERT_EQ(map.player_x, 2);
        TEST_ASSERT_EQ(map.player_y, 1);
        map_free(&map);
        free(p);
    }

    /* --- map non fermée (flood-fill échoue) --- */
    {
        char    *p = write_tmp("NO t/n.xpm\nSO t/s.xpm\nWE t/w.xpm\nEA t/e.xpm\n"
            "F 0,0,0\nC 0,0,0\n111111\n1N0000\n111111\n");
        t_map    map;
        TEST_ASSERT(parse_cub(p, &map) == 1);
        TEST_ASSERT(map_validate(&map) == 0);
        map_free(&map);
        free(p);
    }

    /* --- pas de joueur --- */
    {
        char    *p = write_tmp("NO t/n.xpm\nSO t/s.xpm\nWE t/w.xpm\nEA t/e.xpm\n"
            "F 0,0,0\nC 0,0,0\n1111\n1001\n1001\n1111\n");
        t_map    map;
        TEST_ASSERT(parse_cub(p, &map) == 1);
        TEST_ASSERT(map_validate(&map) == 0);
        map_free(&map);
        free(p);
    }

    /* --- texture manquante --- */
    {
        char    *p = write_tmp("NO t/n.xpm\nSO t/s.xpm\nWE t/w.xpm\n"
            "F 0,0,0\nC 0,0,0\n1111\n1N01\n1001\n1111\n");
        t_map    map;
        TEST_ASSERT(parse_cub(p, &map) == 1);
        TEST_ASSERT(map_validate(&map) == 0);
        map_free(&map);
        free(p);
    }

    /* --- deux joueurs --- */
    {
        char    *p = write_tmp("NO t/n.xpm\nSO t/s.xpm\nWE t/w.xpm\nEA t/e.xpm\n"
            "F 0,0,0\nC 0,0,0\n1111\n1N0E\n1001\n1111\n");
        t_map    map;
        TEST_ASSERT(parse_cub(p, &map) == 1);
        TEST_ASSERT(map_validate(&map) == 0);
        map_free(&map);
        free(p);
    }

    /* --- couleur invalide --- */
    {
        char    *p = write_tmp("NO t/n.xpm\nSO t/s.xpm\nWE t/w.xpm\nEA t/e.xpm\n"
            "F 300,20,30\nC 0,0,0\n1111\n1N01\n1001\n1111\n");
        t_map    map;
        TEST_ASSERT(parse_cub(p, &map) == 0);
        map_free(&map);
        free(p);
    }

    /* --- format couleur invalide --- */
    {
        char    *p = write_tmp("NO t/n.xpm\nSO t/s.xpm\nWE t/w.xpm\nEA t/e.xpm\n"
            "F 10;20;30\nC 0,0,0\n1111\n1N01\n1001\n1111\n");
        t_map    map;
        TEST_ASSERT(parse_cub(p, &map) == 0);
        map_free(&map);
        free(p);
    }

    /* --- map bonus avec porte D (2 murs) --- */
    {
        char    *p = write_tmp("NO t/n.xpm\nSO t/s.xpm\nWE t/w.xpm\nEA t/e.xpm\n"
            "DO t/d.xpm\nSP t/s.xpm\nF 0,0,0\nC 0,0,0\n11111\n1N0D1\n10001\n11111\n");
        t_map    map;
        TEST_ASSERT(parse_cub(p, &map) == 1);
        TEST_ASSERT(map_validate(&map) == 1);
        TEST_ASSERT_EQ(map.doors_count, 1);
        TEST_ASSERT(map.has_do);
        map_free(&map);
        free(p);
    }

    /* --- porte D sans 2 murs --- */
    {
        char    *p = write_tmp("NO t/n.xpm\nSO t/s.xpm\nWE t/w.xpm\nEA t/e.xpm\n"
            "DO t/d.xpm\nF 0,0,0\nC 0,0,0\n11111\n1N0D1\n10D01\n11111\n");
        t_map    map;
        TEST_ASSERT(parse_cub(p, &map) == 1);
        TEST_ASSERT(map_validate(&map) == 0);
        map_free(&map);
        free(p);
    }

    /* --- sprites P et A comptés --- */
    {
        char    *p = write_tmp("NO t/n.xpm\nSO t/s.xpm\nWE t/w.xpm\nEA t/e.xpm\n"
            "SP t/s.xpm\nF 0,0,0\nC 0,0,0\n111111\n1N0P01\n10A001\n111111\n");
        t_map    map;
        TEST_ASSERT(parse_cub(p, &map) == 1);
        TEST_ASSERT(map_validate(&map) == 1);
        TEST_ASSERT_EQ(map.sprites_count, 1);
        TEST_ASSERT_EQ(map.anim_sprites_count, 1);
        map_free(&map);
        free(p);
    }

    /* --- fichier inexistant --- */
    {
        t_map    map;
        TEST_ASSERT(parse_cub("/tmp/cub3d_nonexistent_xyz.cub", &map) == 0);
        map_free(&map);
    }

    /* --- map avec lignes de longueur variable (remplissage espaces) --- */
    {
        char    *p = write_tmp("NO t/n.xpm\nSO t/s.xpm\nWE t/w.xpm\nEA t/e.xpm\n"
            "F 0,0,0\nC 0,0,0\n1111111\n1N0\n1000000\n1111111\n");
        t_map    map;
        TEST_ASSERT(parse_cub(p, &map) == 1);
        TEST_ASSERT_EQ(map.cols, 7);
        /* la ligne raccourcie doit être remplie */
        TEST_ASSERT_EQ(map.grid[1][3], ' ');
        map_free(&map);
        free(p);
    }

    TEST_END();
}
