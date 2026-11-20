/* ************************************************************************** */
/*                                                                            */
/*   player.c                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/20 by jojo8356            #+#    #+#             */
/*   Updated: 2026/11/20 by jojo8356           ###   ########.fr           */
/*                                                                            */
/*   Déplacement, rotation et collisions du joueur.                         */
/* ************************************************************************** */

#include "cub3d.h"
# include <math.h>
# include <stdlib.h>

/* Initialisation du joueur selon la position/direction de la carte. */
void    player_init(t_game *game)
{
    t_player    *p = &game->player;
    int            dir = game->map.dir;

    p->x = (double)game->map.player_x + 0.5;
    p->y = (double)game->map.player_y + 0.5;
    p->move_speed = MOVE_SPEED;
    p->rot_speed = ROT_SPEED;
    if (dir == DIR_N)
    {
        p->dir_x = 0.0;
        p->dir_y = -1.0;
        p->plane_x = -FOV;
        p->plane_y = 0.0;
    }
    else if (dir == DIR_S)
    {
        p->dir_x = 0.0;
        p->dir_y = 1.0;
        p->plane_x = FOV;
        p->plane_y = 0.0;
    }
    else if (dir == DIR_E)
    {
        p->dir_x = 1.0;
        p->dir_y = 0.0;
        p->plane_x = 0.0;
        p->plane_y = FOV;
    }
    else /* W */
    {
        p->dir_x = -1.0;
        p->dir_y = 0.0;
        p->plane_x = 0.0;
        p->plane_y = -FOV;
    }
}

/* La case (gx, gy) est-elle bloquante ? (mur ou porte fermée) */
static int    is_blocking(t_game *game, int gx, int gy)
{
    if (gy < 0 || gy >= game->map.rows || gx < 0 || gx >= game->map.cols)
        return (1);
    char    c = game->map.grid[gy][gx];
    if (c == '1')
        return (1);
    if (c == 'D')
    {
        /* la porte bloque si elle est fermée (ou en animation vers fermé) */
        for (int i = 0; i < game->doors_n; i++)
        {
            if ((int)game->doors[i].x == gx && (int)game->doors[i].y == gy)
                return (game->doors[i].state != DOOR_OPEN);
        }
        return (1);
    }
    return (0);
}

/* Vérifier si le joueur peut se déplacer vers (x, y). */
int    can_move_to(t_game *game, double x, double y)
{
    /* tester avec un petit rayon de marge (0.2) pour éviter de coller */
    if (is_blocking(game, (int)(x + 0.2), (int)y))
        return (0);
    if (is_blocking(game, (int)(x - 0.2), (int)y))
        return (0);
    if (is_blocking(game, (int)x, (int)(y + 0.2)))
        return (0);
    if (is_blocking(game, (int)x, (int)(y - 0.2)))
        return (0);
    return (1);
}

/* Rotation du vecteur direction/plan. */
void    player_rotate(t_game *game, int dir)
{
    t_player    *p = &game->player;
    double        old_dir_x;
    double        old_plane_x;
    double        speed = p->rot_speed * (double)dir;

    old_dir_x = p->dir_x;
    old_plane_x = p->plane_x;
    p->dir_x = p->dir_x * cos(speed) - p->dir_y * sin(speed);
    p->dir_y = old_dir_x * sin(speed) + p->dir_y * cos(speed);
    p->plane_x = p->plane_x * cos(speed) - p->plane_y * sin(speed);
    p->plane_y = old_plane_x * sin(speed) + p->plane_y * cos(speed);
}

/* Déplacement selon les touches pressées. */
void    player_move(t_game *game)
{
    t_player    *p = &game->player;
    double        dx = 0.0;
    double        dy = 0.0;
    double        mx;
    double        my;

    if (p->key_w || p->key_s)
    {
        if (p->key_w)
        {
            dx += p->dir_x * p->move_speed;
            dy += p->dir_y * p->move_speed;
        }
        if (p->key_s)
        {
            dx -= p->dir_x * p->move_speed;
            dy -= p->dir_y * p->move_speed;
        }
    }
    if (p->key_d || p->key_a)
    {
        if (p->key_d)
        {
            dx += p->plane_x * p->move_speed;
            dy += p->plane_y * p->move_speed;
        }
        if (p->key_a)
        {
            dx -= p->plane_x * p->move_speed;
            dy -= p->plane_y * p->move_speed;
        }
    }
    /* rotation gauche/droite */
    if (p->key_left)
        player_rotate(game, -1);
    if (p->key_right)
        player_rotate(game, 1);

    /* déplacement avec collision (essai séparé x/y) */
    mx = p->x + dx;
    if (can_move_to(game, mx, p->y))
        p->x = mx;
    my = p->y + dy;
    if (can_move_to(game, p->x, my))
        p->y = my;
}
