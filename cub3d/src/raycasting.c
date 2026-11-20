/* ************************************************************************** */
/*                                                                            */
/*   raycasting.c                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/20 by jojo8356            #+#    #+#             */
/*   Updated: 2026/11/20 by jojo8356           ###   ########.fr           */
/*                                                                            */
/*   Algorithme DDA (Digital Differential Analysis) pour le raycasting.    */
/* ************************************************************************** */

#include "cub3d.h"
# include <math.h>

/* Initialiser un rayon depuis la position/direction du joueur, avec un
   décalage de colonne donné. */
void    ray_init(t_ray *ray, double x, double y, double dir_x, double dir_y)
{
    ray->start_x = x;
    ray->start_y = y;
    ray->dir_x = dir_x;
    ray->dir_y = dir_y;
    ray->hit = 0;
    ray->side = 0;
    ray->perp_wall_dist = 0.0;
    ray->tile_x = 0;
    ray->tile_y = 0;
    ray->wall_type = CELL_WALL;
    ray->texture = 0;
}

/* Détecter le type de mur à (tile_x, tile_y) et choisir la texture. */
static void    resolve_wall(t_game *game, t_ray *ray)
{
    char    c;

    if (ray->tile_y < 0 || ray->tile_y >= game->map.rows
        || ray->tile_x < 0 || ray->tile_x >= game->map.cols)
    {
        ray->hit = 0;
        ray->perp_wall_dist = 0.0;
        return;
    }
    c = game->map.grid[ray->tile_y][ray->tile_x];
    if (c == 'D')
    {
        ray->wall_type = CELL_DOOR;
        ray->texture = 4; /* DO */
    }
    else
    {
        ray->wall_type = CELL_WALL;
        if (ray->side == 0)
        {
            if (ray->dir_y < 0)
                ray->texture = 0; /* NO */
            else
                ray->texture = 1; /* SO */
        }
        else
        {
            if (ray->dir_x > 0)
                ray->texture = 3; /* EA */
            else
                ray->texture = 2; /* WE */
        }
    }
}

/* Marche du DDA jusqu'au mur. */
void    ray_dda(t_game *game, t_ray *ray)
{
    int    hit = 0;
    int    step_x;
    int    step_y;
    double    delta_dist_x;
    double    delta_dist_y;
    double    side_dist_x;
    double    side_dist_y;
    int    map_x;
    int    map_y;
    double    perp;

    map_x = (int)ray->start_x;
    map_y = (int)ray->start_y;

    if (ray->dir_x < 0)
    {
        step_x = -1;
        side_dist_x = (ray->start_x - map_x) * 2.0;
    }
    else
    {
        step_x = 1;
        side_dist_x = (map_x + 1.0 - ray->start_x) * 2.0;
    }
    if (ray->dir_y < 0)
    {
        step_y = -1;
        side_dist_y = (ray->start_y - map_y) * 2.0;
    }
    else
    {
        step_y = 1;
        side_dist_y = (map_y + 1.0 - ray->start_y) * 2.0;
    }
    if (ray->dir_x == 0)
        delta_dist_x = 1e30;
    else
        delta_dist_x = fabs(2.0 / ray->dir_x);
    if (ray->dir_y == 0)
        delta_dist_y = 1e30;
    else
        delta_dist_y = fabs(2.0 / ray->dir_y);

    while (!hit)
    {
        if (side_dist_x < side_dist_y)
        {
            side_dist_x += delta_dist_x;
            map_x += step_x;
            ray->side = 1;
        }
        else
        {
            side_dist_y += delta_dist_y;
            map_y += step_y;
            ray->side = 0;
        }
        if (map_x < 0 || map_y < 0
            || map_x >= game->map.cols || map_y >= game->map.rows)
        {
            hit = 1;
            ray->tile_x = map_x;
            ray->tile_y = map_y;
            ray->hit = 1;
            /* distance perpendiculaire corrigée (pas de fish-eye) */
            if (ray->side == 0)
                perp = (ray->start_y - map_y)
                    / (ray->dir_y == 0 ? 1e-30 : ray->dir_y);
            else
                perp = (ray->start_x - map_x)
                    / (ray->dir_x == 0 ? 1e-30 : ray->dir_x);
            ray->perp_wall_dist = fabs(perp);
            if (ray->perp_wall_dist < 0.0001)
                ray->perp_wall_dist = 0.0001;
            resolve_wall(game, ray);
            break;
        }
        if (game->map.grid[map_y][map_x] == '1'
            || game->map.grid[map_y][map_x] == 'D')
        {
            hit = 1;
            ray->tile_x = map_x;
            ray->tile_y = map_y;
            ray->hit = 1;
            /* distance perpendiculaire corrigée (pas de fish-eye) */
            if (ray->side == 0)
                perp = (ray->start_y - map_y)
                    / (ray->dir_y == 0 ? 1e-30 : ray->dir_y);
            else
                perp = (ray->start_x - map_x)
                    / (ray->dir_x == 0 ? 1e-30 : ray->dir_x);
            ray->perp_wall_dist = fabs(perp);
            if (ray->perp_wall_dist < 0.0001)
                ray->perp_wall_dist = 0.0001;
            resolve_wall(game, ray);
            break;
        }
    }
}

/* Lancer tous les rayons pour une frame. */
void    cast_rays(t_game *game, t_ray *rays, int n)
{
    t_player    *p = &game->player;
    double        camera_x;

    for (int i = 0; i < n; i++)
    {
        double    dir_x;
        double    dir_y;

        camera_x = 2.0 * i / (double)n - 1.0;
        dir_x = p->dir_x + p->plane_x * camera_x;
        dir_y = p->dir_y + p->plane_y * camera_x;
        ray_init(&rays[i], p->x, p->y, dir_x, dir_y);
        ray_dda(game, &rays[i]);
    }
}
