/* ************************************************************************** */
/*                                                                            */
/*   main.c                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Point d'entrée de miniRT.                                             */
/* ************************************************************************** */

# include "minrt.h"
# include <stdlib.h>

static void    usage(void)
{
    ft_putstr_fd("Usage: miniRT [-b] [-t] [-a] [-w W] [-h H] <scene.rt>\n", 2);
    ft_putstr_fd("  -b  activer les bonus\n", 2);
    ft_putstr_fd("  -t  multithreading\n", 2);
    ft_putstr_fd("  -a  antialiasing\n", 2);
    ft_putstr_fd("  -w  largeur de la fenêtre\n", 2);
    ft_putstr_fd("  -h  hauteur de la fenêtre\n", 2);
}

int    main(int argc, char **argv)
{
    t_game    game;
    int        i;
    int        bonus;
    int        mt;
    int        aa;
    int        w;
    int        h;
    const char    *path;

    bonus = 0;
    mt = 1;
    aa = 0;
    w = WINDOW_W;
    h = WINDOW_H;
    path = NULL;
    i = 1;
    while (i < argc)
    {
        if (ft_strncmp(argv[i], "-b", 2) == 0)
            bonus = 1;
        else if (ft_strncmp(argv[i], "-t", 2) == 0)
            mt = 1;
        else if (ft_strncmp(argv[i], "-a", 2) == 0)
            aa = 1;
        else if (ft_strncmp(argv[i], "-w", 2) == 0 && i + 1 < argc)
            w = ft_atoi(argv[++i]);
        else if (ft_strncmp(argv[i], "-h", 2) == 0 && i + 1 < argc)
            h = ft_atoi(argv[++i]);
        else if (ft_strncmp(argv[i], "-h", 2) == 0)
        {
            usage();
            return (1);
        }
        else
            path = argv[i];
        i++;
    }
    if (!path)
    {
        usage();
        return (1);
    }
    if (w < 1)
        w = WINDOW_W;
    if (h < 1)
        h = WINDOW_H;
    if (game_init(&game, bonus, w, h, mt, aa, path) < 0)
    {
        ft_putendl_fd("miniRT: error: invalid scene file", 2);
        return (1);
    }
    game_loop(&game);
    game_destroy(&game);
    return (0);
}
