/*
** fdf.h — FDF: wireframe 3D terrain renderer (42, Rank 02, Graphic branch)
**
** Central header. Every type, constant and function prototype lives here.
*/

#ifndef FDF_H
# define FDF_H

/* ---------------------------------------------------------------- miniLibX */
# include "mlx.h"
# include "libft.h"

/* ----------------------------------------------------------------- constants */
# define WIN_W  1200
# define WIN_H  800

/* Window title. */
# define TITLE "FDF - Wireframe Terrain"

/* Max grid dimensions (protect against malformed files). */
# define MAX_GRID_SIZE 10000

/* Isometric projection angles (degrees → radians). */
# define ISO_ANGLE_X  0.643501109   /* ~36.87° — classic isometric tilt */
# define ISO_ANGLE_Y  0.785398163   /* ~45°    — classic isometric tilt */

/* Color constants (0xRRGGBB). */
# define COLOR_WHITE 0xFFFFFF
# define COLOR_BLACK 0x000000
# define COLOR_RED   0xFF0000
# define COLOR_GREEN 0x00FF00
# define COLOR_BLUE  0x0000FF
# define COLOR_CYAN  0x00FFFF
# define COLOR_MAGENTA 0xFF00FF
# define COLOR_YELLOW 0xFFFF00

/* Color palette modes for the color-cycling bonus.
** Names prefixed with MODE_ to avoid collision with COLOR_* #defines. */
typedef enum e_color_mode {
    MODE_GRADIENT = 0,     /* Red → Green (default) */
    MODE_GRAYSCALE,        /* White → Black */
    MODE_BLUE,             /* Blue → White */
    MODE_CYAN,             /* Cyan → White */
    MODE_RAINBOW,          /* Full rainbow */
    MODE_COUNT
} t_color_mode;

/* ------------------------------------------------------------- point / map -- */
typedef struct s_point {
    int  x;          /* screen x (after projection) */
    int  y;          /* screen y (after projection) */
    double z;        /* original height value */
    int  color;      /* 0xRRGGBB */
} t_point;

typedef struct s_map {
    t_point *grid;       /* flat array of size width * height */
    int      width;      /* number of columns */
    int      height;     /* number of rows */
    int      min_z;      /* minimum height value (for color scaling) */
    int      max_z;      /* maximum height value (for color scaling) */
} t_map;

/* ------------------------------------------------------------------- env -- */
typedef struct s_env {
    /* ---- miniLibX ---- */
    void    *mlx;
    void    *win;
    void    *img;
    char    *img_data;
    int      bits_per_pixel;
    int      line_size;
    int      endian;

    /* ---- map ---- */
    t_map   *map;

    /* ---- view transformation ---- */
    double   scale;         /* zoom scale factor */
    double   offset_x;      /* horizontal translation */
    double   offset_y;      /* vertical translation */
    double   rot_x;         /* rotation around X axis (radians) */
    double   rot_y;         /* rotation around Y axis (radians) */
    double   rot_z;         /* rotation around Z axis (radians) */
    double   iso_x;         /* isometric angle X */
    double   iso_y;         /* isometric angle Y */

    /* ---- colors ---- */
    t_color_mode color_mode;
    int      color_min;    /* minimum color (0xRRGGBB) */
    int      color_max;    /* maximum color (0xRRGGBB) */

    /* ---- info ---- */
    char    *file_name;
} t_env;

/* --------------------------------------------------------------- prototypes */

/* ---- file parsing (pure, testable) ---- */
int      parse_file(const char *filename, t_map *map);
void     free_map(t_map *map);

/* ---- 3D math (pure, testable) ---- */
double   radians(double degrees);
void     rotate_x(double *x, double *y, double *z, double angle);
void     rotate_y(double *x, double *y, double *z, double angle);
void     rotate_z(double *x, double *y, double *z, double angle);
void     project_iso(double x, double y, double z,
                     double iso_x, double iso_y,
                     double scale, double *out_x, double *out_y);
void     apply_transform(t_env *env, double x, double y, double z,
                         int *out_x, int *out_y);

/* ---- color (pure, testable) ---- */
int      compute_color(double z, t_env *env);
int      color_lerp(int c1, int c2, double percent);
void     cycle_color_mode(t_env *env);

/* ---- drawing (pure for math, mlx for rendering) ---- */
void     bresenham_line(t_env *env, int x0, int y0, int x1, int y1, int color);
void     put_pixel(t_env *env, int x, int y, int color);
void     draw_grid(t_env *env);

/* ---- rendering pipeline (mlx) ---- */
int      init_mlx(t_env *env);
void     render(t_env *env);
int      redraw(t_env *env);

/* ---- events (mlx) ---- */
int      on_key_press(int keycode, void *param);
int      on_mouse_press(int button, int x, int y, void *param);
int      on_mouse_move(int x, int y, void *param);
int      on_expose(void *param);
int      on_close(void *param);

/* ---- helpers ---- */
void     zoom(t_env *env, double factor);
void     translate(t_env *env, double dx, double dy);
void     rotate(t_env *env, char axis, double direction);
void     print_info(t_env *env);
void     cleanup(t_env *env);

/* ---- tests ---- */
/* tests/test_parse.c, tests/test_math.c, tests/test_draw.c, tests/test_color.c */

# endif /* FDF_H */
