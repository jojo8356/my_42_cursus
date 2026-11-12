/*
** transform.c — Screen ↔ complex-plane coordinate conversion and viewport
** helpers (zoom, pan).  All functions are pure math, fully testable headless.
*/

# include "fract-ol.h"
# include <math.h>

/* The complex plane is centred on the origin.  The initial view spans
** approximately [-2, 2] on the real axis and [-2, 2] on the imaginary axis,
** preserving the aspect ratio of the window. */

/* ----------------------------------------------------------------------- *
** screen_to_complex_r — map a horizontal screen x to a complex real value.
**
**   x ranges [0, WIDTH-1]  →  real ranges [shift_r - zoom/2, shift_r + zoom/2]
**
** The zoom factor is the *width* of the visible complex-plane window.
** Smaller zoom = more magnified.
** ----------------------------------------------------------------------- */
double screen_to_complex_r(int x, t_fractal *f)
{
    return f->shift_r + (double)x / WIDTH * f->zoom - f->zoom / 2.0;
}

/* ----------------------------------------------------------------------- *
** screen_to_complex_i — map a vertical screen y to a complex imaginary value.
**
**   y ranges [0, HEIGHT-1]  →  imag ranges [shift_i - zoom/2, shift_i + zoom/2]
**
** The imaginary axis is inverted (screen y grows downward, imag y grows upward)
** so that the fractal is not upside-down.
** ----------------------------------------------------------------------- */
double screen_to_complex_i(int y, t_fractal *f)
{
    /* Invert y: top of screen = +imag */
    return f->shift_i - ((double)y / HEIGHT * f->zoom - f->zoom / 2.0);
}

/* ----------------------------------------------------------------------- *
** shift_view — translate the viewport by (d_r, d_i) in the complex plane.
** ----------------------------------------------------------------------- */
void shift_view(t_fractal *f, double d_r, double d_i)
{
    f->shift_r += d_r;
    f->shift_i += d_i;
}

/* ----------------------------------------------------------------------- *
** zoom_at — zoom the complex plane toward the point (x, y) in *pixel* space.
**
**   factor < 1  →  zoom in
**   factor > 1  →  zoom out
**
** The fractal is re-centred so that the point under the cursor stays fixed.
** ----------------------------------------------------------------------- */
void zoom_at(t_fractal *f, double x, double y, double factor)
{
    /* Save the complex coordinate currently under the cursor. */
    double target_r = screen_to_complex_r((int)x, f);
    double target_i = screen_to_complex_i((int)y, f);

    /* Scale the zoom. */
    f->zoom *= factor;

    /* Clamp zoom to avoid floating-point overflow / underflow. */
    if (f->zoom < 1e-12)
        f->zoom = 1e-12;
    if (f->zoom > 4.0)
        f->zoom = 4.0;

    /* Recompute shift so that the target point stays under (x, y). */
    /*  target = shift + (x/WIDTH)*zoom - zoom/2   (for real) */
    /*  shift = target - (x/WIDTH)*zoom + zoom/2 */
    f->shift_r = target_r - (x / WIDTH) * f->zoom + f->zoom / 2.0;
    f->shift_i = target_i + (y / HEIGHT) * f->zoom - f->zoom / 2.0;
}

/* ----------------------------------------------------------------------- *
** get_max_threads — return the number of CPU cores (clamped to MAX_THREADS).
** ----------------------------------------------------------------------- */
int get_max_threads(void)
{
    int n = 1;
# if defined(_SC_NPROCESSORS_CONF)
    n = sysconf(_SC_NPROCESSORS_CONF);
# endif
    if (n < 1)
        n = 1;
    if (n > MAX_THREADS)
        n = MAX_THREADS;
    return n;
}
