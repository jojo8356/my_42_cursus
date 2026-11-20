/* ************************************************************************** */
/*                                                                            */
/*   test_textures.c                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Tests du chargement des textures XPM.                                  */
/* ************************************************************************** */

#include "test_framework.h"
#include "cub3d.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static t_game    *make_game(const char *map_content)
{
    char    path[] = "/tmp/cub3d_tex_XXXXXX";
    int        fd = mkstemp(path);
    FILE    *f;
    t_game    *game;

    if (fd < 0)
        return (NULL);
    f = fdopen(fd, "w");
    fputs(map_content, f);
    fclose(f);
    game = (t_game *)xcalloc(1, sizeof(t_game));
    if (!parse_cub(path, &game->map) || !map_validate(&game->map))
    {
        game_destroy(game);
        free(game);
        unlink(path);
        return (NULL);
    }
    game->mlx.display = (void *)1;
    game->win = (void *)2;
    player_init(game);
    unlink(path);
    return (game);
}

int    main(void)
{
    TEST_BEGIN();

    TEST_SUITE("Textures — chargement XPM");

    /* --- chargement des 4 textures obligatoires --- */
    {
        t_game    *game = make_game("NO textures/north.xpm\nSO textures/south.xpm\n"
            "WE textures/west.xpm\nEA textures/east.xpm\nF 0,0,0\nC 0,0,0\n"
            "1111\n1N01\n1001\n1111\n");
        TEST_ASSERT(game != NULL);
        TEST_ASSERT(load_textures(game) == 1);
        TEST_ASSERT(game->tex.north.loaded == 1);
        TEST_ASSERT(game->tex.south.loaded == 1);
        TEST_ASSERT(game->tex.west.loaded == 1);
        TEST_ASSERT(game->tex.east.loaded == 1);
        TEST_ASSERT_EQ(game->tex.north.width, 16);
        TEST_ASSERT_EQ(game->tex.north.height, 16);
        TEST_ASSERT(game->tex.north.data != NULL);
        free_textures(game);
        game_destroy(game);
        free(game);
    }

    /* --- chargement des textures bonus --- */
    {
        t_game    *game = make_game("NO textures/north.xpm\nSO textures/south.xpm\n"
            "WE textures/west.xpm\nEA textures/east.xpm\nDO textures/door.xpm\n"
            "SP textures/sprite.xpm\nA1 textures/anim1.xpm\nA2 textures/anim2.xpm\n"
            "F 0,0,0\nC 0,0,0\n11111\n1N0D1\n10001\n11111\n");
        TEST_ASSERT(game != NULL);
        TEST_ASSERT(load_textures(game) == 1);
        TEST_ASSERT(game->tex.door.loaded == 1);
        TEST_ASSERT(game->tex.sprite.loaded == 1);
        TEST_ASSERT(game->tex.anim1.loaded == 1);
        TEST_ASSERT(game->tex.anim2.loaded == 1);
        free_textures(game);
        game_destroy(game);
        free(game);
    }

    /* --- texture inexistante : échec de chargement --- */
    {
        t_game    *game = make_game("NO textures/nonexistent.xpm\nSO textures/south.xpm\n"
            "WE textures/west.xpm\nEA textures/east.xpm\nF 0,0,0\nC 0,0,0\n"
            "1111\n1N01\n1001\n1111\n");
        TEST_ASSERT(game != NULL);
        /* north ne charge pas, donc load_textures retourne 0 */
        TEST_ASSERT(load_textures(game) == 0);
        TEST_ASSERT(game->tex.north.loaded == 0);
        free_textures(game);
        game_destroy(game);
        free(game);
    }

    /* --- le pixel texture est non nul pour une texture chargée --- */
    {
        t_game    *game = make_game("NO textures/north.xpm\nSO textures/south.xpm\n"
            "WE textures/west.xpm\nEA textures/east.xpm\nF 0,0,0\nC 0,0,0\n"
            "1111\n1N01\n1001\n1111\n");
        t_texture    *tex;
        TEST_ASSERT(game != NULL);
        TEST_ASSERT(load_textures(game) == 1);
        tex = &game->tex.north;
        /* les données sont allouées et non nulles */
        TEST_ASSERT(tex->data != NULL);
        TEST_ASSERT(tex->bpp == 32);
        free_textures(game);
        game_destroy(game);
        free(game);
    }

    /* --- free_textures ne crashe pas (libération multiple) --- */
    {
        t_game    *game = make_game("NO textures/north.xpm\nSO textures/south.xpm\n"
            "WE textures/west.xpm\nEA textures/east.xpm\nF 0,0,0\nC 0,0,0\n"
            "1111\n1N01\n1001\n1111\n");
        TEST_ASSERT(game != NULL);
        TEST_ASSERT(load_textures(game) == 1);
        free_textures(game);
        free_textures(game); /* double free protégé par loaded */
        game_destroy(game);
        free(game);
    }

    TEST_END();
}
