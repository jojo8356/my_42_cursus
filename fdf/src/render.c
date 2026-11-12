/*
** render.c — MiniLibX rendering pipeline.
**
** These functions call mlx_* functions and require X11 to run.
*/

# include "fdf.h"
# include <stdio.h>

/* ----------------------------------------------------------------------- */
int init_mlx(t_env *env)
{
    env->mlx = mlx_init();
    if (!env->mlx)
        return -1;

    env->win = mlx_new_window(env->mlx, WIN_W, WIN_H, TITLE);
    if (!env->win)
        return -1;

    env->img = mlx_new_image(env->mlx, WIN_W, WIN_H);
    if (!env->img)
        return -1;

    env->img_data = mlx_get_data_addr(env->img, &env->bits_per_pixel,
                                      &env->line_size, &env->endian);
    if (!env->img_data)
        return -1;

    /* Register event hooks. */
    mlx_hook(env->win, 2, 0, on_key_press, env);       /* KeyPress      */
    mlx_hook(env->win, 4, 0, on_mouse_press, env);     /* ButtonPress   */
    mlx_hook(env->win, 6, 0, on_mouse_move, env);      /* MotionNotify  */
    mlx_hook(env->win, 12, 0, on_expose, env);         /* Expose        */
    mlx_hook(env->win, 17, 0, on_close, env);          /* DestroyNotify */

    return 0;
}

/* ----------------------------------------------------------------------- */
void render(t_env *env)
{
    /* Clear the image buffer (black background). */
    for (int i = 0; i < env->line_size * WIN_H; i++)
        env->img_data[i] = 0;

    /* Draw the grid. */
    draw_grid(env);

    /* Push to window. */
    mlx_put_image_to_window(env->mlx, env->win, env->img, 0, 0);

    /* Display info string. */
    print_info(env);
}

/* ----------------------------------------------------------------------- */
int redraw(t_env *env)
{
    mlx_destroy_image(env->mlx, env->img);
    env->img = mlx_new_image(env->mlx, WIN_W, WIN_H);
    env->img_data = mlx_get_data_addr(env->img, &env->bits_per_pixel,
                                      &env->line_size, &env->endian);
    render(env);
    return 0;
}

/* ----------------------------------------------------------------------- */
void print_info(t_env *env)
{
    char info[256];
    snprintf(info, sizeof(info), "%s  |  %dx%d  |  z: %d..%d  |  scale: %.1f",
             env->file_name,
             env->map->width, env->map->height,
             env->map->min_z, env->map->max_z,
             env->scale);
    mlx_string_put(env->mlx, env->win, 10, 10, COLOR_WHITE, info);

    char controls[128];
    snprintf(controls, sizeof(controls),
             "WASD:move  Arrows:rot  Scroll:zoom  C:color  R:reset  ESC:exit");
    mlx_string_put(env->mlx, env->win, 10, WIN_H - 25, COLOR_CYAN, controls);
}

/* ----------------------------------------------------------------------- */
void cleanup(t_env *env)
{
    if (env->img && env->mlx)
        mlx_destroy_image(env->mlx, env->img);
    if (env->win && env->mlx)
        mlx_destroy_window(env->mlx, env->win);
    if (env->mlx)
        mlx_loop_end(env->mlx);
    free_map(env->map);
}
