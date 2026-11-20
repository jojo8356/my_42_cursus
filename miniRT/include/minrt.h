/* ************************************************************************** */
/*                                                                            */
/*   minrt.h                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   miniRT — header principal. Ray tracer (Rank 05, 42).                   */
/* ************************************************************************** */

# ifndef MINRT_H
# define MINRT_H

# include <stddef.h>
# include <stdio.h>
# include "libft.h"

/* ------------------------------------------------------------------ */
/*  Constantes                                                          */
/* ------------------------------------------------------------------ */

# define MINRT_VERSION        "1.0"

# define WINDOW_W             800
# define WINDOW_H             600

# define EPS                  1e-6
# define MAX_DEPTH           8     /* profondeur max récursion (réflexion/réfraction) */
# define DEFAULT_SHININESS   32.0
# define DEFAULT_SPECULAR    0.3
# define DEFAULT_REFLECTION  0.0
# define DEFAULT_TRANSP      0.0
# define DEFAULT_RI          1.0

/* Antialiasing */
# define AA_SAMPLES          4     /* échantillons par pixel (2x2) */

/* Types d'objets */
# define OBJ_SPHERE           0
# define OBJ_PLANE            1
# define OBJ_CYLINDER         2
# define OBJ_CONE             3
# define OBJ_CUBE             4
# define OBJ_TRIANGLE         5

/* Propriétés matérielles (bonus) */
# define MAT_DEFAULT_SPEC     0.3

/* ------------------------------------------------------------------ */
/*  Vecteur 3D                                                          */
/* ------------------------------------------------------------------ */

typedef struct s_vec3
{
    double    x;
    double    y;
    double    z;
}                   t_vec3;

/* ------------------------------------------------------------------ */
/*  Matrice 4x4 (colonne-major)                                         */
/* ------------------------------------------------------------------ */

typedef struct s_mat4
{
    double    m[4][4];
}                   t_mat4;

/* ------------------------------------------------------------------ */
/*  Rayon                                                                */
/* ------------------------------------------------------------------ */

typedef struct s_ray
{
    t_vec3    origin;
    t_vec3    dir;            /* normalisé */
}                   t_ray;

/* ------------------------------------------------------------------ */
/*  Couleur (canaux 0..255)                                             */
/* ------------------------------------------------------------------ */

typedef struct s_color
{
    double    r;
    double    g;
    double    b;
}                   t_color;

/* ------------------------------------------------------------------ */
/*  Texture (bonus)                                                    */
/* ------------------------------------------------------------------ */

typedef struct s_texture
{
    int        width;
    int        height;
    char    *data;            /* tampon RGBA 32bpp                  */
    int        bpp;
    int        line;
}                   t_texture;

/* ------------------------------------------------------------------ */
/*  Matière / propriétés optiques (bonus)                               */
/* ------------------------------------------------------------------ */

typedef struct s_material
{
    double    specular;       /* ratio spéculaire (sp:)                */
    double    reflection;     /* ratio réflexion (re:)                 */
    double    transparency;   /* ratio transparence (tr:)              */
    double    refr_index;     /* indice de réfraction (ri:)            */
    double    shininess;      /* éclat (sh:)                           */
    int        pattern;       /* type de motif (PAT_*)                 */
    double    pattern_a;      /* paramètre 1 du motif (échelle)       */
    double    pattern_b;      /* paramètre 2 du motif                 */
    t_color    pattern_c1;    /* couleur 1 du motif                   */
    t_color    pattern_c2;    /* couleur 2 du motif                   */
    char    *texture;        /* nom de texture (texture:)            */
    t_texture *tex;          /* texture chargée (NULL si absente)    */
}                   t_material;

/* Types de motifs (bonus) */
# define PAT_NONE             0
# define PAT_STRIPE           1
# define PAT_CHECKER          2
# define PAT_GRADIENT         3
# define PAT_RING              4
# define PAT_TEST             5

/* ------------------------------------------------------------------ */
/*  Objet géométrique (géométrie en space local + transform)           */
/* ------------------------------------------------------------------ */

typedef struct s_obj
{
    int        type;
    t_mat4    transform;      /* translation + rotation              */
    t_mat4    inv_transform;  /* inverse (pour transformer le rayon) */
    t_color    color;
    t_material mat;

    /* géométrie en space local */
    t_vec3    sphere_center;  /* OBJ_SPHERE                          */
    double    sphere_radius;

    t_vec3    plane_point;    /* OBJ_PLANE                           */
    t_vec3    plane_normal;

    t_vec3    cyl_center;     /* OBJ_CYLINDER                        */
    t_vec3    cyl_axis;       /* normalisé                           */
    double    cyl_radius;
    double    cyl_height;

    t_vec3    cone_apex;      /* OBJ_CONE                            */
    t_vec3    cone_axis;      /* normalisé (pointe vers la base)     */
    double    cone_half_angle; /* en radians                        */
    double    cone_height;

    double    cube_size;      /* OBJ_CUBE (côté, centré sur origine) */

    t_vec3    tri_v0;         /* OBJ_TRIANGLE                        */
    t_vec3    tri_v1;
    t_vec3    tri_v2;
}                   t_obj;

/* ------------------------------------------------------------------ */
/*  Intersection                                                        */
/* ------------------------------------------------------------------ */

typedef struct s_hit
{
    int        hit;
    double    t;              /* distance le plus proche            */
    t_vec3    point;          /* point d'impact (world)            */
    t_vec3    normal;         /* normale (world, sortante)         */
    t_obj     *obj;
}                   t_hit;

/* ------------------------------------------------------------------ */
/*  Lumière directionnelle                                              */
/* ------------------------------------------------------------------ */

typedef struct s_light
{
    t_vec3    position;
    t_vec3    direction;      /* normalisé, vers la scène            */
    t_color    color;
    double    ratio;          /* intensité                          */
}                   t_light;

/* ------------------------------------------------------------------ */
/*  Caméra                                                             */
/* ------------------------------------------------------------------ */

typedef struct s_camera
{
    t_vec3    position;
    t_vec3    direction;      /* vecteur d'orientation (normalisé)  */
    double    fov;            /* degrés                             */
    int        width;
    int        height;
    t_vec3    forward;
    t_vec3    right;
    t_vec3    up;
}                   t_camera;

/* ------------------------------------------------------------------ */
/*  Scène                                                                */
/* ------------------------------------------------------------------ */

typedef struct s_scene
{
    t_color    ambient;
    double    ambient_ratio;
    t_camera  camera;
    t_light   light;
    t_obj     *objects;
    size_t    obj_count;
    size_t    obj_capacity;
}                   t_scene;

/* ------------------------------------------------------------------ */
/*  Jeu (MLX)                                                           */
/* ------------------------------------------------------------------ */

typedef struct s_mlx
{
    void    *display;
    void    *win;
    void    *img;
    char    *img_data;
    int        img_bpp;
    int        img_line;
    int        img_end;
    int        width;
    int        height;
}                   t_mlx;

typedef struct s_game
{
    t_mlx        mlx;
    t_scene      scene;
    int            bonus;
    int            frame;
    int            game_over;
    int            show_fps;
    /* clés */
    int            key_w, key_s, key_a, key_d;
    int            key_left, key_right, key_up, key_down;
    /* options bonus */
    int            multithread;
    int            antialias;
    int            supersample;
}                   t_game;

/* ------------------------------------------------------------------ */
/*  Math — vecteurs (vector.c)                                          */
/* ------------------------------------------------------------------ */

t_vec3    vec3_new(double x, double y, double z);
t_vec3    vec3_add(t_vec3 a, t_vec3 b);
t_vec3    vec3_sub(t_vec3 a, t_vec3 b);
t_vec3    vec3_scale(t_vec3 v, double s);
t_vec3    vec3_neg(t_vec3 v);
double    vec3_dot(t_vec3 a, t_vec3 b);
t_vec3    vec3_cross(t_vec3 a, t_vec3 b);
double    vec3_length(t_vec3 v);
double    vec3_length_sq(t_vec3 v);
t_vec3    vec3_normalize(t_vec3 v);
t_vec3    vec3_reflect(t_vec3 i, t_vec3 n);
t_vec3    vec3_refract(t_vec3 i, t_vec3 n, double eta);
double    vec3_dist(t_vec3 a, t_vec3 b);
t_vec3    vec3_lerp(t_vec3 a, t_vec3 b, double t);
t_vec3    vec3_min(t_vec3 a, t_vec3 b);
t_vec3    vec3_max(t_vec3 a, t_vec3 b);

/* ------------------------------------------------------------------ */
/*  Math — matrices (matrix.c)                                          */
/* ------------------------------------------------------------------ */

t_mat4    mat4_identity(void);
t_mat4    mat4_multiply(t_mat4 a, t_mat4 b);
t_vec3    mat4_transform_point(t_mat4 m, t_vec3 p);
t_vec3    mat4_transform_dir(t_mat4 m, t_vec3 d);
t_mat4    mat4_translate(double x, double y, double z);
t_mat4    mat4_rotate_x(double angle);   /* degrés */
t_mat4    mat4_rotate_y(double angle);
t_mat4    mat4_rotate_z(double angle);
t_mat4    mat4_rotate(t_mat4 m, char axis, double angle);
t_mat4    mat4_scale(double x, double y, double z);
t_mat4    mat4_rotation_axis(t_vec3 axis, double angle); /* Rodrigues, degres */
t_mat4    mat4_inverse(t_mat4 m);
int        mat4_invert(t_mat4 m, t_mat4 *out);
t_mat4    mat4_transpose(t_mat4 m);

/* ------------------------------------------------------------------ */
/*  Parsing (.rt)                                                       */
/* ------------------------------------------------------------------ */

int        parse_rt(const char *path, t_scene *scene);
int        parse_rt_file(FILE *f, t_scene *scene);
void    scene_free(t_scene *scene);
void    scene_add_object(t_scene *scene, t_obj *obj);
void    obj_free(t_obj *obj);
int        scene_validate(const t_scene *scene);

/* helpers de parsing */
int        parse_vec3(const char *s, t_vec3 *out);
int        parse_color(const char *s, t_color *out);
int        parse_double(const char *s, double *out);

/* ------------------------------------------------------------------ */
/*  Objets (objects.c)                                                  */
/* ------------------------------------------------------------------ */

void    obj_init(t_obj *obj, int type);
void    obj_apply_transform(t_obj *obj, const t_mat4 *t);
void    obj_compute_inverse(t_obj *obj);
t_obj    *obj_sphere(t_vec3 center, double diameter, t_color color);
t_obj    *obj_plane(t_vec3 point, t_vec3 normal, t_color color);
t_obj    *obj_cylinder(t_vec3 center, t_vec3 axis, double diameter,
                double height, t_color color);
t_obj    *obj_cone(t_vec3 apex, t_vec3 axis, double half_angle_deg,
                double height, t_color color);
t_obj    *obj_cube(double size, t_color color);
t_obj    *obj_cube_at(t_vec3 pos, t_vec3 orient, double size, t_color color);
t_obj    *obj_triangle(t_vec3 v0, t_vec3 v1, t_vec3 v2, t_color color);

/* ------------------------------------------------------------------ */
/*  Intersections (intersect.c)                                         */
/* ------------------------------------------------------------------ */

void    hit_reset(t_hit *hit);
int        obj_intersect(const t_obj *obj, const t_ray *ray, t_hit *hit);
int        intersect_sphere(const t_obj *obj, const t_ray *ray, t_hit *hit);
int        intersect_plane(const t_obj *obj, const t_ray *ray, t_hit *hit);
int        intersect_cylinder(const t_obj *obj, const t_ray *ray, t_hit *hit);
int        intersect_cone(const t_obj *obj, const t_ray *ray, t_hit *hit);
int        intersect_cube(const t_obj *obj, const t_ray *ray, t_hit *hit);
int        intersect_triangle(const t_obj *obj, const t_ray *ray, t_hit *hit);
int        scene_intersect(const t_scene *scene, const t_ray *ray, t_hit *hit);

/* ------------------------------------------------------------------ */
/*  Lumière (lighting.c)                                                */
/* ------------------------------------------------------------------ */

t_color    phong_shade(const t_scene *scene, const t_hit *hit,
                const t_vec3 *view_dir, int depth);
t_color    apply_material(const t_color *base, const t_material *mat,
                const t_vec3 *point);
int        in_shadow(const t_scene *scene, const t_vec3 *point,
                const t_vec3 *light_dir);
t_color    trace_ray(const t_scene *scene, const t_ray *ray, int depth);

/* ------------------------------------------------------------------ */
/*  Caméra / rendu (camera.c, render.c)                                */
/* ------------------------------------------------------------------ */

void    camera_init(t_camera *cam, t_vec3 pos, t_vec3 dir, double fov,
                int w, int h);
t_ray    camera_ray(const t_camera *cam, int x, int y);
t_color    render_pixel(const t_scene *scene, const t_camera *cam,
                int x, int y, int aa);
t_color    skybox_color(const t_ray *ray);

/* ------------------------------------------------------------------ */
/*  Boucle MLX (hooks.c, main.c)                                       */
/* ------------------------------------------------------------------ */

int        game_init(t_game *game, int bonus, int w, int h, int mt, int aa,
                const char *path);
void    game_destroy(t_game *game);
void    render_frame(t_game *game);
void    key_press(int keycode, t_game *game);
void    key_release(int keycode, t_game *game);
int        expose_hook(t_game *game);
int        loop_step(t_game *game);
int        game_loop(t_game *game);

/* ------------------------------------------------------------------ */
/*  Multithreading (bonus, mt.c)                                       */
/* ------------------------------------------------------------------ */

typedef struct s_mt_job
{
    const t_scene    *scene;
    const t_camera    *cam;
    int                y0, y1;
    int                aa;
    t_color            *out;   /* ligne de pixels */
    int                width;
}                   t_mt_job;

void    render_region(const t_scene *scene, const t_camera *cam,
                int y0, int y1, int aa, t_color *out, int width);
void    render_multithreaded(const t_scene *scene, const t_camera *cam,
                int height, int aa, t_color *out, int width, int threads);
int        get_num_threads(void);

/* ------------------------------------------------------------------ */
/*  Utilitaires                                                         */
/* ------------------------------------------------------------------ */

t_color    color_new(double r, double g, double b);
t_color    color_scale(t_color c, double s);
t_color    color_multiply(t_color a, t_color b);
t_color    color_add(t_color a, t_color b);
t_color    color_clamp(t_color c);
int        color_to_int(t_color c);
void    *xmalloc(size_t size);
void    *xcalloc(size_t count, size_t size);
void    *xrealloc(void *ptr, size_t size);

/* ------------------------------------------------------------------ */
/*  Textures (bonus, texture.c)                                        */
/* ------------------------------------------------------------------ */

int        texture_load(t_texture *tex, const char *path);
void    texture_free(t_texture *tex);
t_color    texture_sample(const t_texture *tex, double u, double v);
int        texture_load_by_name(t_texture *tex, const char *name);

# endif
