/* ************************************************************************** */
/*                                                                            */
/*   test_raycasting.c                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Tests du moteur DDA de raycasting.                                     */
/* ************************************************************************** */

#include "test_framework.h"
#include "cub3d.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* Construire un jeu minimal avec une carte pour tester le raycasting. */
static t_game    *make_game(const char *map_content)
{
    char    path[] = "/tmp/cub3d_rc_XXXXXX";
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

    TEST_SUITE("Raycasting — DDA & distances");

    /* --- rayon vers un mur Nord --- */
    {
        t_game    *game = make_game("NO t/n.xpm\nSO t/s.xpm\nWE t/w.xpm\nEA t/e.xpm\n"
            "F 0,0,0\nC 0,0,0\n11111\n1N001\n10001\n11111\n");
        t_ray    ray;
        TEST_ASSERT(game != NULL);
        /* le joueur est en (1,1) orienté N (dir_y=-1). Le mur N est à y=0. */
        ray_init(&ray, game->player.x, game->player.y,
            game->player.dir_x, game->player.dir_y);
        ray_dda(game, &ray);
        TEST_ASSERT(ray.hit == 1);
        TEST_ASSERT_NEAR(ray.perp_wall_dist, 1.5, 0.05);
        TEST_ASSERT_EQ(ray.tile_y, 0);
        TEST_ASSERT_EQ(ray.texture, 0); /* NO */
        game_destroy(game);
        free(game);
    }

    /* --- rayon vers un mur Sud --- */
    {
        t_game    *game = make_game("NO t/n.xpm\nSO t/s.xpm\nWE t/w.xpm\nEA t/e.xpm\n"
            "F 0,0,0\nC 0,0,0\n11111\n10001\n1S001\n10001\n11111\n");
        t_ray    ray;
        TEST_ASSERT(game != NULL);
        ray_init(&ray, game->player.x, game->player.y,
            game->player.dir_x, game->player.dir_y);
        ray_dda(game, &ray);
        TEST_ASSERT(ray.hit == 1);
        TEST_ASSERT_NEAR(ray.perp_wall_dist, 1.5, 0.05);
        TEST_ASSERT_EQ(ray.texture, 1); /* SO */
        game_destroy(game);
        free(game);
    }

    /* --- rayon vers un mur Est --- */
    {
        t_game    *game = make_game("NO t/n.xpm\nSO t/s.xpm\nWE t/w.xpm\nEA t/e.xpm\n"
            "F 0,0,0\nC 0,0,0\n11111\n10001\n10E01\n10001\n11111\n");
        t_ray    ray;
        TEST_ASSERT(game != NULL);
        ray_init(&ray, game->player.x, game->player.y,
            game->player.dir_x, game->player.dir_y);
        ray_dda(game, &ray);
        TEST_ASSERT(ray.hit == 1);
        TEST_ASSERT_NEAR(ray.perp_wall_dist, 1.5, 0.05);
        TEST_ASSERT_EQ(ray.texture, 3); /* EA */
        game_destroy(game);
        free(game);
    }

    /* --- distance corrigée (pas de fish-eye) pour un angle --- */
    {
        t_game    *game = make_game("NO t/n.xpm\nSO t/s.xpm\nWE t/w.xpm\nEA t/e.xpm\n"
            "F 0,0,0\nC 0,0,0\n11111\n1N001\n10001\n11111\n");
        t_ray    ray;
        double    dir_x, dir_y;
        TEST_ASSERT(game != NULL);
        /* rayon à 45° vers le coin NW : touche le mur N à distance ~1.41 */
        dir_x = -0.7071;
        dir_y = -0.7071;
        ray_init(&ray, game->player.x, game->player.y, dir_x, dir_y);
        ray_dda(game, &ray);
        TEST_ASSERT(ray.hit == 1);
        /* distance perpendiculaire doit être < distance euclidienne (correction) */
        TEST_ASSERT(ray.perp_wall_dist <= 2.2);
        game_destroy(game);
        free(game);
    }

    /* --- cast_rays remplit tous les rayons --- */
    {
        t_game    *game = make_game("NO t/n.xpm\nSO t/s.xpm\nWE t/w.xpm\nEA t/e.xpm\n"
            "F 0,0,0\nC 0,0,0\n11111\n1N001\n10001\n11111\n");
        t_ray    *rays;
        int        i;
        int        n = 100;
        int        hit_count = 0;
        TEST_ASSERT(game != NULL);
        rays = (t_ray *)xcalloc((size_t)n, sizeof(t_ray));
        cast_rays(game, rays, n);
        for (i = 0; i < n; i++)
            if (rays[i].hit)
                hit_count++;
        TEST_ASSERT_EQ(hit_count, n); /* tous les rayons touchent un mur */
        free(rays);
        game_destroy(game);
        free(game);
    }

    /* --- rayon dans une carte ouverte ne touche pas (bord) --- */
    {
        t_game    *game = make_game("NO t/n.xpm\nSO t/s.xpm\nWE t/w.xpm\nEA t/e.xpm\n"
            "F 0,0,0\nC 0,0,0\n111111111\n1N0000001\n100000001\n111111111\n");
        t_ray    ray;
        TEST_ASSERT(game != NULL);
        /* rayon vers le Nord : touche le mur N */
        ray_init(&ray, game->player.x, game->player.y, 0.0, -1.0);
        ray_dda(game, &ray);
        TEST_ASSERT(ray.hit == 1);
        game_destroy(game);
        free(game);
    }

    TEST_END();
}
