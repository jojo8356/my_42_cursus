/* ************************************************************************** */
/*                                                                            */
/*   render.c                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Rendu image MLX, skybox, antialiasing.                                */
/* ************************************************************************** */

# include "minrt.h"
# include <math.h>

/* Couleur du skybox : dégradé selon la hauteur du rayon. */
t_color    skybox_color(const t_ray *ray)
{
    double    t;

    t = fmax(ray->dir.y * 0.5 + 0.5, 0.0);
    return (color_new(0.2 * (1.0 - t) + 0.4 * t,
                      0.4 * (1.0 - t) + 0.6 * t,
                      0.8 * (1.0 - t) + 0.9 * t));
}

/* Couleur d'un pixel avec échantillonnage multi-points (antialiasing). */
static t_color    render_aa(const t_scene *scene, const t_camera *cam,
                int x, int y, int aa)
{
    t_color    color;
    t_color    sample;
    t_hit    hit;
    int        sx;
    int        sy;
    int        samples;
    double    total;

    color = color_new(0.0, 0.0, 0.0);
    total = 0.0;
    samples = aa ? AA_SAMPLES : 1;
    sy = 0;
    while (sy < samples)
    {
        sx = 0;
        while (sx < samples)
        {
            t_ray    ray;

            ray = camera_ray(cam,
                x + (sx + 0.5) / samples, y + (sy + 0.5) / samples);
            hit.hit = 0;
            if (scene_intersect(scene, &ray, &hit))
                sample = trace_ray(scene, &ray, 0);
            else
                sample = skybox_color(&ray);
            color = color_add(color, sample);
            total += 1.0;
            sx++;
        }
        sy++;
    }
    return (color_scale(color, 1.0 / total));
}

t_color    render_pixel(const t_scene *scene, const t_camera *cam,
                int x, int y, int aa)
{
    return (render_aa(scene, cam, x, y, aa));
}

/* Rendu d'une plage de lignes dans un tampon de couleurs `out`. */
void    render_region(const t_scene *scene, const t_camera *cam,
                int y0, int y1, int aa, t_color *out, int width)
{
    int    y;
    int    x;

    y = y0;
    while (y < y1)
    {
        x = 0;
        while (x < width)
        {
            out[y * width + x] = render_pixel(scene, cam, x, y, aa);
            x++;
        }
        y++;
    }
}

/* Écrit un tampon de couleurs dans l'image MLX. */
static void    flush_to_image(t_mlx *mlx, const t_color *buf, int width,
                int height)
{
    int    x;
    int    y;

    y = 0;
    while (y < height)
    {
        x = 0;
        while (x < width)
        {
            t_color    c;
            char        *p;

            c = color_clamp(buf[y * width + x]);
            p = mlx->img_data + (size_t)y * mlx->img_line
                + (size_t)x * (mlx->img_bpp / 8);
            p[0] = (char)(c.r * 255.0);
            p[1] = (char)(c.g * 255.0);
            p[2] = (char)(c.b * 255.0);
            p[3] = 255;
            x++;
        }
        y++;
    }
}

void    render_frame(t_game *game)
{
    t_color    *buf;
    int        w;
    int        h;
    int        threads;

    w = game->mlx.width;
    h = game->mlx.height;
    buf = xcalloc((size_t)w * (size_t)h, sizeof(t_color));
    if (game->multithread)
    {
        threads = get_num_threads();
        render_multithreaded(&game->scene, &game->scene.camera, h,
            game->antialias ? 1 : 0, buf, w, threads);
    }
    else
    {
        render_region(&game->scene, &game->scene.camera, 0, h,
            game->antialias ? 1 : 0, buf, w);
    }
    flush_to_image(&game->mlx, buf, w, h);
    free(buf);
}
