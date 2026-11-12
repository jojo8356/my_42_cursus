/*
** color.c — Color computation and interpolation.
**
** All functions here are pure (no mlx calls), fully unit-testable headless.
*/

# include "fdf.h"
# include <math.h>

/* ----------------------------------------------------------------------- *
** color_lerp — linear interpolation between two 0xRRGGBB colors.
**
**   percent = 0.0  →  c1
**   percent = 1.0  →  c2
**   percent = 0.5  →  average of c1 and c2
**
** Uses the ft_lint-style formula to avoid precision loss:
**   c1 * (1 - p) + c2 * p
** ----------------------------------------------------------------------- */
int color_lerp(int c1, int c2, double percent)
{
    if (percent <= 0.0)
        return c1;
    if (percent >= 1.0)
        return c2;
    if (c1 == c2)
        return c1;

    int r1 = (c1 >> 16) & 0xFF;
    int g1 = (c1 >> 8) & 0xFF;
    int b1 = c1 & 0xFF;

    int r2 = (c2 >> 16) & 0xFF;
    int g2 = (c2 >> 8) & 0xFF;
    int b2 = c2 & 0xFF;

    int r = (int)(r1 * (1.0 - percent) + r2 * percent);
    int g = (int)(g1 * (1.0 - percent) + g2 * percent);
    int b = (int)(b1 * (1.0 - percent) + b2 * percent);

    return (r << 16) | (g << 8) | b;
}

/* ----------------------------------------------------------------------- *
** rainbow_color — map a 0.0–1.0 value to a rainbow color (HSL → RGB).
** ----------------------------------------------------------------------- */
static int rainbow_color(double percent);

/* ----------------------------------------------------------------------- *
** compute_color — map a height value to a color based on the current
** color mode and the map's min/max z values.
**
**   percent = (z - min_z) / (max_z - min_z)
**   color   = color_lerp(color_min, color_max, percent)
** ----------------------------------------------------------------------- */
int compute_color(double z, t_env *env)
{
    t_map *map = env->map;
    double range = map->max_z - map->min_z;
    double percent;

    if (range == 0)
        percent = 0.0;
    else
        percent = (z - map->min_z) / range;

    if (percent < 0.0)
        percent = 0.0;
    if (percent > 1.0)
        percent = 1.0;

    switch (env->color_mode)
    {
        case MODE_GRAYSCALE:
            /* White → Black based on height. */
            return color_lerp(COLOR_WHITE, COLOR_BLACK, percent);
        case MODE_BLUE:
            /* Blue → White. */
            return color_lerp(COLOR_BLUE, COLOR_WHITE, percent);
        case MODE_CYAN:
            /* Cyan → White. */
            return color_lerp(COLOR_CYAN, COLOR_WHITE, percent);
        case MODE_RAINBOW:
            /* Rainbow: map height to hue. */
            return rainbow_color(percent);
        case MODE_GRADIENT:
        default:
            /* Red → Green (default). */
            return color_lerp(COLOR_RED, COLOR_GREEN, percent);
    }
}

/* ----------------------------------------------------------------------- *
** rainbow_color — map a 0.0–1.0 value to a rainbow color (HSL → RGB).
** ----------------------------------------------------------------------- */
static int rainbow_color(double percent)
{
    double h = percent * 5.0;   /* 0–5 for red→yellow→green→cyan→blue→magenta */
    double c = 1.0;
    double x = c * (1.0 - fabs(fmod(h, 2.0) - 1.0));
    int r, g, b;

    if (h < 1.0)      { r = (int)(c * 255); g = (int)(x * 255); b = 0; }
    else if (h < 2.0) { r = (int)(x * 255); g = (int)(c * 255); b = 0; }
    else if (h < 3.0) { r = 0; g = (int)(c * 255); b = (int)(x * 255); }
    else if (h < 4.0) { r = 0; g = (int)(x * 255); b = (int)(c * 255); }
    else if (h < 5.0) { r = (int)(x * 255); g = 0; b = (int)(c * 255); }
    else              { r = (int)(c * 255); g = 0; b = (int)(x * 255); }

    return (r << 16) | (g << 8) | b;
}

/* ----------------------------------------------------------------------- */
void cycle_color_mode(t_env *env)
{
    env->color_mode = (t_color_mode)((env->color_mode + 1) % MODE_COUNT);
}
