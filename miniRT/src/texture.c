/* ************************************************************************** */
/*                                                                            */
/*   texture.c                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Chargement et échantillonnage de textures XPM (bonus).                */
/* ************************************************************************** */

# include "minrt.h"
# include "mlx.h"
# include <stdlib.h>

int    texture_load(t_texture *tex, const char *path)
{
    void    *mlx;
    void    *img;
    char    *data;
    int        w;
    int        h;
    int        bpp;
    int        line;
    int        end;

    mlx = mlx_init();
    if (!mlx)
        return (-1);
    img = mlx_xpm_file_to_image(mlx, (char *)path, &w, &h);
    if (!img)
    {
        mlx_destroy_display(mlx);
        return (-1);
    }
    data = mlx_get_data_addr(img, &bpp, &line, &end);
    if (!data || w <= 0 || h <= 0)
    {
        mlx_destroy_image(mlx, img);
        mlx_destroy_display(mlx);
        return (-1);
    }
    tex->width = w;
    tex->height = h;
    tex->bpp = bpp;
    tex->line = line;
    tex->data = (char *)xmalloc((size_t)line * (size_t)h);
    ft_memcpy(tex->data, data, (size_t)line * (size_t)h);
    mlx_destroy_image(mlx, img);
    mlx_destroy_display(mlx);
    return (0);
}

void    texture_free(t_texture *tex)
{
    if (!tex)
        return;
    free(tex->data);
    tex->data = NULL;
    tex->width = 0;
    tex->height = 0;
}

t_color    texture_sample(const t_texture *tex, double u, double v)
{
    int        x;
    int        y;
    size_t    idx;
    char    *p;

    if (!tex || !tex->data)
        return (color_new(1.0, 1.0, 1.0));
    x = (int)(u * (tex->width - 1));
    y = (int)(v * (tex->height - 1));
    if (x < 0)
        x = 0;
    if (y < 0)
        y = 0;
    if (x >= tex->width)
        x = tex->width - 1;
    if (y >= tex->height)
        y = tex->height - 1;
    idx = (size_t)y * tex->line + (size_t)x * (tex->bpp / 8);
    p = tex->data + idx;
    return (color_new(p[0] / 255.0, p[1] / 255.0, p[2] / 255.0));
}

int    texture_load_by_name(t_texture *tex, const char *name)
{
    char    *path;
    int        ret;

    path = ft_strjoin("textures/", name);
    if (!path)
        return (-1);
    {
        char    *ext = ft_strjoin(path, ".xpm");
        free(path);
        path = ext;
    }
    ret = texture_load(tex, path);
    free(path);
    return (ret);
}
