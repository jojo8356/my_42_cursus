/* ************************************************************************** */
/*                                                                            */
/*   test_doors.c                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Tests des portes (bonus) : collecte, ouverture/fermeture.              */
/* ************************************************************************** */

#include "test_framework.h"
#include "cub3d.h"
#include <stdlib.h>
#include <math.h>
#include <unistd.h>

/* Carte 7x4 avec porte D valide (2 murs adjacents : haut + bas). */
static const char *DOOR_MAP =
    "NO textures/north.xpm\nSO textures/south.xpm\nWE textures/west.xpm\n"
    "EA textures/east.xpm\nDO textures/door.xpm\n"
    "F 0,0,0\nC 0,0,0\n"
    "1111111\n1N0D001\n1011001\n1111111\n";

static t_game    *make_game_bonus(const char *map_content)
{
    char    path[] = "/tmp/cub3d_door_XXXXXX";
    int        fd = mkstemp(path);
    FILE    *f;
    t_game    *game;

    if (fd < 0)
        return (NULL);
    f = fdopen(fd, "w");
    fputs(map_content, f);
    fclose(f);
    game = (t_game *)xcalloc(1, sizeof(t_game));
    game->bonus = 1;
    if (!parse_cub(path, &game->map) || !map_validate(&game->map))
    {
        game_destroy(game);
        free(game);
        unlink(path);
        return (NULL);
    }
    game->mlx.display = (void *)1;
    game->win = (void *)2;
    game->width = 800;
    game->height = 600;
    player_init(game);
    collect_doors(game);
    unlink(path);
    return (game);
}

int    main(void)
{
    TEST_BEGIN();

    TEST_SUITE("Portes — collecte & animation");

    /* --- collecte des portes --- */
    {
        t_game    *game = make_game_bonus(DOOR_MAP);
        TEST_ASSERT(game != NULL);
        if (game)
        {
            TEST_ASSERT_EQ(game->doors_n, 1);
            TEST_ASSERT_EQ(game->doors[0].state, DOOR_CLOSED);
            TEST_ASSERT_EQ(game->doors[0].open_progress, 0);
            game_destroy(game);
        }
        free(game);
    }

    /* --- une porte s'ouvre quand le joueur est proche et touche E --- */
    {
        t_game    *game = make_game_bonus(DOOR_MAP);
        TEST_ASSERT(game != NULL);
        if (game)
        {
            /* placer le joueur juste à gauche de la porte */
            game->player.x = game->doors[0].x - 0.8;
            game->player.y = game->doors[0].y;
            game->door_key = 1;
            for (int i = 0; i < 20; i++)
                update_doors(game);
            game->door_key = 0;
            TEST_ASSERT(game->doors[0].state == DOOR_OPEN);
            TEST_ASSERT(game->doors[0].open_progress >= 0.9);
            game_destroy(game);
        }
        free(game);
    }

    /* --- la porte se ferme quand on s'éloigne --- */
    {
        t_game    *game = make_game_bonus(DOOR_MAP);
        TEST_ASSERT(game != NULL);
        if (game)
        {
            game->player.x = game->doors[0].x - 0.8;
            game->player.y = game->doors[0].y;
            game->door_key = 1;
            for (int i = 0; i < 20; i++)
                update_doors(game);
            game->door_key = 0;
            /* on s'éloigne */
            game->player.x = 0.5;
            game->player.y = 0.5;
            for (int i = 0; i < 20; i++)
                update_doors(game);
            TEST_ASSERT(game->doors[0].state == DOOR_CLOSED);
            game_destroy(game);
        }
        free(game);
    }

    /* --- une porte fermée bloque le passage --- */
    {
        t_game    *game = make_game_bonus(DOOR_MAP);
        TEST_ASSERT(game != NULL);
        if (game)
        {
            game->player.x = game->doors[0].x - 0.8;
            game->player.y = game->doors[0].y;
            TEST_ASSERT(can_move_to(game, game->doors[0].x, game->doors[0].y) == 0);
            game_destroy(game);
        }
        free(game);
    }

    /* --- une porte ouverte ne bloque plus --- */
    {
        t_game    *game = make_game_bonus(DOOR_MAP);
        TEST_ASSERT(game != NULL);
        if (game)
        {
            game->doors[0].state = DOOR_OPEN;
            TEST_ASSERT(can_move_to(game, game->doors[0].x, game->doors[0].y) == 1);
            game_destroy(game);
        }
        free(game);
    }

    /* --- plusieurs portes collectées --- */
    {
        const char *map2 =
            "NO textures/north.xpm\nSO textures/south.xpm\nWE textures/west.xpm\n"
            "EA textures/east.xpm\nDO textures/door.xpm\n"
            "F 0,0,0\nC 0,0,0\n"
            "111111111\n1N0D00D01\n101101111\n111111111\n";
        t_game    *game = make_game_bonus(map2);
        TEST_ASSERT(game != NULL);
        if (game)
        {
            TEST_ASSERT_EQ(game->doors_n, 2);
            game_destroy(game);
        }
        free(game);
    }

    TEST_END();
}
