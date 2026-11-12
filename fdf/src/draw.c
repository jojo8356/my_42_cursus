/*
** draw.c — Line drawing and grid rendering.
**
** bresenham_line and put_pixel are the core drawing primitives.
** draw_grid iterates over the map and draws all grid lines.
**
** bresenham_line and color computation are testable headless (they only
** write to the image buffer). put_pixel needs mlx for the real render.
*/

# include "fdf.h"

/* ----------------------------------------------------------------------- *
** put_pixel — write a single 0xRRGGBB pixel into the image buffer.
** ----------------------------------------------------------------------- */
void put_pixel(t_env *env, int x, int y, int color)
{
    if (x < 0 || x >= WIN_W || y < 0 || y >= WIN_H)
        return;

    int r = (color >> 16) & 0xFF;
    int g = (color >> 8) & 0xFF;
    int b = color & 0xFF;
    int bpp = env->bits_per_pixel / 8;
    int offset = y * env->line_size + x * bpp;

    env->img_data[offset]     = (char)r;
    env->img_data[offset + 1] = (char)g;
    env->img_data[offset + 2] = (char)b;
    env->img_data[offset + 3] = 255;
}

/* ----------------------------------------------------------------------- *
** bresenham_line — draw a line from (x0, y0) to (x1, y1) using Bresenham's
** algorithm. Writes pixels directly to the image buffer.
**
** This is the workhorse of wireframe rendering. All grid lines are drawn
** with this single function.
** ----------------------------------------------------------------------- */
void bresenham_line(t_env *env, int x0, int y0, int x1, int y1, int color)
{
    int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx - dy;

    while (1)
    {
        put_pixel(env, x0, y0, color);

        if (x0 == x1 && y0 == y1)
            break;

        int e2 = 2 * err;
        if (e2 > -dy)
        {
            err -= dy;
            x0 += sx;
        }
        if (e2 < dx)
        {
            err += dx;
            y0 += sy;
        }
    }
}

/* ----------------------------------------------------------------------- *
** draw_grid — iterate over the map and draw all grid lines.
**
** Draws horizontal lines (left-to-right) and vertical lines (top-to-bottom)
** between adjacent points. Each line's color is computed from the height
** of the starting point.
** ----------------------------------------------------------------------- */
void draw_grid(t_env *env)
{
    t_map *map = env->map;

    for (int y = 0; y < map->height; y++)
    {
        for (int x = 0; x < map->width; x++)
        {
            int idx = y * map->width + x;
            double z = map->grid[idx].z;
            int color = compute_color(z, env);

            int x0, y0, x1, y1;

            /* Transform current point. */
            apply_transform(env, map->grid[idx].x, map->grid[idx].y, z,
                            &x0, &y0);

            /* Draw horizontal line to the right neighbor. */
            if (x + 1 < map->width)
            {
                int idx2 = idx + 1;
                double z2 = map->grid[idx2].z;
                apply_transform(env, map->grid[idx2].x, map->grid[idx2].y, z2,
                                &x1, &y1);
                bresenham_line(env, x0, y0, x1, y1, color);
            }

            /* Draw vertical line to the bottom neighbor. */
            if (y + 1 < map->height)
            {
                int idx2 = idx + map->width;
                double z2 = map->grid[idx2].z;
                apply_transform(env, map->grid[idx2].x, map->grid[idx2].y, z2,
                                &x1, &y1);
                bresenham_line(env, x0, y0, x1, y1, color);
            }
        }
    }
}
