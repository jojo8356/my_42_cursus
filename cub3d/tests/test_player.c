/* ************************************************************************** */
/*                                                                            */
/*   test_player.c                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Tests du joueur : mouvement, rotation, collisions.                    */
/* ************************************************************************** */

#include "test_framework.h"
#include "cub3d.h"
#include <stdlib.h>
#include <math.h>
#include <unistd.h>

static t_game    *make_game(const char *map_content)
{
    char    path[] = "/tmp/cub3d_pl_XXXXXX";
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
    game->width = 800;
    game->height = 600;
    player_init(game);
    unlink(path);
    return (game);
}

int    main(void)
{
    TEST_BEGIN();

    TEST_SUITE("Joueur — mouvement, rotation, collisions");

    /* --- initialisation N --- */
    {
        t_game    *game = make_game("NO t/n.xpm\nSO t/s.xpm\nWE t/w.xpm\nEA t/e.xpm\n"
            "F 0,0,0\nC 0,0,0\n1111\n1N01\n1001\n1111\n");
        TEST_ASSERT(game != NULL);
        TEST_ASSERT_NEAR(game->player.x, 1.5, 0.01);
        TEST_ASSERT_NEAR(game->player.y, 1.5, 0.01);
        TEST_ASSERT_NEAR(game->player.dir_y, -1.0, 0.01);
        TEST_ASSERT_NEAR(game->player.plane_x, -FOV, 0.01);
        game_destroy(game);
        free(game);
    }

    /* --- rotation à gauche change dir --- */
    {
        t_game    *game = make_game("NO t/n.xpm\nSO t/s.xpm\nWE t/w.xpm\nEA t/e.xpm\n"
            "F 0,0,0\nC 0,0,0\n1111\n1N01\n1001\n1111\n");
        double    before_x = game->player.dir_x;
        double    before_y = game->player.dir_y;
        TEST_ASSERT(game != NULL);
        player_rotate(game, 1);
        TEST_ASSERT(game->player.dir_x != before_x
            || game->player.dir_y != before_y);
        game_destroy(game);
        free(game);
    }

    /* --- déplacement vers le Nord (W) avance vers le mur N --- */
    {
        t_game    *game = make_game("NO t/n.xpm\nSO t/s.xpm\nWE t/w.xpm\nEA t/e.xpm\n"
            "F 0,0,0\nC 0,0,0\n1111111\n1N00001\n1000001\n1111111\n");
        double    start_y;
        TEST_ASSERT(game != NULL);
        start_y = game->player.y;
        game->player.key_w = 1;
        for (int i = 0; i < 10; i++)
            player_move(game);
        game->player.key_w = 0;
        /* le joueur avance vers le Nord (y diminue) mais ne traverse pas le mur */
        TEST_ASSERT(game->player.y < start_y);
        TEST_ASSERT(game->player.y >= 1.0);
        game_destroy(game);
        free(game);
    }

    /* --- collision : ne pas traverser un mur --- */
    {
        t_game    *game = make_game("NO t/n.xpm\nSO t/s.xpm\nWE t/w.xpm\nEA t/e.xpm\n"
            "F 0,0,0\nC 0,0,0\n1111111\n1N10001\n1000001\n1111111\n");
        TEST_ASSERT(game != NULL);
        /* le joueur est en (1,1), mur à (2,1). Aller Est doit être bloqué. */
        game->player.key_d = 1;
        for (int i = 0; i < 20; i++)
            player_move(game);
        game->player.key_d = 0;
        /* le joueur ne doit pas avoir dépassé x=2 (mur) */
        TEST_ASSERT(game->player.x < 2.0);
        game_destroy(game);
        free(game);
    }

    /* --- can_move_to sur un mur --- */
    {
        t_game    *game = make_game("NO t/n.xpm\nSO t/s.xpm\nWE t/w.xpm\nEA t/e.xpm\n"
            "F 0,0,0\nC 0,0,0\n1111\n1N01\n1001\n1111\n");
        TEST_ASSERT(game != NULL);
        TEST_ASSERT(can_move_to(game, 0.5, 0.5) == 0); /* mur (0,0) */
        TEST_ASSERT(can_move_to(game, 1.5, 1.5) == 1); /* espace libre */
        TEST_ASSERT(can_move_to(game, -1.0, 1.5) == 0); /* hors carte */
        game_destroy(game);
        free(game);
    }

    /* --- déplacement diagonal avec collision séparée x/y --- */
    {
        t_game    *game = make_game("NO t/n.xpm\nSO t/s.xpm\nWE t/w.xpm\nEA t/e.xpm\n"
            "F 0,0,0\nC 0,0,0\n11111\n1N001\n10001\n11111\n");
        double    sx, sy;
        TEST_ASSERT(game != NULL);
        sx = game->player.x;
        sy = game->player.y;
        game->player.key_w = 1;
        game->player.key_d = 1;
        player_move(game);
        game->player.key_w = 0;
        game->player.key_d = 0;
        /* les deux composantes peuvent avancer */
        TEST_ASSERT(game->player.x != sx || game->player.y != sy);
        game_destroy(game);
        free(game);
    }

    TEST_END();
}
