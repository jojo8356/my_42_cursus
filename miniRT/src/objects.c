/* ************************************************************************** */
/*                                                                            */
/*   objects.c                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Construction des objets (géométrie locale + transform).               */
/* ************************************************************************** */

# include "minrt.h"
# include <math.h>

/* Rotation mapping `from` vers `to` (vecteurs normalisés). */
static t_mat4    rotation_from_to(t_vec3 from, t_vec3 to)
{
    double    dot;

    from = vec3_normalize(from);
    to = vec3_normalize(to);
    dot = vec3_dot(from, to);
    if (dot > 0.999999)
        return (mat4_identity());
    if (dot < -0.999999)
    {
        t_vec3    axis = vec3_cross(from, vec3_new(1.0, 0.0, 0.0));
        if (vec3_length_sq(axis) < EPS)
            axis = vec3_cross(from, vec3_new(0.0, 1.0, 0.0));
        axis = vec3_normalize(axis);
        return (mat4_rotation_axis(axis, 180.0));
    }
    {
        t_vec3    axis = vec3_normalize(vec3_cross(from, to));
        double    angle = acos(dot) * 180.0 / M_PI;
        return (mat4_rotation_axis(axis, angle));
    }
}

void    obj_init(t_obj *obj, int type)
{
    ft_bzero(obj, sizeof(*obj));
    obj->type = type;
    obj->transform = mat4_identity();
    obj->inv_transform = mat4_identity();
    obj->color = color_new(1.0, 1.0, 1.0);
    obj->mat.specular = DEFAULT_SPECULAR;
    obj->mat.reflection = DEFAULT_REFLECTION;
    obj->mat.transparency = DEFAULT_TRANSP;
    obj->mat.refr_index = DEFAULT_RI;
    obj->mat.shininess = DEFAULT_SHININESS;
    obj->mat.pattern = PAT_NONE;
}

void    obj_compute_inverse(t_obj *obj)
{
    obj->inv_transform = mat4_inverse(obj->transform);
}

/* Applique une transform en préfixe : transform = t * transform. */
void    obj_apply_transform(t_obj *obj, const t_mat4 *t)
{
    obj->transform = mat4_multiply(*t, obj->transform);
    obj_compute_inverse(obj);
}

t_obj    *obj_sphere(t_vec3 center, double diameter, t_color color)
{
    t_obj    *o;
    t_mat4    t;

    o = (t_obj *)xcalloc(1, sizeof(t_obj));
    obj_init(o, OBJ_SPHERE);
    o->sphere_center = vec3_new(0.0, 0.0, 0.0);
    o->sphere_radius = diameter / 2.0;
    o->color = color;
    t = mat4_translate(center.x, center.y, center.z);
    obj_apply_transform(o, &t);
    return (o);
}

t_obj    *obj_plane(t_vec3 point, t_vec3 normal, t_color color)
{
    t_obj    *o;
    t_mat4    align;
    t_mat4    base;

    o = (t_obj *)xcalloc(1, sizeof(t_obj));
    obj_init(o, OBJ_PLANE);
    o->plane_point = vec3_new(0.0, 0.0, 0.0);
    o->plane_normal = vec3_new(0.0, 1.0, 0.0);
    o->color = color;
    align = rotation_from_to(vec3_new(0.0, 1.0, 0.0), normal);
    base = mat4_multiply(align, mat4_translate(point.x, point.y, point.z));
    obj_apply_transform(o, &base);
    return (o);
}

t_obj    *obj_cylinder(t_vec3 center, t_vec3 axis, double diameter,
                double height, t_color color)
{
    t_obj    *o;
    t_mat4    align;
    t_mat4    base;

    o = (t_obj *)xcalloc(1, sizeof(t_obj));
    obj_init(o, OBJ_CYLINDER);
    o->cyl_center = vec3_new(0.0, 0.0, 0.0);
    o->cyl_axis = vec3_new(0.0, 1.0, 0.0);
    o->cyl_radius = diameter / 2.0;
    o->cyl_height = height;
    o->color = color;
    align = rotation_from_to(vec3_new(0.0, 1.0, 0.0), axis);
    base = mat4_multiply(align, mat4_translate(center.x, center.y, center.z));
    obj_apply_transform(o, &base);
    return (o);
}

t_obj    *obj_cone(t_vec3 apex, t_vec3 axis, double half_angle_deg,
                double height, t_color color)
{
    t_obj    *o;
    t_mat4    align;
    t_mat4    base;

    o = (t_obj *)xcalloc(1, sizeof(t_obj));
    obj_init(o, OBJ_CONE);
    /* apex local à l'origine, axe (0,1,0), le cône s'ouvre vers +Y */
    o->cone_apex = vec3_new(0.0, 0.0, 0.0);
    o->cone_axis = vec3_new(0.0, 1.0, 0.0);
    o->cone_half_angle = half_angle_deg * M_PI / 180.0;
    o->cone_height = height;
    o->color = color;
    align = rotation_from_to(vec3_new(0.0, 1.0, 0.0), axis);
    base = mat4_multiply(align, mat4_translate(apex.x, apex.y, apex.z));
    obj_apply_transform(o, &base);
    return (o);
}

t_obj    *obj_cube(double size, t_color color)
{
    t_obj    *o;

    o = (t_obj *)xcalloc(1, sizeof(t_obj));
    obj_init(o, OBJ_CUBE);
    o->cube_size = size;
    o->color = color;
    /* centré sur l'origine, identité (position donnée via tr si besoin) */
    return (o);
}

t_obj    *obj_cube_at(t_vec3 pos, t_vec3 orient, double size, t_color color)
{
    t_obj    *o;
    t_mat4    rot;
    t_mat4    base;

    o = obj_cube(size, color);
    rot = rotation_from_to(vec3_new(0.0, 1.0, 0.0), orient);
    base = mat4_multiply(mat4_translate(pos.x, pos.y, pos.z), rot);
    obj_apply_transform(o, &base);
    return (o);
}

t_obj    *obj_triangle(t_vec3 v0, t_vec3 v1, t_vec3 v2, t_color color)
{
    t_obj    *o;

    o = (t_obj *)xcalloc(1, sizeof(t_obj));
    obj_init(o, OBJ_TRIANGLE);
    o->tri_v0 = v0;
    o->tri_v1 = v1;
    o->tri_v2 = v2;
    o->color = color;
    return (o);
}

void    obj_free(t_obj *obj)
{
    if (obj)
        free(obj);
}
