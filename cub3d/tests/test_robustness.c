/* ************************************************************************** */
/*                                                                            */
/*   test_robustness.c                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Tests de robustesse : cas limites, stress, injection.                 */
/* ************************************************************************** */

#include "test_framework.h"
#include "cub3d.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static char    *write_tmp(const char *content)
{
    char    path[] = "/tmp/cub3d_rb_XXXXXX";
    int        fd = mkstemp(path);
    FILE    *f;

    if (fd < 0)
        return (NULL);
    f = fdopen(fd, "w");
    fputs(content, f);
    fclose(f);
    return (strdup(path));
}

/* Générer une grande carte fermée 21x21 et écrire dans un fichier temp. */
static char    *gen_big_map(void)
{
    char    path[] = "/tmp/cub3d_big_XXXXXX";
    int        fd = mkstemp(path);
    FILE    *f = fdopen(fd, "w");
    char    row[24];
    int        i, j;

    fputs("NO textures/north.xpm\nSO textures/south.xpm\nWE textures/west.xpm\n"
        "EA textures/east.xpm\nF 0,0,0\nC 0,0,0\n", f);
    for (i = 0; i < 21; i++)
    {
        for (j = 0; j < 21; j++)
        {
            if (i == 0 || i == 20 || j == 0 || j == 20)
                row[j] = '1';
            else if (i == 1 && j == 1)
                row[j] = 'N';
            else
                row[j] = '0';
        }
        row[21] = '\n';
        row[22] = '\0';
        fputs(row, f);
    }
    fclose(f);
    return (strdup(path));
}

int    main(void)
{
    TEST_BEGIN();

    TEST_SUITE("Robustesse — cas limites, stress, injection");

    /* --- carte minimale (3x3) --- */
    {
        char    *p = write_tmp("NO textures/north.xpm\nSO textures/south.xpm\n"
            "WE textures/west.xpm\nEA textures/east.xpm\nF 0,0,0\nC 0,0,0\n"
            "111\n1N1\n111\n");
        t_map    map;
        TEST_ASSERT(p != NULL);
        TEST_ASSERT(parse_cub(p, &map) == 1);
        TEST_ASSERT(map_validate(&map) == 1);
        TEST_ASSERT_EQ(map.rows, 3);
        map_free(&map);
        free(p);
    }

    /* --- carte avec plusieurs portes valides --- */
    {
        char    *p = write_tmp("NO textures/north.xpm\nSO textures/south.xpm\n"
            "WE textures/west.xpm\nEA textures/east.xpm\nDO textures/door.xpm\n"
            "F 0,0,0\nC 0,0,0\n"
            "111111111\n1N0D00D01\n101101111\n111111111\n");
        t_map    map;
        TEST_ASSERT(p != NULL);
        TEST_ASSERT(parse_cub(p, &map) == 1);
        TEST_ASSERT(map_validate(&map) == 1);
        TEST_ASSERT_EQ(map.doors_count, 2);
        map_free(&map);
        free(p);
    }

    /* --- grande carte de stress (21x21) --- */
    {
        char    *p = gen_big_map();
        t_game    game;
        TEST_ASSERT(p != NULL);
        TEST_ASSERT(game_init(&game, 1, p) == 1);
        TEST_ASSERT(game.map.rows == 21);
        render_frame(&game);
        game_destroy(&game);
        free(p);
    }

    /* --- couleur avec valeurs limites (0 et 255) --- */
    {
        char    *p = write_tmp("NO textures/north.xpm\nSO textures/south.xpm\n"
            "WE textures/west.xpm\nEA textures/east.xpm\nF 0,0,0\nC 255,255,255\n"
            "1111\n1N01\n1001\n1111\n");
        t_map    map;
        TEST_ASSERT(p != NULL);
        TEST_ASSERT(parse_cub(p, &map) == 1);
        TEST_ASSERT(map_validate(&map) == 1);
        TEST_ASSERT_EQ(color_r(map.ceiling), 255);
        TEST_ASSERT_EQ(color_b(map.floor), 0);
        map_free(&map);
        free(p);
    }

    /* --- couleur hors bornes (256) --- */
    {
        char    *p = write_tmp("NO textures/north.xpm\nSO textures/south.xpm\n"
            "WE textures/west.xpm\nEA textures/east.xpm\nF 256,0,0\nC 0,0,0\n"
            "1111\n1N01\n1001\n1111\n");
        t_map    map;
        TEST_ASSERT(p != NULL);
        TEST_ASSERT(parse_cub(p, &map) == 0);
        map_free(&map);
        free(p);
    }

    /* --- textures avec chemin absolu --- */
    {
        char    *p = write_tmp("NO textures/north.xpm\nSO textures/south.xpm\n"
            "WE textures/west.xpm\nEA textures/east.xpm\nF 0,0,0\nC 0,0,0\n"
            "1111\n1N01\n1001\n1111\n");
        t_map    map;
        TEST_ASSERT(p != NULL);
        TEST_ASSERT(parse_cub(p, &map) == 1);
        TEST_ASSERT_STR(map.tex_no, "textures/north.xpm");
        map_free(&map);
        free(p);
    }

    /* --- map avec sprite A animé et porte D, rendu complet --- */
    {
        t_game    game;
        TEST_ASSERT(game_init(&game, 1, "maps/bonus.cub") == 1);
        for (int i = 0; i < 10; i++)
        {
            player_move(&game);
            update_doors(&game);
            update_sprites(&game);
            render_frame(&game);
        }
        TEST_ASSERT(game.frame == 10);
        game_destroy(&game);
    }

    /* --- jeu avec rotation continue ne crashe pas --- */
    {
        t_game    game;
        TEST_ASSERT(game_init(&game, 1, "maps/bonus.cub") == 1);
        game.player.key_left = 1;
        for (int i = 0; i < 30; i++)
            player_move(&game);
        game.player.key_left = 0;
        game_destroy(&game);
    }

    /* --- rendu sur une carte avec sprites derrière les murs --- */
    {
        t_game    game;
        TEST_ASSERT(game_init(&game, 1, "maps/bonus.cub") == 1);
        render_frame(&game);
        TEST_ASSERT(game.frame == 1);
        game_destroy(&game);
    }

    TEST_END();
}
