/* ************************************************************************** */
/*                                                                            */
/*   lighting.c                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Éclairage Phong, ombres, motifs.                                      */
/* ************************************************************************** */

# include "minrt.h"
# include <math.h>

static t_color    color_lerp(t_color a, t_color b, double t)
{
    return (color_new(a.r + (b.r - a.r) * t,
                      a.g + (b.g - a.g) * t,
                      a.b + (b.b - a.b) * t));
}

t_color    apply_material(const t_color *base, const t_material *mat,
                const t_vec3 *point)
{
    double    t;
    double    u;
    double    v;
    double    scale;

    (void)base;
    if (mat->tex && mat->tex->data)
    {
        scale = mat->pattern_a > 0.0 ? mat->pattern_a : 1.0;
        u = fmod(point->x * scale, 1.0);
        v = fmod(point->z * scale, 1.0);
        if (u < 0.0)
            u += 1.0;
        if (v < 0.0)
            v += 1.0;
        return (texture_sample(mat->tex, u, v));
    }
    if (mat->pattern == PAT_NONE)
        return (*base);
    switch (mat->pattern)
    {
        case PAT_STRIPE:
            t = (fmod(floor(point->x * mat->pattern_a), 2.0) == 0.0) ? 0.0 : 1.0;
            return (color_lerp(mat->pattern_c1, mat->pattern_c2, t));
        case PAT_CHECKER:
            t = (fmod(floor(point->x * mat->pattern_a)
                + floor(point->z * mat->pattern_a), 2.0) == 0.0) ? 0.0 : 1.0;
            return (color_lerp(mat->pattern_c1, mat->pattern_c2, t));
        case PAT_GRADIENT:
            t = fmin(fmax(point->y * mat->pattern_a + mat->pattern_b, 0.0), 1.0);
            return (color_lerp(mat->pattern_c1, mat->pattern_c2, t));
        case PAT_RING:
            t = sin(vec3_length(*point) * mat->pattern_a) * 0.5 + 0.5;
            return (color_lerp(mat->pattern_c1, mat->pattern_c2, t));
        case PAT_TEST:
            t = (floor(point->x * 2.0) + floor(point->z * 2.0)) * 0.25 + 0.5;
            return (color_lerp(mat->pattern_c1, mat->pattern_c2, t));
        default:
            return (*base);
    }
}

int    in_shadow(const t_scene *scene, const t_vec3 *point,
                const t_vec3 *light_dir)
{
    t_ray    shadow_ray;
    t_hit    hit;

    shadow_ray.origin = vec3_add(*point, vec3_scale(*light_dir, EPS * 10.0));
    shadow_ray.dir = *light_dir;
    return (scene_intersect(scene, &shadow_ray, &hit));
}

t_color    phong_shade(const t_scene *scene, const t_hit *hit,
                const t_vec3 *view_dir, int depth)
{
    const t_obj    *obj = hit->obj;
    t_color    base;
    t_color    ambient;
    t_color    diffuse;
    t_color    specular;
    t_color    result;
    t_vec3    l;
    t_vec3    r;
    double    diff;
    double    spec;

    (void)depth;
    base = apply_material(&obj->color, &obj->mat, &hit->point);
    l = vec3_neg(scene->light.direction);

    ambient = color_scale(scene->ambient, scene->ambient_ratio);
    result = color_multiply(ambient, base);

    diff = fmax(vec3_dot(hit->normal, l), 0.0);
    if (!in_shadow(scene, &hit->point, &l) && diff > 0.0)
    {
        diffuse = color_scale(base, diff * scene->light.ratio);
        result = color_add(result, diffuse);

        r = vec3_reflect(vec3_neg(l), hit->normal);
        spec = fmax(vec3_dot(r, *view_dir), 0.0);
        spec = pow(spec, obj->mat.shininess);
        specular = color_scale(scene->light.color,
            spec * obj->mat.specular * scene->light.ratio);
        result = color_add(result, specular);
    }

    return (color_clamp(result));
}

t_color    trace_ray(const t_scene *scene, const t_ray *ray, int depth)
{
    t_hit    hit;
    t_color    color;
    t_color    refl;
    t_color    trans;
    t_vec3    view_dir;
    t_vec3    n;
    t_vec3    r;
    double    eta;

    if (depth > MAX_DEPTH)
        return (color_new(0.0, 0.0, 0.0));
    if (!scene_intersect(scene, ray, &hit))
        return (color_scale(scene->ambient, scene->ambient_ratio));

    view_dir = vec3_normalize(vec3_sub(ray->origin, hit.point));
    color = phong_shade(scene, &hit, &view_dir, depth);
    n = hit.normal;

    /* réflexion pure */
    if (hit.obj->mat.reflection > 0.0)
    {
        t_ray    rr;

        rr.origin = vec3_add(hit.point, vec3_scale(n, EPS * 10.0));
        rr.dir = vec3_normalize(vec3_reflect(ray->dir, n));
        refl = trace_ray(scene, &rr, depth + 1);
        color = color_add(color_scale(color, 1.0 - hit.obj->mat.reflection),
            color_scale(refl, hit.obj->mat.reflection));
    }

    /* transparence / réfraction */
    if (hit.obj->mat.transparency > 0.0)
    {
        t_ray    tr;

        eta = (vec3_dot(ray->dir, n) < 0.0)
            ? 1.0 / hit.obj->mat.refr_index : hit.obj->mat.refr_index;
        r = vec3_refract(ray->dir, n, eta);
        if (vec3_length_sq(r) > EPS)
        {
            tr.origin = vec3_add(hit.point,
                vec3_scale(vec3_normalize(r), EPS * 10.0));
            tr.dir = vec3_normalize(r);
            trans = trace_ray(scene, &tr, depth + 1);
            color = color_add(color_scale(color,
                1.0 - hit.obj->mat.transparency),
                color_scale(trans, hit.obj->mat.transparency));
        }
    }

    return (color_clamp(color));
}
