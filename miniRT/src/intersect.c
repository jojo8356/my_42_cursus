/* ************************************************************************** */
/*                                                                            */
/*   intersect.c                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Intersections rayon-objet (DDA mathématique).                         */
/* ************************************************************************** */

# include "minrt.h"
# include <math.h>

void    hit_reset(t_hit *hit)
{
    hit->hit = 0;
    hit->t = INFINITY;
    hit->obj = NULL;
}

/* Transforme le rayon dans l'espace local de l'objet. */
static void    to_local(const t_obj *obj, const t_ray *ray, t_ray *local)
{
    local->origin = mat4_transform_point(obj->inv_transform, ray->origin);
    local->dir = mat4_transform_dir(obj->inv_transform, ray->dir);
    local->dir = vec3_normalize(local->dir);
}

/* Accepte le hit si plus proche, transforme point/normale en world. */
static void    accept_hit(t_obj *obj, const t_mat4 *m, double t,
                t_vec3 local_point, t_vec3 local_normal, t_hit *hit)
{
    if (t < EPS || t >= hit->t)
        return;
    hit->hit = 1;
    hit->t = t;
    hit->point = mat4_transform_point(*m, local_point);
    hit->normal = vec3_normalize(mat4_transform_dir(*m, local_normal));
    hit->obj = obj;
}

/* ---------- SPHERE (locale : centre 0,0,0) ---------- */
int    intersect_sphere(const t_obj *obj, const t_ray *ray, t_hit *hit)
{
    t_ray    local;
    double    oc_dot;
    double    disc;
    double    sqrt_disc;
    double    t0, t1, t;
    t_vec3    oc;

    to_local(obj, ray, &local);
    oc = local.origin;
    oc_dot = vec3_dot(oc, local.dir);
    disc = oc_dot * oc_dot - vec3_length_sq(oc)
        + obj->sphere_radius * obj->sphere_radius;
    if (disc < 0.0)
        return (0);
    sqrt_disc = sqrt(disc);
    t0 = -oc_dot - sqrt_disc;
    t1 = -oc_dot + sqrt_disc;
    t = t0;
    if (t < EPS)
        t = t1;
    if (t < EPS)
        return (0);
    {
        t_vec3    p = vec3_add(local.origin, vec3_scale(local.dir, t));
        t_vec3    n = vec3_normalize(p);
        accept_hit((t_obj *)obj, &obj->transform, t, p, n, hit);
    }
    return (hit->hit);
}

/* ---------- PLAN (locale : point 0,0,0, normale 0,1,0) ---------- */
int    intersect_plane(const t_obj *obj, const t_ray *ray, t_hit *hit)
{
    t_ray    local;
    double    denom;
    double    t;
    t_vec3    p;

    (void)obj;
    to_local(obj, ray, &local);
    denom = vec3_dot(local.dir, vec3_new(0.0, 1.0, 0.0));
    if (fabs(denom) < EPS)
        return (0);
    t = -local.origin.y / denom;
    if (t < EPS)
        return (0);
    p = vec3_add(local.origin, vec3_scale(local.dir, t));
    accept_hit((t_obj *)obj, &obj->transform, t, p,
        vec3_new(0.0, 1.0, 0.0), hit);
    return (hit->hit);
}

/* ---------- CYLINDRE (locale : centre 0,0,0, axe 0,1,0) ---------- */
int    intersect_cylinder(const t_obj *obj, const t_ray *ray, t_hit *hit)
{
    t_ray    local;
    double    r;
    double    a, b, c, disc, sqrt_disc;
    double    t0, t1;
    double    half_h;
    t_vec3    o_perp, d_perp;

    to_local(obj, ray, &local);
    r = obj->cyl_radius;
    half_h = obj->cyl_height / 2.0;
    o_perp = vec3_new(local.origin.x, 0.0, local.origin.z);
    d_perp = vec3_new(local.dir.x, 0.0, local.dir.z);
    a = vec3_dot(d_perp, d_perp);
    b = 2.0 * vec3_dot(o_perp, d_perp);
    c = vec3_dot(o_perp, o_perp) - r * r;

    /* côté (paroi latérale) */
    if (a > EPS)
    {
        disc = b * b - 4.0 * a * c;
        if (disc >= 0.0)
        {
            sqrt_disc = sqrt(disc);
            t0 = (-b - sqrt_disc) / (2.0 * a);
            t1 = (-b + sqrt_disc) / (2.0 * a);
            if (t0 < EPS)
                t0 = t1;
            if (t0 >= EPS)
            {
                double    y = local.origin.y + t0 * local.dir.y;
                t_vec3    p;
                t_vec3    n;
                if (fabs(y) <= half_h + EPS)
                {
                    p = vec3_add(local.origin, vec3_scale(local.dir, t0));
                    n = vec3_normalize(vec3_new(p.x, 0.0, p.z));
                    accept_hit((t_obj *)obj, &obj->transform, t0, p, n, hit);
                }
            }
        }
    }

    /* caps (disques haut/bas) */
    {
        double    t_cap;
        double    y_target[2] = {half_h, -half_h};
        for (int i = 0; i < 2; i++)
        {
            double    y0 = y_target[i];
            double    denom = local.dir.y;
            if (fabs(denom) < EPS)
                continue;
            t_cap = (y0 - local.origin.y) / denom;
            if (t_cap < EPS || t_cap >= hit->t)
                continue;
            {
                t_vec3    p = vec3_add(local.origin,
                    vec3_scale(local.dir, t_cap));
                double    rx = sqrt(p.x * p.x + p.z * p.z);
                if (rx <= r + EPS)
                {
                    t_vec3    n = vec3_new(0.0, (i == 0 ? 1.0 : -1.0), 0.0);
                    accept_hit((t_obj *)obj, &obj->transform, t_cap, p, n, hit);
                }
            }
        }
    }
    return (hit->hit);
}

/* ---------- CONE (locale : sommet 0,0,0, axe 0,1,0, ouvre vers +Y) ---------- */
int    intersect_cone(const t_obj *obj, const t_ray *ray, t_hit *hit)
{
    t_ray    local;
    double    k;        /* tan(half_angle) */
    double    a, b, c, disc, sqrt_disc;
    double    t0, t1, t;

    to_local(obj, ray, &local);
    k = tan(obj->cone_half_angle);
    a = local.dir.x * local.dir.x + local.dir.z * local.dir.z
        - k * k * local.dir.y * local.dir.y;
    b = 2.0 * (local.origin.x * local.dir.x + local.origin.z * local.dir.z
        - k * k * local.origin.y * local.dir.y);
    c = local.origin.x * local.origin.x + local.origin.z * local.origin.z
        - k * k * local.origin.y * local.origin.y;

    if (fabs(a) < EPS)
        return (0);
    disc = b * b - 4.0 * a * c;
    if (disc < 0.0)
        return (0);
    sqrt_disc = sqrt(disc);
    t0 = (-b - sqrt_disc) / (2.0 * a);
    t1 = (-b + sqrt_disc) / (2.0 * a);
    t = t0;
    if (t < EPS)
        t = t1;
    if (t < EPS)
        return (0);
    {
        double    y = local.origin.y + t * local.dir.y;
        if (y >= -EPS && y <= obj->cone_height + EPS)
        {
            t_vec3    p = vec3_add(local.origin, vec3_scale(local.dir, t));
            t_vec3    n = vec3_normalize(vec3_new(2.0 * p.x,
                -2.0 * k * k * p.y, 2.0 * p.z));
            if (vec3_dot(n, local.dir) > 0.0)
                n = vec3_neg(n);
            accept_hit((t_obj *)obj, &obj->transform, t, p, n, hit);
        }
    }
    return (hit->hit);
}

/* ---------- CUBE (locale : centré origine, côté s) ---------- */
int    intersect_cube(const t_obj *obj, const t_ray *ray, t_hit *hit)
{
    t_ray    local;
    double    half;
    double    tmin, tmax;
    int        hit_axis;
    t_vec3    n;

    to_local(obj, ray, &local);
    half = obj->cube_size / 2.0;
    tmin = -INFINITY;
    tmax = INFINITY;
    hit_axis = -1;

    /* axes x, y, z */
    {
        double    axis_t[3][2];
        for (int i = 0; i < 3; i++)
        {
            double    d = (i == 0) ? local.dir.x : (i == 1 ? local.dir.y : local.dir.z);
            double    o = (i == 0) ? local.origin.x : (i == 1 ? local.origin.y : local.origin.z);
            if (fabs(d) < EPS)
            {
                if (o < -half || o > half)
                    return (0);
                axis_t[i][0] = -INFINITY;
                axis_t[i][1] = INFINITY;
                continue;
            }
            double    t1 = (-half - o) / d;
            double    t2 = (half - o) / d;
            if (t1 > t2)
            {
                double    tmp = t1;
                t1 = t2;
                t2 = tmp;
            }
            axis_t[i][0] = t1;
            axis_t[i][1] = t2;
        }
        tmin = fmax(fmax(axis_t[0][0], axis_t[1][0]), axis_t[2][0]);
        tmax = fmin(fmin(axis_t[0][1], axis_t[1][1]), axis_t[2][1]);
        if (tmin > tmax || tmax < EPS)
            return (0);
        /* déterminer la face d'entrée */
        double    best = -INFINITY;
        for (int i = 0; i < 3; i++)
        {
            if (fabs(local.dir.x) < EPS && i == 0)
                continue;
            if (fabs(local.dir.y) < EPS && i == 1)
                continue;
            if (fabs(local.dir.z) < EPS && i == 2)
                continue;
            if (axis_t[i][0] > best)
            {
                best = axis_t[i][0];
                hit_axis = i;
            }
        }
    }
    (void)hit_axis;
    {
        double    t = tmin;
        if (t < EPS)
            t = tmax;
        if (t < EPS)
            return (0);
        t_vec3    p = vec3_add(local.origin, vec3_scale(local.dir, t));
        /* normale = direction de la face la plus proche (entrée) */
        n = vec3_new(0.0, 0.0, 0.0);
        if (fabs(p.x) >= fabs(p.y) - EPS && fabs(p.x) >= fabs(p.z) - EPS)
            n = vec3_new(p.x >= 0.0 ? 1.0 : -1.0, 0.0, 0.0);
        else if (fabs(p.y) >= fabs(p.z) - EPS)
            n = vec3_new(0.0, p.y >= 0.0 ? 1.0 : -1.0, 0.0);
        else
            n = vec3_new(0.0, 0.0, p.z >= 0.0 ? 1.0 : -1.0);
        accept_hit((t_obj *)obj, &obj->transform, t, p, n, hit);
    }
    return (hit->hit);
}

/* ---------- TRIANGLE (locale : Möller-Trumbore) ---------- */
int    intersect_triangle(const t_obj *obj, const t_ray *ray, t_hit *hit)
{
    t_ray    local;
    t_vec3    edge1, edge2, p, tvec, qvec;
    double    det, u, v, t;

    to_local(obj, ray, &local);
    edge1 = vec3_sub(obj->tri_v1, obj->tri_v0);
    edge2 = vec3_sub(obj->tri_v2, obj->tri_v0);
    p = vec3_cross(local.dir, edge2);
    det = vec3_dot(edge1, p);
    if (fabs(det) < EPS)
        return (0);
    det = 1.0 / det;
    tvec = vec3_sub(local.origin, obj->tri_v0);
    u = vec3_dot(tvec, p) * det;
    if (u < 0.0 || u > 1.0)
        return (0);
    qvec = vec3_cross(tvec, edge1);
    v = vec3_dot(local.dir, qvec) * det;
    if (v < 0.0 || u + v > 1.0)
        return (0);
    t = vec3_dot(edge2, qvec) * det;
    if (t < EPS || t >= hit->t)
        return (0);
    {
        t_vec3    p_hit = vec3_add(local.origin, vec3_scale(local.dir, t));
        t_vec3    n = vec3_normalize(vec3_cross(edge1, edge2));
        if (vec3_dot(n, local.dir) > 0.0)
            n = vec3_neg(n);
        accept_hit((t_obj *)obj, &obj->transform, t, p_hit, n, hit);
    }
    return (hit->hit);
}

int    obj_intersect(const t_obj *obj, const t_ray *ray, t_hit *hit)
{
    if (!hit->hit)
        hit->t = INFINITY;
    switch (obj->type)
    {
        case OBJ_SPHERE:
            intersect_sphere(obj, ray, hit);
            break;
        case OBJ_PLANE:
            intersect_plane(obj, ray, hit);
            break;
        case OBJ_CYLINDER:
            intersect_cylinder(obj, ray, hit);
            break;
        case OBJ_CONE:
            intersect_cone(obj, ray, hit);
            break;
        case OBJ_CUBE:
            intersect_cube(obj, ray, hit);
            break;
        case OBJ_TRIANGLE:
            intersect_triangle(obj, ray, hit);
            break;
        default:
            break;
    }
    return (hit->hit);
}

int    scene_intersect(const t_scene *scene, const t_ray *ray, t_hit *hit)
{
    hit_reset(hit);
    for (size_t i = 0; i < scene->obj_count; i++)
        obj_intersect(&scene->objects[i], ray, hit);
    return (hit->hit);
}
