/*
** math.c — 3D math: rotation and isometric projection.
**
** All functions here are pure (no mlx calls) and fully unit-testable in a
** headless environment.
*/

# include "fdf.h"
# include <math.h>

/* ----------------------------------------------------------------------- */
double radians(double degrees)
{
    return degrees * M_PI / 180.0;
}

/* ----------------------------------------------------------------------- *
** Rotate a point around the X axis by `angle` radians.
** ----------------------------------------------------------------------- */
void rotate_x(double *x, double *y, double *z, double angle)
{
    double cos_a = cos(angle);
    double sin_a = sin(angle);
    double old_y = *y;
    double old_z = *z;
    (void)x;  /* x is unchanged when rotating around X axis. */
    *y = old_y * cos_a - old_z * sin_a;
    *z = old_y * sin_a + old_z * cos_a;
}

/* ----------------------------------------------------------------------- *
** Rotate a point around the Y axis by `angle` radians.
** ----------------------------------------------------------------------- */
void rotate_y(double *x, double *y, double *z, double angle)
{
    double cos_a = cos(angle);
    double sin_a = sin(angle);
    double old_x = *x;
    double old_z = *z;
    (void)y;  /* y is unchanged when rotating around Y axis. */
    *x = old_x * cos_a + old_z * sin_a;
    *z = -old_x * sin_a + old_z * cos_a;
}

/* ----------------------------------------------------------------------- *
** Rotate a point around the Z axis by `angle` radians.
** ----------------------------------------------------------------------- */
void rotate_z(double *x, double *y, double *z, double angle)
{
    double cos_a = cos(angle);
    double sin_a = sin(angle);
    double old_x = *x;
    double old_y = *y;
    (void)z;  /* z is unchanged when rotating around Z axis. */
    *x = old_x * cos_a - old_y * sin_a;
    *y = old_x * sin_a + old_y * cos_a;
}

/* ----------------------------------------------------------------------- *
** Isometric projection: convert 3D (x, y, z) to 2D screen coordinates.
**
**   screen_x = (x - y) * cos(iso_x) * scale + offset_x
**   screen_y = (x + y) * sin(iso_x) * scale - z * scale + offset_y
**
** With iso_x = 30° and iso_y = 45°, this gives the classic isometric look.
** ----------------------------------------------------------------------- */
void project_iso(double x, double y, double z,
                 double iso_x, double iso_y,
                 double scale, double *out_x, double *out_y)
{
    double cos_x = cos(iso_x);
    double sin_x = sin(iso_x);

    (void)iso_y;  /* iso_y reserved for future use. */

    /* Standard simplified isometric projection. */
    /* Higher z → smaller screen y (up on screen). */
    *out_x = (x - y) * cos_x * scale;
    *out_y = (x + y) * sin_x * scale - z * scale;
}

/* ----------------------------------------------------------------------- *
** Apply full transformation pipeline: rotate → project → translate.
** Output is in screen pixel coordinates.
** ----------------------------------------------------------------------- */
void apply_transform(t_env *env, double x, double y, double z,
                     int *out_x, int *out_y)
{
    double rx = x, ry = y, rz = z;

    /* Apply rotations in order: Z, Y, X. */
    rotate_z(&rx, &ry, &rz, env->rot_z);
    rotate_y(&rx, &ry, &rz, env->rot_y);
    rotate_x(&rx, &ry, &rz, env->rot_x);

    /* Isometric projection. */
    double sx, sy;
    project_iso(rx, ry, rz, env->iso_x, env->iso_y, env->scale, &sx, &sy);

    /* Add offset and convert to integer screen coordinates. */
    /* Center the map in the window. */
    double center_x = (env->map->width - 1) * env->scale * cos(env->iso_x);
    double center_y = (env->map->width - 1) * env->scale * sin(env->iso_x)
                      + (env->map->height - 1) * env->scale * sin(env->iso_x);

    *out_x = (int)(sx - center_x + WIN_W / 2 + env->offset_x);
    *out_y = (int)(sy - center_y / 2 + WIN_H / 2 + env->offset_y);
}
