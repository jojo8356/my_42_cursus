/* ************************************************************************** */
/*                                                                            */
/*   matrix.c                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Matrices 4x4 (m[row][col], convention colonne).                      */
/* ************************************************************************** */

# include "minrt.h"
# include <math.h>

t_mat4    mat4_identity(void)
{
    t_mat4    m;
    int        i, j;

    for (i = 0; i < 4; i++)
        for (j = 0; j < 4; j++)
            m.m[i][j] = (i == j) ? 1.0 : 0.0;
    return (m);
}

t_mat4    mat4_multiply(t_mat4 a, t_mat4 b)
{
    t_mat4    r;
    int        i, j, k;

    for (i = 0; i < 4; i++)
        for (j = 0; j < 4; j++)
        {
            r.m[i][j] = 0.0;
            for (k = 0; k < 4; k++)
                r.m[i][j] += a.m[i][k] * b.m[k][j];
        }
    return (r);
}

t_vec3    mat4_transform_point(t_mat4 m, t_vec3 p)
{
    double    x, y, z, w;

    x = m.m[0][0] * p.x + m.m[0][1] * p.y + m.m[0][2] * p.z + m.m[0][3];
    y = m.m[1][0] * p.x + m.m[1][1] * p.y + m.m[1][2] * p.z + m.m[1][3];
    z = m.m[2][0] * p.x + m.m[2][1] * p.y + m.m[2][2] * p.z + m.m[2][3];
    w = m.m[3][0] * p.x + m.m[3][1] * p.y + m.m[3][2] * p.z + m.m[3][3];
    if (w != 1.0 && w != 0.0)
    {
        x /= w;
        y /= w;
        z /= w;
    }
    return (vec3_new(x, y, z));
}

t_vec3    mat4_transform_dir(t_mat4 m, t_vec3 d)
{
    double    x, y, z;

    x = m.m[0][0] * d.x + m.m[0][1] * d.y + m.m[0][2] * d.z;
    y = m.m[1][0] * d.x + m.m[1][1] * d.y + m.m[1][2] * d.z;
    z = m.m[2][0] * d.x + m.m[2][1] * d.y + m.m[2][2] * d.z;
    return (vec3_new(x, y, z));
}

t_mat4    mat4_translate(double x, double y, double z)
{
    t_mat4    m;

    m = mat4_identity();
    m.m[0][3] = x;
    m.m[1][3] = y;
    m.m[2][3] = z;
    return (m);
}

t_mat4    mat4_rotate_x(double angle)
{
    double    c, s;
    t_mat4    m;

    c = cos(angle * M_PI / 180.0);
    s = sin(angle * M_PI / 180.0);
    m = mat4_identity();
    m.m[1][1] = c;
    m.m[1][2] = -s;
    m.m[2][1] = s;
    m.m[2][2] = c;
    return (m);
}

t_mat4    mat4_rotate_y(double angle)
{
    double    c, s;
    t_mat4    m;

    c = cos(angle * M_PI / 180.0);
    s = sin(angle * M_PI / 180.0);
    m = mat4_identity();
    m.m[0][0] = c;
    m.m[0][2] = s;
    m.m[2][0] = -s;
    m.m[2][2] = c;
    return (m);
}

t_mat4    mat4_rotate_z(double angle)
{
    double    c, s;
    t_mat4    m;

    c = cos(angle * M_PI / 180.0);
    s = sin(angle * M_PI / 180.0);
    m = mat4_identity();
    m.m[0][0] = c;
    m.m[0][1] = -s;
    m.m[1][0] = s;
    m.m[1][1] = c;
    return (m);
}

t_mat4    mat4_rotate(t_mat4 m, char axis, double angle)
{
    t_mat4    r;

    if (axis == 'x' || axis == 'X')
        r = mat4_rotate_x(angle);
    else if (axis == 'y' || axis == 'Y')
        r = mat4_rotate_y(angle);
    else
        r = mat4_rotate_z(angle);
    return (mat4_multiply(r, m));
}

t_mat4    mat4_scale(double x, double y, double z)
{
    t_mat4    m;

    m = mat4_identity();
    m.m[0][0] = x;
    m.m[1][1] = y;
    m.m[2][2] = z;
    return (m);
}

/* Rotation autour d'un axe unitaire (formule de Rodrigues), angle en degres. */
t_mat4    mat4_rotation_axis(t_vec3 axis, double angle)
{
    double    c, s, oc;
    t_vec3    k;
    t_mat4    m;
    double    kx, ky, kz;

    k = vec3_normalize(axis);
    kx = k.x;
    ky = k.y;
    kz = k.z;
    c = cos(angle * M_PI / 180.0);
    s = sin(angle * M_PI / 180.0);
    oc = 1.0 - c;

    m = mat4_identity();
    m.m[0][0] = c + kx * kx * oc;
    m.m[0][1] = kx * ky * oc - kz * s;
    m.m[0][2] = kx * kz * oc + ky * s;
    m.m[1][0] = ky * kx * oc + kz * s;
    m.m[1][1] = c + ky * ky * oc;
    m.m[1][2] = ky * kz * oc - kx * s;
    m.m[2][0] = kz * kx * oc - ky * s;
    m.m[2][1] = kz * ky * oc + kx * s;
    m.m[2][2] = c + kz * kz * oc;
    return (m);
}

t_mat4    mat4_transpose(t_mat4 m)
{
    t_mat4    r;
    int        i, j;

    for (i = 0; i < 4; i++)
        for (j = 0; j < 4; j++)
            r.m[i][j] = m.m[j][i];
    return (r);
}

/* Inverse générale 4x4 par élimination de Gauss-Jordan sur [M|I].
   Retourne 1 si déterminant non nul, 0 sinon (out non modifié). */
int        mat4_invert(t_mat4 m, t_mat4 *out)
{
    double    aug[4][8];
    int        i, j, k;
    double    pivot;

    for (i = 0; i < 4; i++)
    {
        for (j = 0; j < 4; j++)
            aug[i][j] = m.m[i][j];
        for (j = 0; j < 4; j++)
            aug[i][j + 4] = (i == j) ? 1.0 : 0.0;
    }

    for (i = 0; i < 4; i++)
    {
        /* recherche du pivot */
        pivot = aug[i][i];
        k = i;
        for (j = i + 1; j < 4; j++)
            if (fabs(aug[j][i]) > fabs(pivot))
            {
                pivot = aug[j][i];
                k = j;
            }
        if (fabs(pivot) < 1e-12)
            return (0);
        if (k != i)
            for (j = 0; j < 8; j++)
            {
                double    tmp = aug[i][j];
                aug[i][j] = aug[k][j];
                aug[k][j] = tmp;
            }
        /* normalisation */
        pivot = aug[i][i];
        for (j = 0; j < 8; j++)
            aug[i][j] /= pivot;
        /* élimination */
        for (k = 0; k < 4; k++)
        {
            if (k == i)
                continue;
            double    factor = aug[k][i];
            for (j = 0; j < 8; j++)
                aug[k][j] -= factor * aug[i][j];
        }
    }

    for (i = 0; i < 4; i++)
        for (j = 0; j < 4; j++)
            out->m[i][j] = aug[i][j + 4];
    return (1);
}

t_mat4    mat4_inverse(t_mat4 m)
{
    t_mat4    out;

    if (!mat4_invert(m, &out))
        return (mat4_identity());
    return (out);
}
