/* ************************************************************************** */
/*                                                                            */
/*   vector.c                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Opérations sur les vecteurs 3D.                                        */
/* ************************************************************************** */

# include "minrt.h"
# include <math.h>

t_vec3    vec3_new(double x, double y, double z)
{
    t_vec3    v;

    v.x = x;
    v.y = y;
    v.z = z;
    return (v);
}

t_vec3    vec3_add(t_vec3 a, t_vec3 b)
{
    return (vec3_new(a.x + b.x, a.y + b.y, a.z + b.z));
}

t_vec3    vec3_sub(t_vec3 a, t_vec3 b)
{
    return (vec3_new(a.x - b.x, a.y - b.y, a.z - b.z));
}

t_vec3    vec3_scale(t_vec3 v, double s)
{
    return (vec3_new(v.x * s, v.y * s, v.z * s));
}

t_vec3    vec3_neg(t_vec3 v)
{
    return (vec3_new(-v.x, -v.y, -v.z));
}

double    vec3_dot(t_vec3 a, t_vec3 b)
{
    return (a.x * b.x + a.y * b.y + a.z * b.z);
}

t_vec3    vec3_cross(t_vec3 a, t_vec3 b)
{
    return (vec3_new(a.y * b.z - a.z * b.y,
                     a.z * b.x - a.x * b.z,
                     a.x * b.y - a.y * b.x));
}

double    vec3_length_sq(t_vec3 v)
{
    return (v.x * v.x + v.y * v.y + v.z * v.z);
}

double    vec3_length(t_vec3 v)
{
    return (sqrt(vec3_length_sq(v)));
}

t_vec3    vec3_normalize(t_vec3 v)
{
    double    len;

    len = vec3_length(v);
    if (len < EPS)
        return (vec3_new(0.0, 0.0, 0.0));
    return (vec3_scale(v, 1.0 / len));
}

/* R = I - 2 (I·N) N. I pointe vers la surface, N sortante. */
t_vec3    vec3_reflect(t_vec3 i, t_vec3 n)
{
    double    d;

    d = 2.0 * vec3_dot(i, n);
    return (vec3_sub(i, vec3_scale(n, d)));
}

/* Réfraction (Snell). eta = n1/n2. I vers la surface, N sortante.
   Retourne (0,0,0) en réflexion totale interne. */
t_vec3    vec3_refract(t_vec3 i, t_vec3 n, double eta)
{
    double    cos_i;
    double    k;
    t_vec3    r;

    cos_i = -vec3_dot(i, n);
    if (cos_i < 0.0)
    {
        n = vec3_neg(n);
        cos_i = -cos_i;
    }
    k = 1.0 - eta * eta * (1.0 - cos_i * cos_i);
    if (k < 0.0)
        return (vec3_new(0.0, 0.0, 0.0));
    r = vec3_add(vec3_scale(i, eta),
        vec3_scale(n, eta * cos_i - sqrt(k)));
    return (r);
}

double    vec3_dist(t_vec3 a, t_vec3 b)
{
    return (vec3_length(vec3_sub(a, b)));
}

t_vec3    vec3_lerp(t_vec3 a, t_vec3 b, double t)
{
    return (vec3_add(vec3_scale(a, 1.0 - t), vec3_scale(b, t)));
}

t_vec3    vec3_min(t_vec3 a, t_vec3 b)
{
    return (vec3_new(fmin(a.x, b.x), fmin(a.y, b.y), fmin(a.z, b.z)));
}

t_vec3    vec3_max(t_vec3 a, t_vec3 b)
{
    return (vec3_new(fmax(a.x, b.x), fmax(a.y, b.y), fmax(a.z, b.z)));
}
