/* ************************************************************************** */
/*                                                                            */
/*   textures.c                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/20 by jojo8356            #+#    #+#             */
/*   Updated: 2026/11/20 by jojo8356           ###   ########.fr           */
/*                                                                            */
/* ************************************************************************** */

#include "cub3d.h"
# include <stdlib.h>
# include <stdio.h>

/* Charger une texture XPM via miniLibX. */
static int    load_one(void *mlx, t_texture *tex, const char *path)
{
    int    w;
    int    h;

    if (!path)
        return (0);
    tex->img = mlx_xpm_file_to_image(mlx, (char *)path, &w, &h);
    if (!tex->img)
    {
        fprintf(stderr, "cub3d: cannot load texture '%s'\n", path);
        return (0);
    }
    tex->width = w;
    tex->height = h;
    tex->data = mlx_get_data_addr(tex->img, &tex->bpp, &tex->size_line, &tex->endian);
    tex->loaded = 1;
    return (1);
}

/* Charger toutes les textures du jeu. */
int            load_textures(t_game *game)
{
    void    *mlx = (void *)game->mlx.display ? game->mlx.display : (void *)1;
    int        ok = 1;

    if (!load_one(mlx, &game->tex.north, game->map.tex_no))
        ok = 0;
    if (!load_one(mlx, &game->tex.south, game->map.tex_so))
        ok = 0;
    if (!load_one(mlx, &game->tex.west, game->map.tex_we))
        ok = 0;
    if (!load_one(mlx, &game->tex.east, game->map.tex_ea))
        ok = 0;
    /* Bonus */
    if (game->map.has_do)
        load_one(mlx, &game->tex.door, game->map.tex_do);
    if (game->map.has_sp)
        load_one(mlx, &game->tex.sprite, game->map.tex_sp);
    if (game->map.has_a1)
        load_one(mlx, &game->tex.anim1, game->map.tex_a1);
    if (game->map.has_a2)
        load_one(mlx, &game->tex.anim2, game->map.tex_a2);
    return (ok);
}

/* Libération des textures. */
void        free_textures(t_game *game)
{
    void    *mlx = (void *)game->mlx.display ? game->mlx.display : (void *)1;
    t_textures    *t = &game->tex;

    (void)mlx;
    if (t->north.loaded)
    {
        mlx_destroy_image(mlx, t->north.img);
        t->north.img = NULL;
        t->north.data = NULL;
        t->north.loaded = 0;
    }
    if (t->south.loaded)
    {
        mlx_destroy_image(mlx, t->south.img);
        t->south.img = NULL;
        t->south.data = NULL;
        t->south.loaded = 0;
    }
    if (t->west.loaded)
    {
        mlx_destroy_image(mlx, t->west.img);
        t->west.img = NULL;
        t->west.data = NULL;
        t->west.loaded = 0;
    }
    if (t->east.loaded)
    {
        mlx_destroy_image(mlx, t->east.img);
        t->east.img = NULL;
        t->east.data = NULL;
        t->east.loaded = 0;
    }
    if (t->door.loaded)
    {
        mlx_destroy_image(mlx, t->door.img);
        t->door.img = NULL;
        t->door.data = NULL;
        t->door.loaded = 0;
    }
    if (t->sprite.loaded)
    {
        mlx_destroy_image(mlx, t->sprite.img);
        t->sprite.img = NULL;
        t->sprite.data = NULL;
        t->sprite.loaded = 0;
    }
    if (t->anim1.loaded)
    {
        mlx_destroy_image(mlx, t->anim1.img);
        t->anim1.img = NULL;
        t->anim1.data = NULL;
        t->anim1.loaded = 0;
    }
    if (t->anim2.loaded)
    {
        mlx_destroy_image(mlx, t->anim2.img);
        t->anim2.img = NULL;
        t->anim2.data = NULL;
        t->anim2.loaded = 0;
    }
}
