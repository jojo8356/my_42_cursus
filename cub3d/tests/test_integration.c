/* ************************************************************************** */
/*                                                                            */
/*   test_integration.c                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Tests d'intégration : lancement complet du jeu (headless).            */
/* ************************************************************************** */

#include "test_framework.h"
#include "cub3d.h"
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <unistd.h>

int    main(void)
{
    TEST_BEGIN();

    TEST_SUITE("Intégration — lancement complet");

    /* --- lancement mandatory : game_init + render_frame --- */
    {
        t_game    game;
        TEST_ASSERT(game_init(&game, 0, "maps/test.cub") == 1);
        render_frame(&game);
        /* une frame produit des pixels et un clear */
        TEST_ASSERT(game.frame == 1);
        game_destroy(&game);
    }

    /* --- lancement bonus : sprites et portes collectés --- */
    {
        t_game    game;
        TEST_ASSERT(game_init(&game, 1, "maps/bonus.cub") == 1);
        TEST_ASSERT(game.bonus == 1);
        TEST_ASSERT(game.sprites_n > 0);
        TEST_ASSERT(game.doors_n > 0);
        render_frame(&game);
        TEST_ASSERT(game.frame == 1);
        game_destroy(&game);
    }

    /* --- plusieurs frames --- */
    {
        t_game    game;
        TEST_ASSERT(game_init(&game, 0, "maps/test.cub") == 1);
        for (int i = 0; i < 5; i++)
            render_frame(&game);
        TEST_ASSERT_EQ(game.frame, 5);
        game_destroy(&game);
    }

    /* --- touche ESC met fin au jeu --- */
    {
        t_game    game;
        TEST_ASSERT(game_init(&game, 0, "maps/test.cub") == 1);
        key_press(65307, &game); /* ESC */
        TEST_ASSERT(game.game_over == 1);
        game_destroy(&game);
    }

    /* --- touche M bascule la minimap --- */
    {
        t_game    game;
        TEST_ASSERT(game_init(&game, 1, "maps/bonus.cub") == 1);
        TEST_ASSERT(game.show_minimap == 1);
        key_press(109, &game); /* M */
        TEST_ASSERT(game.show_minimap == 0);
        key_press(109, &game);
        TEST_ASSERT(game.show_minimap == 1);
        game_destroy(&game);
    }

    /* --- game_destroy ne crashe pas sur jeu non initialisé --- */
    {
        t_game    game;
        memset(&game, 0, sizeof(game));
        game_destroy(&game);
        TEST_ASSERT(1); /* si on arrive ici, pas de crash */
    }

    /* --- game_init échoue sur une carte invalide --- */
    {
        t_game    game;
        /* une carte sans textures obligatoires */
        TEST_ASSERT(game_init(&game, 0, "maps/invalid_no_textures.cub") == 0);
    }

    TEST_END();
}
