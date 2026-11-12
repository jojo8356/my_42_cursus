/*
** fract-ol.h — Fract'ol : fractal renderer (42, Rank 02, Graphic branch)
**
** Central header for the whole project. Every type, constant and function
** prototype lives here so that sources stay free of forward declarations and
** the Norm's "one header per project" rule is respected.
*/

#ifndef FRACTOL_H
# define FRACTOL_H

/* ---------------------------------------------------------------- miniLibX */
# include "mlx.h"
# include "libft.h"

/* ----------------------------------------------------------------- constants */
# define WIDTH  900    /* window width  in pixels */
# define HEIGHT 700    /* window height in pixels */

/* Max number of pthreads used for parallel row rendering. */
# define MAX_THREADS 8

/* Escape radius: |z|² > 4 means the point escapes the set. */
# define MAX_ITER   1000

/* Palette count for the color-cycling bonus. */
# define PALETTE_COUNT 6

/* ------------------------------------------------------------- fractal kinds */
typedef enum e_fractal_type {
    FRACTAL_MANDELBROT = 0,
    FRACTAL_JULIA,
    FRACTAL_BURNING_SHIP,
    FRACTAL_TRICORN,
    FRACTAL_PHOENIX,
    FRACTAL_PHOENIX_VARIANT,
    FRACTAL_MULTIBROT,
    FRACTAL_COUNT
} t_fractal_type;

/* ------------------------------------------------------------------ color --- */
typedef struct s_color {
    int r;
    int g;
    int b;
} t_color;

/* --------------------------------------------------------------- fractal ---- */
/*
** t_fractal — the single source of truth for the whole application.
** Passed by reference everywhere so that the Norm's "≤ 4 parameters" rule is
** never violated and no global variable is required.
*/
typedef struct s_fractal {
    /* ---- miniLibX handles ---- */
    void    *mlx_ptr;
    void    *win_ptr;
    void    *img_ptr;
    char    *img_data;       /* raw pixel buffer (RGBA, 4 bytes/pixel) */
    int      bits_per_pixel;
    int      line_size;
    int      endian;

    /* ---- complex plane viewport ---- */
    double   zoom;            /* scale factor of the complex plane */
    double   shift_r;         /* horizontal translation in the complex plane */
    double   shift_i;         /* vertical   translation in the complex plane */

    /* ---- complex numbers ---- */
    double   z_r, z_i;        /* current z for the iteration loop */
    double   c_r, c_i;        /* the constant c (Julia: fixed; others: per-pixel) */
    double   prev_z_r, prev_z_i; /* z_{n-1} for the Phoenix recurrence */

    /* ---- Phoenix delta parameter ---- */
    double   delta;

    /* ---- rendering ---- */
    int      max_iter;
    int      palette;         /* current color palette index */
    int      color_shift;     /* additive offset for the color range */
    int      grayscale;       /* 0 = colored, 1 = grayscale */
    int      multibrot_power; /* power for the Multibrot fractal (≥ 2) */

    /* ---- identity ---- */
    t_fractal_type type;
    char    *name;           /* human-readable fractal name */
} t_fractal;

/* --------------------------------------------------------------- prototypes */

/* ---- argument parsing & initialisation ---- */
int      parse_args(int argc, char **argv, t_fractal *f);
int      init_mlx(t_fractal *f);
void     init_viewport(t_fractal *f);
int      init_fractal(t_fractal *f);

/* ---- coordinate transforms (pure, testable) ---- */
double   screen_to_complex_r(int x, t_fractal *f);
double   screen_to_complex_i(int y, t_fractal *f);

/* ---- fractal iteration (pure, testable) ----
** Each returns the number of iterations before escape, or max_iter if the
** point stays bounded. `f->z_r`, `f->z_i` and `f->prev_z_r`, `f->prev_z_i`
** are updated as a side effect so callers can reuse them.
*/
int      mandelbrot_iter(t_fractal *f);
int      julia_iter(t_fractal *f);
int      burning_ship_iter(t_fractal *f);
int      tricorn_iter(t_fractal *f);
int      phoenix_iter(t_fractal *f);
int      phoenix_variant_iter(t_fractal *f);
int      multibrot_iter(t_fractal *f);

/* ---- color (pure, testable) ---- */
t_color  get_color(int iter, t_fractal *f);
void     render_pixel(t_fractal *f, int x, int y, int iter);
void     put_pixel(t_fractal *f, int x, int y, t_color c);

/* ---- rendering pipeline ---- */
int      compute_pixel(t_fractal *f, int x, int y);
void     render_row(t_fractal *f, int y);
void    *render_thread(void *arg);
void     render_image(t_fractal *f);
int      redraw(t_fractal *f);

/* ---- event hooks ---- */
int      on_key_press(int keycode, void *param);
int      on_mouse_press(int button, int x, int y, void *param);
int      on_mouse_move(int x, int y, void *param);
int      on_expose(void *param);
int      on_close(void *param);

/* ---- helpers ---- */
void     shift_view(t_fractal *f, double d_r, double d_i);
void     zoom_at(t_fractal *f, double x, double y, double factor);
void     change_palette(t_fractal *f);
void     toggle_grayscale(t_fractal *f);
int      get_max_threads(void);
char    *fractal_name(t_fractal_type t);
int      fractal_from_name(const char *name);
void     print_usage(void);
int      cleanup_and_exit(t_fractal *f, int code);

/* ---- tests ---- */
/* tests/test_math.c, tests/test_color.c, tests/test_args.c, tests/test_thread.c */

# endif /* FRACTOL_H */
