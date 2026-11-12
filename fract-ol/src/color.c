/*
** color.c — Color palettes and pixel-color computation.
**
** All functions here are pure (no mlx calls) so they can be unit-tested in a
** headless environment.  `put_pixel` and `render_pixel` touch the image buffer
** and are called only by the rendering pipeline.
*/

# include "fract-ol.h"
# include <math.h>

/* ----------------------------------------------------------------------- *
** All palettes are generated procedurally from HSL→RGB at runtime.
** Each palette gets a different hue offset, producing visually distinct
** colour schemes for the palette-cycling bonus.
** ----------------------------------------------------------------------- */

/* Generate a HSL→RGB palette into `out` with a hue offset. */
static void generate_palette(t_color *out, int hue_offset, double sat, double lig)
{
    for (int i = 0; i < 256; i++)
    {
        double h = (double)(i + hue_offset) / 256.0;
        double s = sat;
        double l = lig;
        /* HSL → RGB */
        double c = (1.0 - fabs(2.0 * l - 1.0)) * s;
        double x = c * (1.0 - fabs(fmod(h * 6.0, 2.0) - 1.0));
        double m = l - c / 2.0;
        double r, g, b;
        if (h < 1.0 / 6.0)      { r = c; g = x; b = 0; }
        else if (h < 2.0 / 6.0) { r = x; g = c; b = 0; }
        else if (h < 3.0 / 6.0) { r = 0; g = c; b = x; }
        else if (h < 4.0 / 6.0) { r = 0; g = x; b = c; }
        else if (h < 5.0 / 6.0) { r = x; g = 0; b = c; }
        else                    { r = c; g = 0; b = x; }
        out[i].r = (int)((r + m) * 255);
        out[i].g = (int)((g + m) * 255);
        out[i].b = (int)((b + m) * 255);
    }
}

/* Master palette table — indexed by f->palette. */
typedef struct s_palette_spec {
    int    hue_offset;
    double sat;
    double lig;
} t_palette_spec;

static const t_palette_spec palette_specs[PALETTE_COUNT] = {
    {   0, 0.90, 0.50 },  /* 0: Classic rainbow          */
    {  32, 0.85, 0.55 },  /* 1: Shifted rainbow          */
    {  64, 0.70, 0.45 },  /* 2: Muted teal               */
    {  96, 0.95, 0.60 },  /* 3: Bright warm              */
    { 128, 0.80, 0.50 },  /* 4: Cool blue                */
    { 160, 0.90, 0.55 },  /* 5: Purple-magenta           */
};

static t_color (*get_palette(int idx))[256]
{
    static t_color palettes[PALETTE_COUNT][256];
    static int initialized = 0;

    if (!initialized)
    {
        for (int p = 0; p < PALETTE_COUNT; p++)
            generate_palette(palettes[p],
                             palette_specs[p].hue_offset,
                             palette_specs[p].sat,
                             palette_specs[p].lig);
        initialized = 1;
    }
    if (idx < 0 || idx >= PALETTE_COUNT)
        idx = 0;
    return palettes + idx;
}

/* ----------------------------------------------------------------------- *
** get_color — map an iteration count to an RGB triple.
**
**   •  iter == max_iter  →  black  (point is in the set)
**   •  otherwise         →  palette[(iter + color_shift) % 256]
** ----------------------------------------------------------------------- */
t_color get_color(int iter, t_fractal *f)
{
    t_color black = { 0, 0, 0 };
    t_color c;

    if (iter >= f->max_iter)
        return black;

    if (f->grayscale)
    {
        /* Smooth grayscale ramp based on iteration density. */
        int v = iter * 255 / f->max_iter;
        c.r = c.g = c.b = v;
        return c;
    }

    int idx = (iter + f->color_shift) % 256;
    if (idx < 0)
        idx += 256;
    c = (*get_palette(f->palette))[idx];
    return c;
}

/* ----------------------------------------------------------------------- *
** render_pixel — compute the iteration for one pixel and write its color to
** the image buffer.  Touches mlx state, so NOT unit-testable headless.
** ----------------------------------------------------------------------- */
void render_pixel(t_fractal *f, int x, int y, int iter)
{
    t_color c = get_color(iter, f);
    put_pixel(f, x, y, c);
}

/* ----------------------------------------------------------------------- *
** put_pixel — write a single RGBA pixel into the raw buffer.
** ----------------------------------------------------------------------- */
void put_pixel(t_fractal *f, int x, int y, t_color c)
{
    if (x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT)
        return;
    int bpp = f->bits_per_pixel / 8;   /* bytes per pixel */
    int offset = y * f->line_size + x * bpp;
    f->img_data[offset]     = (char)c.r;
    f->img_data[offset + 1] = (char)c.g;
    f->img_data[offset + 2] = (char)c.b;
    f->img_data[offset + 3] = 255;
}
