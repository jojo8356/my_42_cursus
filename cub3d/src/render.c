/* ************************************************************************** */
/*                                                                            */
/*   render.c                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/20 by jojo8356            #+#    #+#             */
/*   Updated: 2026/11/20 by jojo8356           ###   ########.fr           */
/*                                                                            */
/*   Rendu des murs, du sol/plafond, des sprites et de la minimap.         */
/* ************************************************************************** */

#include "cub3d.h"
# include <math.h>
# include <stdlib.h>
# include <stdio.h>

/* Lire un pixel texture selon la couleur du format mlx (RGBA, little-endian
   par défaut sur la plupart des systèmes). */
static t_color    texture_pixel(t_texture *tex, int tx, int ty)
{
    size_t    index;
    int        bpp = tex->bpp;
    int        size_line = tex->size_line;
    char        *data = tex->data;

    if (!tex || !tex->data || !tex->loaded)
        return (color(0, 0, 0, 255));
    if (tx < 0 || tx >= tex->width || ty < 0 || ty >= tex->height)
        return (color(0, 0, 0, 255));
    if (bpp == 32)
    {
        index = (size_t)ty * size_line + (size_t)tx * 4;
        unsigned char    r = (unsigned char)data[index];
        unsigned char    g = (unsigned char)data[index + 1];
        unsigned char    b = (unsigned char)data[index + 2];
        unsigned char    a = (unsigned char)data[index + 3];
        return (color(r, g, b, a));
    }
    else if (bpp == 24)
    {
        index = (size_t)ty * size_line + (size_t)tx * 3;
        unsigned char    r = (unsigned char)data[index];
        unsigned char    g = (unsigned char)data[index + 1];
        unsigned char    b = (unsigned char)data[index + 2];
        return (color(r, g, b, 255));
    }
    return (color(0, 0, 0, 255));
}

/* Choisir la texture selon l'indice. */
static t_texture    *get_wall_texture(t_game *game, int idx)
{
    if (idx == 0)
        return (&game->tex.north);
    if (idx == 1)
        return (&game->tex.south);
    if (idx == 2)
        return (&game->tex.west);
    if (idx == 3)
        return (&game->tex.east);
    return (&game->tex.door);
}

/* Rendu des murs : pour chaque rayon, une colonne de pixels. */
void    draw_walls(t_game *game, t_ray *rays, int n)
{
    int    y;
    int    start;
    int    end;

    for (int i = 0; i < n; i++)
    {
        t_ray        *r = &rays[i];
        t_texture    *tex;
        double        wall_height;
        int            line_height;
        int            draw_start;
        int            draw_end;
        double        wall_x;
        int            tex_x;
        double        step;
        double        tex_pos;

        if (r->perp_wall_dist <= 0.0001)
            continue;
        wall_height = (double)game->height / r->perp_wall_dist;
        line_height = (int)wall_height;
        draw_start = -line_height / 2 + game->height / 2;
        if (draw_start < 0)
            draw_start = 0;
        draw_end = line_height / 2 + game->height / 2;
        if (draw_end >= game->height)
            draw_end = game->height - 1;

        tex = get_wall_texture(game, r->texture);
        /* coordonnée texture en x selon le côté du mur */
        if (r->side == 0)
            wall_x = game->player.y + r->perp_wall_dist * r->dir_y;
        else
            wall_x = game->player.x + r->perp_wall_dist * r->dir_x;
        wall_x -= floor(wall_x);
        tex_x = (int)(wall_x * (double)tex->width);
        if (r->side == 1 && tex_x > 0)
            tex_x = tex->width - tex_x;
        if (tex_x < 0)
            tex_x = 0;
        if (tex_x >= tex->width)
            tex_x = tex->width - 1;

        step = (double)tex->height / (double)line_height;
        tex_pos = (double)(draw_start - (game->height - line_height) / 2) * step;
        for (y = draw_start; y <= draw_end; y++)
        {
            int    ty = (int)tex_pos;
            t_color    c = texture_pixel(tex, tex_x, ty);
            /* assombrissement selon la distance (effets de lumière) */
            if (r->side == 1)
            {
                c = color(color_r(c) / 2, color_g(c) / 2, color_b(c) / 2, color_a(c));
            }
            mlx_pixel_put(game->mlx.display ? game->mlx.display : (void *)1,
                game->win, i, y, c);
            tex_pos += step;
        }
        (void)start;
        (void)end;
    }
}

/* Rendu du sol et du plafond (sans texture : couleurs unies, ou textures
   si présentes en bonus). */
void    draw_floor_ceiling(t_game *game, t_ray *rays, int n)
{
    int    x;
    int    y;

    for (x = 0; x < game->width; x++)
    {
        /* pour la colonne x, on utilise le rayon correspondant pour le sol */
        double    floor_step = 0.0;
        double    ray_dir_x;
        double    ray_dir_y;
        double    px;
        double    py;
        int        p;

        if (x < n)
        {
            ray_dir_x = rays[x].dir_x;
            ray_dir_y = rays[x].dir_y;
        }
        else
        {
            ray_dir_x = game->player.dir_x;
            ray_dir_y = game->player.dir_y;
        }
        for (p = 0; p < game->height / 2; p++)
        {
            floor_step = (double)p / (double)(game->height / 2.0);
            px = game->player.x + ray_dir_x * floor_step * (game->width / 2.0);
            py = game->player.y + ray_dir_y * floor_step * (game->width / 2.0);
            (void)px;
            (void)py;
        }
        (void)floor_step;
    }
    /* Rendu simple : sol = floor, plafond = ceiling (couleurs) */
    for (y = 0; y < game->height / 2; y++)
        for (x = 0; x < game->width; x++)
            mlx_pixel_put(game->mlx.display ? game->mlx.display : (void *)1,
                game->win, x, y, game->map.ceiling);
    for (y = game->height / 2; y < game->height; y++)
        for (x = 0; x < game->width; x++)
            mlx_pixel_put(game->mlx.display ? game->mlx.display : (void *)1,
                game->win, x, y, game->map.floor);
}

/* Collecter les sprites (P/A) et leurs distances. */
void            collect_sprites(t_game *game)
{
    if (game->sprites)
        free(game->sprites);
    game->sprites = NULL;
    game->sprites_n = 0;
    for (int y = 0; y < game->map.rows; y++)
        for (int x = 0; x < game->map.cols; x++)
        {
            char    c = game->map.grid[y][x];
            if (c == 'P' || c == 'A')
            {
                t_sprite    *s = (t_sprite *)xcalloc(1, sizeof(t_sprite));
                s->x = (double)x + 0.5;
                s->y = (double)y + 0.5;
                s->type = (c == 'A') ? CELL_ANIM_SPRITE : CELL_SPRITE;
                s->frame = 0;
                s->id = game->sprites_n;
                s->dist = 0.0;
                game->sprites = (t_sprite *)xrealloc(game->sprites,
                    (size_t)(game->sprites_n + 1) * sizeof(t_sprite));
                game->sprites[game->sprites_n++] = *s;
                free(s);
            }
        }
    /* tri par distance décroissante (loin -> proche) */
    for (int i = 0; i < game->sprites_n; i++)
    {
        double    dx = game->sprites[i].x - game->player.x;
        double    dy = game->sprites[i].y - game->player.y;
        game->sprites[i].dist = sqrt(dx * dx + dy * dy);
    }
    for (int i = 0; i < game->sprites_n; i++)
        for (int j = i + 1; j < game->sprites_n; j++)
            if (game->sprites[j].dist > game->sprites[i].dist)
            {
                t_sprite    tmp = game->sprites[i];
                game->sprites[i] = game->sprites[j];
                game->sprites[j] = tmp;
            }
}

/* Collecter les portes (D) pour le rendu et l'animation. */
void            collect_doors(t_game *game)
{
    if (game->doors)
        free(game->doors);
    game->doors = NULL;
    game->doors_n = 0;
    for (int y = 0; y < game->map.rows; y++)
        for (int x = 0; x < game->map.cols; x++)
        {
            if (game->map.grid[y][x] == 'D')
            {
                t_door    *d = (t_door *)xcalloc(1, sizeof(t_door));
                d->x = (double)x + 0.5;
                d->y = (double)y + 0.5;
                d->state = DOOR_CLOSED;
                d->open_progress = 0.0;
                d->dist = 0.0;
                game->doors = (t_door *)xrealloc(game->doors,
                    (size_t)(game->doors_n + 1) * sizeof(t_door));
                game->doors[game->doors_n++] = *d;
                free(d);
            }
        }
}

/* Mettre à jour les portes (animation d'ouverture/fermeture). */
void    update_doors(t_game *game)
{
    for (int i = 0; i < game->doors_n; i++)
    {
        t_door    *d = &game->doors[i];
        double    dx = game->player.x - d->x;
        double    dy = game->player.y - d->y;
        double    dist = sqrt(dx * dx + dy * dy);
        d->dist = dist;
        if (game->door_key && dist < 1.5)
        {
            if (d->state == DOOR_CLOSED || d->state == DOOR_ANIMATING)
            {
                d->state = DOOR_ANIMATING;
                d->open_progress += 0.1;
                if (d->open_progress >= 1.0)
                {
                    d->open_progress = 1.0;
                    d->state = DOOR_OPEN;
                }
            }
        }
        else
        {
            if (d->state == DOOR_OPEN || d->state == DOOR_ANIMATING)
            {
                d->state = DOOR_ANIMATING;
                d->open_progress -= 0.1;
                if (d->open_progress <= 0.0)
                {
                    d->open_progress = 0.0;
                    d->state = DOOR_CLOSED;
                }
            }
        }
    }
}

/* Mettre à jour les sprites animés (alternance des frames). */
void    update_sprites(t_game *game)
{
    for (int i = 0; i < game->sprites_n; i++)
    {
        if (game->sprites[i].type == CELL_ANIM_SPRITE)
            game->sprites[i].frame = (game->sprites[i].frame + 1) % 2;
    }
}

/* Rendu des sprites (bonus) — approximation par calcul de projection. */
void    draw_sprites(t_game *game)
{
    for (int i = 0; i < game->sprites_n; i++)
    {
        t_sprite    *s = &game->sprites[i];
        double    dx = s->x - game->player.x;
        double    dy = s->y - game->player.y;
        double    inv_det = 1.0 / (game->player.dir_x * game->player.plane_y
            - game->player.dir_y * game->player.plane_x);
        double    transform_x = inv_det * (game->player.plane_y * dx
            - game->player.plane_x * dy) * 0.5;
        double    transform_y = inv_det * (-game->player.dir_y * dx
            + game->player.dir_x * dy) * 0.5;
        int        sprite_screen_x;
        int        sprite_height;
        int        sprite_width;
        int        y;
        t_texture    *tex = &game->tex.sprite;

        if (transform_y <= 0.001)
            continue;
        sprite_screen_x = (int)((double)game->width / 2.0
            * (1.0 + transform_x / transform_y));
        sprite_height = (int)((double)game->height / transform_y)
            * (tex->height > 0 ? 0.5 : 1);
        sprite_width = (int)((double)game->height / transform_y)
            * (tex->width > 0 ? 0.5 : 1);
        for (y = 0; y < sprite_height; y++)
        {
            int    draw_y = y - sprite_height / 2 + game->height / 2;
            int    x;
            for (x = 0; x < sprite_width; x++)
            {
                int    draw_x = x - sprite_width / 2 + sprite_screen_x;
                int    tx = (x * tex->width) / sprite_width;
                int    ty = (y * tex->height) / sprite_height;
                t_color    c = texture_pixel(tex, tx, ty);
                if (c != color(0, 0, 0, 255) && draw_x >= 0
                    && draw_x < game->width && draw_y >= 0
                    && draw_y < game->height)
                    mlx_pixel_put(game->mlx.display ? game->mlx.display : (void *)1,
                        game->win, draw_x, draw_y, c);
            }
        }
    }
}

/* Rendu de la minimap (bonus). */
void    draw_minimap(t_game *game)
{
    int    scale = 6;
    int    off_x = 10;
    int    off_y = 10;
    int    i;
    int    j;

    for (i = 0; i < game->map.rows; i++)
        for (j = 0; j < game->map.cols; j++)
        {
            int    x = off_x + j * scale;
            int    y = off_y + i * scale;
            t_color    c;
            char    cell = game->map.grid[i][j];
            if (cell == '1')
                c = color(120, 120, 120, 255);
            else if (cell == 'D')
                c = color(255, 200, 0, 255);
            else
                c = color(40, 40, 40, 255);
            for (int k = 0; k < scale; k++)
                for (int l = 0; l < scale; l++)
                    mlx_pixel_put(game->mlx.display ? game->mlx.display : (void *)1,
                        game->win, x + k, y + l, c);
        }
    /* position du joueur */
    int    px = off_x + (int)game->player.x * scale;
    int    py = off_y + (int)game->player.y * scale;
    mlx_pixel_put(game->mlx.display ? game->mlx.display : (void *)1,
        game->win, px, py, color(0, 255, 0, 255));
}

/* HUD (bonus) : compteur de frames, mode. */
void    draw_hud(t_game *game)
{
    char    buf[64];
    snprintf(buf, sizeof(buf), "frame: %d", game->frame);
    mlx_string_put(game->mlx.display ? game->mlx.display : (void *)1,
        game->win, 10, game->height - 20, color(255, 255, 255, 255), buf);
}

/* Rendu complet d'une frame. */
void    render_frame(t_game *game)
{
    t_ray        *rays;
    int            n = game->width;

    rays = (t_ray *)xcalloc((size_t)n, sizeof(t_ray));
    mlx_clear_window(game->mlx.display ? game->mlx.display : (void *)1, game->win);
    cast_rays(game, rays, n);
    draw_floor_ceiling(game, rays, n);
    draw_walls(game, rays, n);
    if (game->bonus && game->sprites_n > 0)
        draw_sprites(game);
    if (game->bonus && game->show_minimap)
        draw_minimap(game);
    if (game->bonus)
        draw_hud(game);
    free(rays);
    game->frame++;
}
