/* ************************************************************************** */
/*                                                                            */
/*   test_sprites.c                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Tests des sprites (bonus) : collecte, animation, tri.                  */
/* ************************************************************************** */

#include "test_framework.h"
#include "cub3d.h"
#include <stdlib.h>
#include <unistd.h>

/* Carte avec sprites P (fixe) et A (animé), sans porte. */
static const char *SPRITE_MAP =
    "NO textures/north.xpm\nSO textures/south.xpm\nWE textures/west.xpm\n"
    "EA textures/east.xpm\nSP textures/sprite.xpm\n"
    "F 0,0,0\nC 0,0,0\n"
    "1111111\n1N0P0A1\n1000001\n1111111\n";

/* Carte avec 2 sprites pour le tri par distance. */
static const char *TWO_SPRITE_MAP =
    "NO textures/north.xpm\nSO textures/south.xpm\nWE textures/west.xpm\n"
    "EA textures/east.xpm\nSP textures/sprite.xpm\n"
    "F 0,0,0\nC 0,0,0\n"
    "111111111\n1N00P0001\n10000A001\n111111111\n";

static t_game    *make_game_bonus(const char *map_content)
{
    char    path[] = "/tmp/cub3d_spt_XXXXXX";
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
    collect_sprites(game);
    unlink(path);
    return (game);
}

int    main(void)
{
    TEST_BEGIN();

    TEST_SUITE("Sprites — collecte, animation, tri");

    /* --- collecte des sprites P et A --- */
    {
        t_game    *game = make_game_bonus(SPRITE_MAP);
        TEST_ASSERT(game != NULL);
        if (game)
        {
            TEST_ASSERT_EQ(game->sprites_n, 2);
            game_destroy(game);
        }
        free(game);
    }

    /* --- types des sprites --- */
    {
        t_game    *game = make_game_bonus(SPRITE_MAP);
        int        found_sprite = 0;
        int        found_anim = 0;
        TEST_ASSERT(game != NULL);
        if (game)
        {
            for (int i = 0; i < game->sprites_n; i++)
            {
                if (game->sprites[i].type == CELL_SPRITE)
                    found_sprite = 1;
                if (game->sprites[i].type == CELL_ANIM_SPRITE)
                    found_anim = 1;
            }
            TEST_ASSERT(found_sprite);
            TEST_ASSERT(found_anim);
            game_destroy(game);
        }
        free(game);
    }

    /* --- tri par distance décroissante --- */
    {
        t_game    *game = make_game_bonus(TWO_SPRITE_MAP);
        TEST_ASSERT(game != NULL);
        if (game)
        {
            TEST_ASSERT_EQ(game->sprites_n, 2);
            TEST_ASSERT(game->sprites[0].dist >= game->sprites[1].dist);
            game_destroy(game);
        }
        free(game);
    }

    /* --- animation alterne les frames --- */
    {
        t_game    *game = make_game_bonus(SPRITE_MAP);
        int        frame0, frame1, frame2;
        TEST_ASSERT(game != NULL);
        if (game)
        {
            frame0 = game->sprites[0].frame;
            update_sprites(game);
            frame1 = game->sprites[0].frame;
            update_sprites(game);
            frame2 = game->sprites[0].frame;
            TEST_ASSERT(frame1 != frame0);
            TEST_ASSERT(frame2 == frame0);
            game_destroy(game);
        }
        free(game);
    }

    /* --- les sprites fixes ne changent pas de frame --- */
    {
        t_game    *game = make_game_bonus(SPRITE_MAP);
        int        frame0;
        TEST_ASSERT(game != NULL);
        if (game)
        {
            /* le sprite fixe est celui de type CELL_SPRITE */
            for (int i = 0; i < game->sprites_n; i++)
            {
                if (game->sprites[i].type == CELL_SPRITE)
                {
                    frame0 = game->sprites[i].frame;
                    update_sprites(game);
                    TEST_ASSERT_EQ(game->sprites[i].frame, frame0);
                    break;
                }
            }
            game_destroy(game);
        }
        free(game);
    }

    TEST_END();
}
