/*
** render.c — mlx-dependent rendering (render_image, redraw).
**
** The mlx-independent core (compute_pixel, render_row, render_thread) lives in
** render_core.c so it can be unit-tested without X11.
*/

# include "fract-ol.h"
# include <pthread.h>

/* ----------------------------------------------------------------------- *
** render_image — spawn threads, render the whole image, push to window.
** ----------------------------------------------------------------------- */
void render_image(t_fractal *f)
{
    int threads = get_max_threads();
    int rows_per_thread = HEIGHT / threads;
    pthread_t  tid[MAX_THREADS];
    /* t_render_args is defined in render_core.c; we need it here too. */
    typedef struct s_render_args {
        t_fractal *f;
        int        y_start;
        int        y_end;
        int        thread_id;
    } t_render_args_local;
    t_render_args_local args[MAX_THREADS];

    for (int t = 0; t < threads; t++)
    {
        args[t].f         = f;
        args[t].y_start   = t * rows_per_thread;
        args[t].y_end     = (t == threads - 1) ? HEIGHT : (t + 1) * rows_per_thread;
        args[t].thread_id = t;
        pthread_create(&tid[t], NULL, render_thread, &args[t]);
    }
    for (int t = 0; t < threads; t++)
        pthread_join(tid[t], NULL);

    /* Push the finished image to the window. */
    mlx_put_image_to_window(f->mlx_ptr, f->win_ptr, f->img_ptr, 0, 0);
}

/* ----------------------------------------------------------------------- *
** redraw — full re-render (used after any state change).
** ----------------------------------------------------------------------- */
int redraw(t_fractal *f)
{
    mlx_destroy_image(f->mlx_ptr, f->img_ptr);
    f->img_ptr = mlx_new_image(f->mlx_ptr, WIDTH, HEIGHT);
    f->img_data = mlx_get_data_addr(f->img_ptr, &f->bits_per_pixel,
                                     &f->line_size, &f->endian);
    render_image(f);
    return 0;
}
