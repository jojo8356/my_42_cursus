/*
** init.c — miniLibX window/image initialisation and viewport setup.
**
** These functions are the only ones that call mlx_* functions besides the
** event hooks and the render pipeline, so they require X11 to run.
*/

# include "fract-ol.h"
# include <stdlib.h>

/* ----------------------------------------------------------------------- */
/* Initial complex-plane view: centred on origin, zoom = 4 (spans [-2,2]). */
void init_viewport(t_fractal *f)
{
    f->zoom     = 4.0;
    f->shift_r = 0.0;
    f->shift_i = 0.0;
    f->color_shift = 0;
}

/* ----------------------------------------------------------------------- */
int init_mlx(t_fractal *f)
{
    f->mlx_ptr = mlx_init();
    if (!f->mlx_ptr)
        return -1;

    f->win_ptr = mlx_new_window(f->mlx_ptr, WIDTH, HEIGHT, "Fract'ol");
    if (!f->win_ptr)
        return -1;

    f->img_ptr = mlx_new_image(f->mlx_ptr, WIDTH, HEIGHT);
    if (!f->img_ptr)
        return -1;

    f->img_data = mlx_get_data_addr(f->img_ptr, &f->bits_per_pixel,
                                     &f->line_size, &f->endian);
    if (!f->img_data)
        return -1;

    /* Register event hooks. */
    mlx_hook(f->win_ptr, 2, 0, on_key_press, f);       /* KeyPress        */
    mlx_hook(f->win_ptr, 4, 0, on_mouse_press, f);     /* ButtonPress     */
    mlx_hook(f->win_ptr, 6, 0, on_mouse_move, f);      /* MotionNotify    */
    mlx_hook(f->win_ptr, 12, 0, on_expose, f);         /* Expose          */
    mlx_hook(f->win_ptr, 17, 0, on_close, f);          /* DestroyNotify   */

    return 0;
}

/* ----------------------------------------------------------------------- */
int init_fractal(t_fractal *f)
{
    init_viewport(f);
    f->max_iter    = MAX_ITER;
    f->palette     = 0;
    f->grayscale   = 0;
    f->color_shift = 0;
    f->prev_z_r    = 0.0;
    f->prev_z_i    = 0.0;
    return 0;
}

/* ----------------------------------------------------------------------- */
int cleanup_and_exit(t_fractal *f, int code)
{
    if (f->img_ptr && f->mlx_ptr)
        mlx_destroy_image(f->mlx_ptr, f->img_ptr);
    if (f->win_ptr && f->mlx_ptr)
        mlx_destroy_window(f->mlx_ptr, f->win_ptr);
    if (f->mlx_ptr)
        mlx_loop_end(f->mlx_ptr);
    return code;
}
