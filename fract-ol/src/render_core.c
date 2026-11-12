/*
** render_core.c — mlx-independent rendering core (compute_pixel, render_row,
** render_thread).  These functions only touch the pixel buffer and call the
** fractal math layer; they never call mlx_* functions, making them fully
** testable in a headless environment.
*/

# include "fract-ol.h"
# include <pthread.h>
# include <math.h>

/* Argument struct passed to each render thread. */
typedef struct s_render_args {
    t_fractal   *f;
    int          y_start;
    int          y_end;
    int          thread_id;
} t_render_args;

/* ----------------------------------------------------------------------- *
** compute_pixel — compute the iteration count for the pixel at (x, y) and
** write its colour into the image buffer.  Returns the iteration count.
** ----------------------------------------------------------------------- */
int compute_pixel(t_fractal *f, int x, int y)
{
    /* Map screen coordinates → complex plane. */
    f->z_r = screen_to_complex_r(x, f);
    f->z_i = screen_to_complex_i(y, f);

    /* Dispatch to the correct fractal iteration function. */
    int iter;
    switch (f->type)
    {
        case FRACTAL_MANDELBROT:      iter = mandelbrot_iter(f); break;
        case FRACTAL_JULIA:          iter = julia_iter(f);     break;
        case FRACTAL_BURNING_SHIP:   iter = burning_ship_iter(f); break;
        case FRACTAL_TRICORN:        iter = tricorn_iter(f);   break;
        case FRACTAL_PHOENIX:        iter = phoenix_iter(f);   break;
        case FRACTAL_PHOENIX_VARIANT: iter = phoenix_variant_iter(f); break;
        case FRACTAL_MULTIBROT:      iter = multibrot_iter(f); break;
        default:                     iter = mandelbrot_iter(f); break;
    }

    render_pixel(f, x, y, iter);
    return iter;
}

/* ----------------------------------------------------------------------- *
** render_row — render every pixel in row y.
** ----------------------------------------------------------------------- */
void render_row(t_fractal *f, int y)
{
    for (int x = 0; x < WIDTH; x++)
        compute_pixel(f, x, y);
}

/* ----------------------------------------------------------------------- *
** render_thread — pthread entry point.  Renders a stripe of rows.
** Uses a local copy of the fractal struct to avoid race conditions on
** f->z_r, f->z_i, f->c_r, f->c_i (each thread writes its own copy).
** The image buffer (img_data) is shared but each thread writes disjoint
** rows, so there is no race on the pixel data.
** ----------------------------------------------------------------------- */
void *render_thread(void *arg)
{
    t_render_args *a = (t_render_args *)arg;
    t_fractal local_f = *a->f;   /* copy — avoids race on z/c fields */
    for (int y = a->y_start; y < a->y_end; y++)
        render_row(&local_f, y);
    return NULL;
}
