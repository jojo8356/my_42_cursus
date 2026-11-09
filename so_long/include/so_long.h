/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   so_long.h                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/08 22:23:12 by jojo8356          #+#    #+#             */
/*   Updated: 2026/11/08 22:23:12 by jojo8356         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SO_LONG_H
# define SO_LONG_H

# include <stdlib.h>
# include <unistd.h>
# include <fcntl.h>
# include <stdio.h>
# include "../libft/libft.h"

# define TILE 64
# define KEY_ESC 65307
# define KEY_W 119
# define KEY_A 97
# define KEY_S 115
# define KEY_D 100

# define T_FLOOR '0'
# define T_WALL '1'
# define T_COLLECT 'C'
# define T_EXIT 'E'
# define T_PLAYER 'P'
# define T_ENEMY 'X'

typedef struct s_map
{
	char	**grid;
	int		width;
	int		height;
	int		collectibles;
	int		start_x;
	int		start_y;
}	t_map;

typedef struct s_img
{
	void	*ptr;
	int		width;
	int		height;
}	t_img;

typedef struct s_enemy
{
	int		x;
	int		y;
	int		dir;
}	t_enemy;

typedef struct s_game
{
	void	*mlx;
	void	*win;
	t_map	map;
	t_img	img[9];
	t_enemy	*enemies;
	int		enemy_count;
	int		player_x;
	int		player_y;
	int		moves;
	int		collected;
	int		frame;
	int		bonus;
	int		over;
}	t_game;

/* ---- parsing (mandatory) ---- */
int		map_load(const char *path, t_map *map);
int		map_valid(t_map *map, const char *path);
int		check_sprites(t_map *m, const char *path);
void	map_free(t_map *map);
void	err_exit(const char *msg);
int		bad_map(const char *path, const char *msg);
int		charset_ok(char c);
int		bonus_charset(char c);

/* ---- file helpers ---- */
char	*read_file(const char *path);
int		count_lines(const char *content);
char	**split_lines(const char *content, int height);
void	map_free_rows(char **rows);
int		ps_len(const char *s);
int		has_ber_extension(const char *path);
int		count_char(const t_map *m, char c);
void	find_player(t_map *m);

/* ---- rendering ---- */
int		render_init(t_game *g, int bonus);
void	render_draw(t_game *g);
void	render_close(t_game *g);
char	tile_at(const t_map *m, int x, int y);

/* ---- game loop (mandatory) ---- */
int		game_init(t_game *g, const char *path);
void	game_move(t_game *g, int dx, int dy);
int		game_hooks(t_game *g);
int		game_loop(t_game *g);

/* ---- bonus ---- */
int		bonus_spawn_enemies(t_game *g);
void	bonus_tick(t_game *g);
void	bonus_draw_hud(t_game *g);
void	bonus_frame(t_game *g);

#endif
