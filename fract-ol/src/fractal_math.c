/*
** fractal_math.c — Pure fractal iteration logic.
**
** Every function here is completely independent of miniLibX: it only reads and
** writes the complex-number fields of t_fractal.  That makes the entire math
** layer unit-testable in a headless environment (no X11 required).
**
** Conventions:
**   •  z_r, z_i  — current z  (mutated in place each iteration)
**   •  c_r, c_i  — the constant c
**   •  prev_z_r, prev_z_i — z_{n-1} (only used by Phoenix)
**   •  Return value = iteration count at escape, or max_iter if bounded.
*/

# include "fract-ol.h"

/* ----------------------------------------------------------------------- *
** Mandelbrot:  z_{n+1} = z_n² + c     (z₀ = 0, c = pixel)
** ----------------------------------------------------------------------- */
int mandelbrot_iter(t_fractal *f)
{
    int iter = 0;
    double zr = 0.0, zi = 0.0;
    double zr2, zi2;

    f->c_r = f->z_r;   /* pixel → c */
    f->c_i = f->z_i;
    while (iter < f->max_iter)
    {
        zr2 = zr * zr;
        zi2 = zi * zi;
        if (zr2 + zi2 > 4.0)
            break;
        /* z = z² + c */
        zi = 2.0 * zr * zi + f->c_i;
        zr = zr2 - zi2 + f->c_r;
        iter++;
    }
    f->z_r = zr;
    f->z_i = zi;
    return iter;
}

/* ----------------------------------------------------------------------- *
** Julia:  z_{n+1} = z_n² + c     (z₀ = pixel, c = fixed argument)
** ----------------------------------------------------------------------- */
int julia_iter(t_fractal *f)
{
    int iter = 0;
    double zr = f->z_r;   /* pixel → z₀ */
    double zi = f->z_i;
    double zr2, zi2;

    /* c is already set from the command-line arguments */
    while (iter < f->max_iter)
    {
        zr2 = zr * zr;
        zi2 = zi * zi;
        if (zr2 + zi2 > 4.0)
            break;
        zi = 2.0 * zr * zi + f->c_i;
        zr = zr2 - zi2 + f->c_r;
        iter++;
    }
    f->z_r = zr;
    f->z_i = zi;
    return iter;
}

/* ----------------------------------------------------------------------- *
** Burning Ship:  z_{n+1} = (|Re(z)| + i|Im(z)|)² + c
**                 = (|zr|² - |zi|² + i·2|zr||zi|) + c
** ----------------------------------------------------------------------- */
int burning_ship_iter(t_fractal *f)
{
    int iter = 0;
    double zr = 0.0, zi = 0.0;
    double ar, ai;

    f->c_r = f->z_r;
    f->c_i = f->z_i;
    while (iter < f->max_iter)
    {
        ar = zr < 0.0 ? -zr : zr;
        ai = zi < 0.0 ? -zi : zi;
        if (ar * ar + ai * ai > 4.0)
            break;
        /* z = (ar + i·ai)² + c */
        zi = 2.0 * ar * ai + f->c_i;
        zr = ar * ar - ai * ai + f->c_r;
        iter++;
    }
    f->z_r = zr;
    f->z_i = zi;
    return iter;
}

/* ----------------------------------------------------------------------- *
** Tricorn (Mandelbar):  z_{n+1} = conj(z)² + c = (zr − i·zi)² + c
** ----------------------------------------------------------------------- */
int tricorn_iter(t_fractal *f)
{
    int iter = 0;
    double zr = 0.0, zi = 0.0;
    double zr2, zi2;

    f->c_r = f->z_r;
    f->c_i = f->z_i;
    while (iter < f->max_iter)
    {
        zr2 = zr * zr;
        zi2 = zi * zi;
        if (zr2 + zi2 > 4.0)
            break;
        /* conj(z)² = (zr − i·zi)² = (zr² − zi²) − i·(2·zr·zi) */
        zi = -2.0 * zr * zi + f->c_i;
        zr = zr2 - zi2 + f->c_r;
        iter++;
    }
    f->z_r = zr;
    f->z_i = zi;
    return iter;
}

/* ----------------------------------------------------------------------- *
** Phoenix (Mandelbrot-style):  z_{n+1} = z_n² + c + δ·z_{n-1}
**   z₀ = 0, z_{-1} = 0, c = pixel.
**   With δ = 0, reduces to Mandelbrot: z₁ = c.
** ----------------------------------------------------------------------- */
int phoenix_iter(t_fractal *f)
{
    int iter = 0;
    double zr = 0.0, zi = 0.0;          /* z₀ = 0 */
    double prev_r = 0.0, prev_i = 0.0;  /* z_{-1} = 0 */
    double zr2, zi2;

    f->c_r = f->z_r;   /* pixel → c */
    f->c_i = f->z_i;

    /* z₁ = z₀² + c + δ·z_{-1} = 0 + c + 0 = c */
    zi = f->c_i;
    zr = f->c_r;
    iter = 1;

    while (iter < f->max_iter)
    {
        zr2 = zr * zr;
        zi2 = zi * zi;
        if (zr2 + zi2 > 4.0)
            break;
        double new_r = zr2 - zi2 + f->c_r + f->delta * prev_r;
        double new_i = 2.0 * zr * zi + f->c_i + f->delta * prev_i;
        prev_r = zr;   /* save z_n for next iteration's z_{n-1} */
        prev_i = zi;
        zr = new_r;
        zi = new_i;
        iter++;
    }
    f->z_r = zr;
    f->z_i = zi;
    return iter;
}

/* ----------------------------------------------------------------------- *
** Phoenix variant (Julia-style):  z_{n+1} = z_n² + c + δ·z_{n-1}
**   z₀ = pixel, z_{-1} = 0, c = fixed (from command-line arguments).
**   z₁ = z₀² + c, z₂ = z₁² + c + δ·z₀, ...
** ----------------------------------------------------------------------- */
int phoenix_variant_iter(t_fractal *f)
{
    int iter = 0;
    double zr = f->z_r;          /* z₀ = pixel */
    double zi = f->z_i;
    double z0_r = zr;           /* save z₀ for z₂ recurrence */
    double z0_i = zi;
    double prev_r = 0.0, prev_i = 0.0;  /* z_{-1} = 0 */
    double zr2, zi2;

    /* z₁ = z₀² + c + δ·z_{-1} = z₀² + c + 0 */
    double old_zi = zi;
    zi = 2.0 * zr * old_zi + f->c_i;
    zr = zr * zr - old_zi * old_zi + f->c_r;
    iter = 1;

    prev_r = z0_r;   /* z₀ for next iteration's z_{n-1} */
    prev_i = z0_i;

    while (iter < f->max_iter)
    {
        zr2 = zr * zr;
        zi2 = zi * zi;
        if (zr2 + zi2 > 4.0)
            break;
        double new_r = zr2 - zi2 + f->c_r + f->delta * prev_r;
        double new_i = 2.0 * zr * zi + f->c_i + f->delta * prev_i;
        prev_r = zr;
        prev_i = zi;
        zr = new_r;
        zi = new_i;
        iter++;
    }
    f->z_r = zr;
    f->z_i = zi;
    return iter;
}

/* ----------------------------------------------------------------------- *
** Multibrot:  z_{n+1} = z_n^p + c   (p ≥ 2, default 3 = Tricorn-like "cube")
** Uses the binomial expansion for integer powers.
** ----------------------------------------------------------------------- */
int multibrot_iter(t_fractal *f)
{
    int iter = 0;
    int p = f->multibrot_power;
    if (p < 2)
        p = 2;
    double zr = 0.0, zi = 0.0;

    f->c_r = f->z_r;
    f->c_i = f->z_i;
    while (iter < f->max_iter)
    {
        double mag2 = zr * zr + zi * zi;
        if (mag2 > 4.0)
            break;
        /* z^p via repeated squaring on (a+bi) */
        double pr = 1.0, pi = 0.0;     /* result */
        double ar = zr, ai = zi;       /* base   */
        int k = p;
        while (k > 0)
        {
            if (k & 1)
            {
                double nr = pr * ar - pi * ai;
                double ni = pr * ai + pi * ar;
                pr = nr;
                pi = ni;
            }
            double nr = ar * ar - ai * ai;
            double ni = 2.0 * ar * ai;
            ar = nr;
            ai = ni;
            k >>= 1;
        }
        /* z = z^p + c */
        zi = pi + f->c_i;
        zr = pr + f->c_r;
        iter++;
    }
    f->z_r = zr;
    f->z_i = zi;
    return iter;
}
