/* ************************************************************************** */
/*                                                                            */
/*   main.c                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/20 by jojo8356            #+#    #+#             */
/*   Updated: 2026/11/20 by jojo8356           ###   ########.fr           */
/*                                                                            */
/*   Point d'entrée : ./cub3D map.cub  (ou ./cub3D_bonus pour le bonus).    */
/* ************************************************************************** */

#include "cub3d.h"
# include <string.h>
# include <stdio.h>
# include <stdlib.h>

/* Détecter si le binaire est la version bonus. */
static int    is_bonus_binary(const char *argv0)
{
    const char    *p;

    if (!argv0)
        return (0);
    p = strrchr(argv0, '/');
    p = p ? p + 1 : argv0;
    return (strstr(p, "bonus") != NULL);
}

int    main(int argc, char **argv)
{
    t_game        game;
    int            bonus;

    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s <map.cub>\n", argv[0] ? argv[0] : "cub3D");
        return (1);
    }
    bonus = is_bonus_binary(argv[0]);
    if (!game_init(&game, bonus, argv[1]))
    {
        fprintf(stderr, "cub3d: failed to initialize (%s)\n", argv[1]);
        return (1);
    }
    game_loop(&game);
    game_destroy(&game);
    return (0);
}
