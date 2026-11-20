/* ************************************************************************** */
/*                                                                            */
/*   hooks.c                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/20 by jojo8356            #+#    #+#             */
/*   Updated: 2026/11/20 by jojo8356           ###   ########.fr           */
/*                                                                            */
/*   Hooks clavier / exposition et boucle principale.                       */
/* ************************************************************************** */

#include "cub3d.h"
# include <stdlib.h>
# include <stdio.h>
# include <string.h>

/* Codes de touches (miniLibX / X11). */
# define KEY_W 119
# define KEY_A 97
# define KEY_S 115
# define KEY_D 100
# define KEY_LEFT 65361
# define KEY_RIGHT 65363
# define KEY_ESC 65307
# define KEY_M 109
# define KEY_E 101

/* Appui sur une touche. */
int    key_press(int keycode, void *param)
{
    t_game    *game = (t_game *)param;

    if (keycode == KEY_ESC)
    {
        game->game_over = 1;
        return (0);
    }
    if (keycode == KEY_W)
        game->player.key_w = 1;
    else if (keycode == KEY_S)
        game->player.key_s = 1;
    else if (keycode == KEY_A)
        game->player.key_a = 1;
    else if (keycode == KEY_D)
        game->player.key_d = 1;
    else if (keycode == KEY_LEFT)
        game->player.key_left = 1;
    else if (keycode == KEY_RIGHT)
        game->player.key_right = 1;
    else if (keycode == KEY_M)
        game->show_minimap = !game->show_minimap;
    else if (keycode == KEY_E)
        game->door_key = 1;
    return (0);
}

/* Relâchement d'une touche. */
int    key_release(int keycode, void *param)
{
    t_game    *game = (t_game *)param;

    if (keycode == KEY_W)
        game->player.key_w = 0;
    else if (keycode == KEY_S)
        game->player.key_s = 0;
    else if (keycode == KEY_A)
        game->player.key_a = 0;
    else if (keycode == KEY_D)
        game->player.key_d = 0;
    else if (keycode == KEY_LEFT)
        game->player.key_left = 0;
    else if (keycode == KEY_RIGHT)
        game->player.key_right = 0;
    else if (keycode == KEY_E)
        game->door_key = 0;
    return (0);
}

/* Hook d'exposition : on redessine à chaque demande. */
int    expose_hook(void *param)
{
    t_game    *game = (t_game *)param;

    render_frame(game);
    return (0);
}

/* Une itération de la boucle (appliquée par mlx_loop_hook). */
static int    loop_step(void *param)
{
    t_game    *game = (t_game *)param;

    if (game->game_over)
    {
        mlx_loop_end(game->mlx.display ? game->mlx.display : (void *)1);
        return (0);
    }
    player_move(game);
    if (game->bonus)
    {
        update_doors(game);
        update_sprites(game);
    }
    render_frame(game);
    return (0);
}

/* Boucle principale. */
void    game_loop(t_game *game)
{
    void    *mlx = game->mlx.display ? game->mlx.display : (void *)1;

    mlx_loop_hook(mlx, loop_step, game);
    mlx_loop(mlx);
}

/* Initialisation complète du jeu. */
int    game_init(t_game *game, int bonus, const char *path)
{
    memset(game, 0, sizeof(*game));
    game->bonus = bonus;
    game->width = WINDOW_W;
    game->height = WINDOW_H;
    game->show_minimap = 1;
    game->frame_rate_cap = 60;

    if (!parse_cub(path, &game->map))
        return (0);
    if (!map_validate(&game->map))
        return (0);
    game->mlx.display = mlx_init();
    game->win = mlx_new_window(game->mlx.display, game->width, game->height,
        bonus ? "cub3D_bonus" : "cub3D");
    player_init(game);
    if (!load_textures(game))
        return (0);
    if (bonus)
    {
        collect_sprites(game);
        collect_doors(game);
    }
    mlx_key_hook(game->win, key_press, game);
    mlx_hook(game->win, 2, 1, expose_hook, game);
    return (1);
}

/* Libération des ressources. */
void    game_destroy(t_game *game)
{
    void    *mlx = game->mlx.display ? game->mlx.display : (void *)1;

    free_textures(game);
    if (game->sprites)
        free(game->sprites);
    if (game->doors)
        free(game->doors);
    if (game->win)
        mlx_destroy_window(mlx, game->win);
    if (game->mlx.display)
        mlx_destroy_display(mlx);
    map_free(&game->map);
}
