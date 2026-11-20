/* ************************************************************************** */
/*                                                                            */
/*   hooks.c                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Hooks MLX, mouvement caméra, boucle de rendu.                        */
/* ************************************************************************** */

# include "minrt.h"
# include "mlx.h"
# include <math.h>

# define MOVE_SPEED    0.05
# define ROT_SPEED    0.04

static void    move_camera(t_game *game)
{
    t_camera    *cam;
    t_vec3    move;

    cam = &game->scene.camera;
    move = vec3_new(0.0, 0.0, 0.0);
    if (game->key_w)
        move = vec3_add(move, cam->forward);
    if (game->key_s)
        move = vec3_sub(move, cam->forward);
    if (game->key_d)
        move = vec3_add(move, cam->right);
    if (game->key_a)
        move = vec3_sub(move, cam->right);
    if (vec3_length_sq(move) > EPS)
    {
        move = vec3_normalize(move);
        cam->position = vec3_add(cam->position,
            vec3_scale(move, MOVE_SPEED));
        camera_init(cam, cam->position, cam->direction, cam->fov,
            cam->width, cam->height);
    }
}

static void    rotate_camera(t_game *game)
{
    t_camera    *cam;
    t_mat4    rot;
    t_vec3    new_dir;

    cam = &game->scene.camera;
    if (game->key_left)
    {
        rot = mat4_rotation_axis(cam->up, ROT_SPEED * 57.29578);
        new_dir = vec3_normalize(mat4_transform_dir(rot, cam->forward));
    }
    else if (game->key_right)
    {
        rot = mat4_rotation_axis(cam->up, -ROT_SPEED * 57.29578);
        new_dir = vec3_normalize(mat4_transform_dir(rot, cam->forward));
    }
    else if (game->key_up)
    {
        rot = mat4_rotation_axis(cam->right, ROT_SPEED * 57.29578);
        new_dir = vec3_normalize(mat4_transform_dir(rot, cam->forward));
    }
    else if (game->key_down)
    {
        rot = mat4_rotation_axis(cam->right, -ROT_SPEED * 57.29578);
        new_dir = vec3_normalize(mat4_transform_dir(rot, cam->forward));
    }
    else
        return;
    camera_init(cam, cam->position, new_dir, cam->fov, cam->width, cam->height);
}

void    key_press(int keycode, t_game *game)
{
    switch (keycode)
    {
        case 119: case 65: game->key_w = !game->key_w; break;
        case 115: game->key_s = !game->key_s; break;
        case 97: game->key_a = !game->key_a; break;
        case 100: game->key_d = !game->key_d; break;
        case 63234: game->key_left = !game->key_left; break;
        case 63236: game->key_right = !game->key_right; break;
        case 63232: game->key_up = !game->key_up; break;
        case 63235: game->key_down = !game->key_down; break;
        case 49: game->multithread = !game->multithread; break;
        case 50: game->antialias = !game->antialias; break;
        case 113: game->game_over = 1; break;
        default: break;
    }
}

void    key_release(int keycode, t_game *game)
{
    (void)keycode;
    (void)game;
}

static int    key_hook(int keycode, t_game *game)
{
    key_press(keycode, game);
    return (0);
}

int    expose_hook(t_game *game)
{
    render_frame(game);
    mlx_put_image_to_window(game->mlx.display, game->mlx.win,
        game->mlx.img, 0, 0);
    return (0);
}

int    loop_step(t_game *game)
{
    if (game->game_over)
        return (0);
    move_camera(game);
    rotate_camera(game);
    game->frame++;
    expose_hook(game);
    return (0);
}

int    game_init(t_game *game, int bonus, int w, int h, int mt, int aa,
        const char *path)
{
    ft_bzero(game, sizeof(*game));
    game->bonus = bonus;
    game->multithread = mt;
    game->antialias = aa;
    game->mlx.width = w;
    game->mlx.height = h;
    game->mlx.display = mlx_init();
    if (!game->mlx.display)
        return (-1);
    game->mlx.win = mlx_new_window(game->mlx.display,
        game->mlx.width, game->mlx.height, "miniRT");
    game->mlx.img = mlx_new_image(game->mlx.display,
        game->mlx.width, game->mlx.height);
    game->mlx.img_data = mlx_get_data_addr(game->mlx.img,
        &game->mlx.img_bpp, &game->mlx.img_line, &game->mlx.img_end);
    if (parse_rt(path, &game->scene) < 0)
        return (-1);
    game->scene.camera.width = game->mlx.width;
    game->scene.camera.height = game->mlx.height;
    camera_init(&game->scene.camera, game->scene.camera.position,
        game->scene.camera.direction, game->scene.camera.fov,
        game->mlx.width, game->mlx.height);
    mlx_key_hook(game->mlx.win, key_hook, game);
    mlx_expose_hook(game->mlx.win, expose_hook, game);
    mlx_loop_hook(game->mlx.display, loop_step, game);
    return (0);
}

void    game_destroy(t_game *game)
{
    if (game->mlx.img)
        mlx_destroy_image(game->mlx.display, game->mlx.img);
    if (game->mlx.win)
        mlx_destroy_window(game->mlx.display, game->mlx.win);
    if (game->mlx.display)
        mlx_destroy_display(game->mlx.display);
    scene_free(&game->scene);
}

int    game_loop(t_game *game)
{
    expose_hook(game);
    mlx_loop(game->mlx.display);
    return (0);
}
