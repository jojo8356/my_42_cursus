/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cub3d.h                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/20 by jojo8356            #+#    #+#             */
/*   Updated: 2026/11/20 by jojo8356           ###   ########.fr           */
/*                                                                            */
/* ************************************************************************** */

#ifndef CUB3D_H
# define CUB3D_H

# include <stddef.h>

/* ------------------------------------------------------------------ */
/* MiniLibX (réel ou shim headless pour les tests)                    */
/* ------------------------------------------------------------------ */
typedef struct s_mlx_image
{
    int        width;
    int        height;
    int        bpp;
    int        size_line;
    int        endian;
    char       *data;
}               t_mlx_image;

typedef struct s_mlx
{
    void       *display;
    void       *win;
}               t_mlx;

void            *mlx_init(void);
void            *mlx_new_window(void *mlx, int w, int h, char *title);
int             mlx_clear_window(void *mlx, void *win);
int             mlx_pixel_put(void *mlx, void *win, int x, int y, int color);
void            *mlx_new_image(void *mlx, int w, int h);
char            *mlx_get_data_addr(void *img, int *bpp, int *line, int *end);
int             mlx_put_image_to_window(void *mlx, void *win, void *img, int x, int y);
int             mlx_string_put(void *mlx, void *win, int x, int y, int color, char *s);
void            *mlx_xpm_file_to_image(void *mlx, char *file, int *w, int *h);
int             mlx_destroy_image(void *mlx, void *img);
int             mlx_destroy_window(void *mlx, void *win);
int             mlx_destroy_display(void *mlx);
int             mlx_key_hook(void *win, int (*fn)(), void *param);
int             mlx_hook(void *win, int event, int mask, int (*fn)(), void *param);
int             mlx_loop_hook(void *mlx, int (*fn)(), void *param);
int             mlx_loop(void *mlx);
int             mlx_loop_end(void *mlx);
int             mlx_expose_hook(void *win, int (*fn)(), void *param);

/* ------------------------------------------------------------------ */
/* Types                                                              */
/* ------------------------------------------------------------------ */

/* direction du joueur */
# define DIR_N 0
# define DIR_S 1
# define DIR_E 2
# define DIR_W 3

/* type de cellule de la carte */
# define CELL_EMPTY 0
# define CELL_WALL 1
# define CELL_DOOR 2
# define CELL_SPRITE 3
# define CELL_ANIM_SPRITE 4

/* état d'une porte */
# define DOOR_CLOSED 0
# define DOOR_OPEN 1
# define DOOR_ANIMATING 2

/* couleur RGBA packée (0xRRGGBBAA) */
typedef unsigned int    t_color;

/* texture XPM chargée */
typedef struct s_texture
{
    void            *img;
    int             width;
    int             height;
    char            *data;
    int             bpp;
    int             size_line;
    int             endian;
    int             loaded;
}                   t_texture;

/* textures du jeu */
typedef struct s_textures
{
    t_texture        north;
    t_texture        south;
    t_texture        west;
    t_texture        east;
    t_texture        door;
    t_texture        sprite;
    t_texture        anim1;
    t_texture        anim2;
    t_texture        floor;
    t_texture        ceiling;
}                   t_textures;

/* carte 2D + infos */
typedef struct s_map
{
    char            **grid;          /* lignes de la carte */
    int             rows;            /* nombre de lignes */
    int             cols;            /* nombre de colonnes */
    int             player_x;        /* position du joueur (colonne) */
    int             player_y;        /* position du joueur (ligne) */
    int             dir;             /* direction initiale N/S/E/W */
    t_color         floor;           /* couleur sol */
    t_color         ceiling;         /* couleur plafond */
    char            *tex_no;         /* chemin texture NO */
    char            *tex_so;
    char            *tex_we;
    char            *tex_ea;
    char            *tex_do;         /* bonus : porte */
    char            *tex_sp;         /* bonus : sprite */
    char            *tex_a1;         /* bonus : frame animée 1 */
    char            *tex_a2;         /* bonus : frame animée 2 */
    int             has_no, has_so, has_we, has_ea;
    int             has_do, has_sp, has_a1, has_a2;
    int             doors_count;
    int             sprites_count;
    int             anim_sprites_count;
    int             bonus_textures; /* 1 si textures bonus présentes */
}                   t_map;

/* joueur */
typedef struct s_player
{
    double          x;                /* position en coordonnées réelles */
    double          y;
    double          dir_x;            /* vecteur direction (norme 1) */
    double          dir_y;
    double          plane_x;          /* vecteur plan (perpendiculaire, norme 0.66) */
    double          plane_y;
    double          move_speed;       /* vitesse de déplacement */
    double          rot_speed;        /* vitesse de rotation */
    int             key_w, key_s, key_a, key_d;
    int             key_left, key_right;
}                   t_player;

/* rayon calculé par le raycasting */
typedef struct s_ray
{
    double          start_x;
    double          start_y;
    double          dir_x;
    double          dir_y;
    int             hit;             /* 1 si mur touché */
    int             side;            /* 0=N/S, 1=E/W */
    double          perp_wall_dist;  /* distance perpendiculaire au mur */
    int             tile_x;          /* coordonnée grille du mur */
    int             tile_y;
    int             wall_type;       /* CELL_WALL / CELL_DOOR */
    int             texture;         /* NO/SO/WE/EA/DO pour la texture */
}                   t_ray;

/* sprite (bonus) */
typedef struct s_sprite
{
    double          x;
    double          y;
    int             type;            /* CELL_SPRITE / CELL_ANIM_SPRITE */
    int             frame;           /* frame animée (0/1) */
    int             id;
    double          dist;            /* distance au joueur (pour tri) */
}                   t_sprite;

/* porte (bonus) */
typedef struct s_door
{
    double          x;
    double          y;
    int             state;           /* DOOR_CLOSED / OPEN / ANIMATING */
    double          open_progress;   /* 0..1 animation */
    double          dist;
}                   t_door;

/* jeu global */
typedef struct s_game
{
    t_mlx           mlx;
    void            *win;
    t_map           map;
    t_player        player;
    t_textures      tex;
    int             bonus;           /* 1 si mode bonus */
    int             width;           /* largeur fenêtre */
    int             height;          /* hauteur fenêtre */
    int             frame;           /* compteur de frames */
    int             game_over;
    int             show_minimap;    /* 1 si minimap affiché */
    int             door_key;        /* état de la touche porte (E) */
    t_sprite        *sprites;        /* tableau sprites */
    int             sprites_n;
    t_door          *doors;          /* tableau portes */
    int             doors_n;
    double          last_frame_time;
    int             frame_rate_cap;
}                   t_game;

/* ------------------------------------------------------------------ */
/* Parser                                                              */
/* ------------------------------------------------------------------ */

int             parse_cub(const char *path, t_map *map);
void            map_free(t_map *map);
int             map_validate(t_map *map);

/* ------------------------------------------------------------------ */
/* Textures                                                            */
/* ------------------------------------------------------------------ */

int             load_textures(t_game *game);
void            free_textures(t_game *game);

/* ------------------------------------------------------------------ */
/* Joueur / mouvement                                                  */
/* ------------------------------------------------------------------ */

void            player_init(t_game *game);
void            player_move(t_game *game);
void            player_rotate(t_game *game, int dir);
int             can_move_to(t_game *game, double x, double y);
void            update_doors(t_game *game);
void            update_sprites(t_game *game);

/* ------------------------------------------------------------------ */
/* Raycasting                                                          */
/* ------------------------------------------------------------------ */

void            cast_rays(t_game *game, t_ray *rays, int n);
void            ray_init(t_ray *ray, double x, double y, double dir_x, double dir_y);
void            ray_dda(t_game *game, t_ray *ray);
int             ray_hit_door(t_game *game, t_ray *ray);

void            collect_sprites(t_game *game);
void            collect_doors(t_game *game);

/* ------------------------------------------------------------------ */
/* Rendering                                                           */
/* ------------------------------------------------------------------ */

void            render_frame(t_game *game);
void            draw_floor_ceiling(t_game *game, t_ray *rays, int n);
void            draw_walls(t_game *game, t_ray *rays, int n);
void            draw_sprites(t_game *game);
void            draw_minimap(t_game *game);
void            draw_hud(t_game *game);

/* ------------------------------------------------------------------ */
/* Hooks / boucle                                                      */
/* ------------------------------------------------------------------ */

int             key_press(int keycode, void *param);
int             key_release(int keycode, void *param);
int             expose_hook(void *param);
void            game_loop(t_game *game);
int             game_init(t_game *game, int bonus, const char *path);
void            game_destroy(t_game *game);

/* ------------------------------------------------------------------ */
/* Utilitaires                                                          */
/* ------------------------------------------------------------------ */

t_color         color(int r, int g, int b, int a);
int             color_r(t_color c);
int             color_g(t_color c);
int             color_b(t_color c);
int             color_a(t_color c);
void            *xrealloc(void *ptr, size_t size);
void            *xmalloc(size_t size);
void            *xcalloc(size_t n, size_t size);

/* constantes */
# define FOV 0.66
# define MOVE_SPEED 0.08
# define ROT_SPEED 0.04
# define WINDOW_W 800
# define WINDOW_H 600

#endif
