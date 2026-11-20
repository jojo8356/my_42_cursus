/* ************************************************************************** */
/*                                                                            */
/*   camera.c                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Caméra perspective + génération de rayons.                           */
/* ************************************************************************** */

# include "minrt.h"
# include <math.h>

# define M_PI_LOCAL    3.14159265358979323846

/* Initialise la caméra : calcule la base orthonormée (forward/right/up). */
void    camera_init(t_camera *cam, t_vec3 pos, t_vec3 dir, double fov,
                int w, int h)
{
    cam->position = pos;
    cam->direction = vec3_normalize(dir);
    cam->fov = fov;
    cam->width = w;
    cam->height = h;
    cam->forward = cam->direction;
    cam->right = vec3_normalize(vec3_cross(cam->forward,
        (t_vec3){0.0, 1.0, 0.0}));
    cam->up = vec3_cross(cam->right, cam->forward);
}

/* Génère le rayon passant par le pixel (x, y) en tenant compte du fov. */
t_ray    camera_ray(const t_camera *cam, int x, int y)
{
    double    tan_y;
    double    tan_x;
    double    ndc_x;
    double    ndc_y;
    t_vec3    world_dir;
    t_ray    ray;

    tan_y = tan(cam->fov * 0.5 * M_PI_LOCAL / 180.0);
    tan_x = tan_y * ((double)cam->width / (double)cam->height);
    ndc_x = (2.0 * ((double)x + 0.5) / (double)cam->width - 1.0) * tan_x;
    ndc_y = (1.0 - 2.0 * ((double)y + 0.5) / (double)cam->height) * tan_y;
    world_dir = vec3_normalize(vec3_add(
        vec3_add(vec3_scale(cam->right, ndc_x),
                 vec3_scale(cam->up, ndc_y)),
        cam->forward));
    ray.origin = cam->position;
    ray.dir = world_dir;
    return (ray);
}
