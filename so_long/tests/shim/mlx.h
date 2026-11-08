#ifndef SHIM_MLX_H
# define SHIM_MLX_H

/*
** Drop-in replacement for the minilibX header, used only by the test suite.
**
** The real miniLibX needs an X server; a CI machine has none. This shim keeps
** the exact prototypes of mlx.h so the project compiles untouched, and records
** every call in $MLX_SHIM_LOG instead of drawing. That is what makes the game
** testable: the tests can assert which images were blitted where, how many
** frames were drawn, and which hooks were registered -- without a display.
*/

typedef struct s_shim_image
{
	int		width;
	int		height;
	int		bpp;
	int		size_line;
	int		endian;
	char	*data;
}	t_shim_image;

void	*mlx_init(void);
void	*mlx_new_window(void *mlx, int w, int h, char *title);
int		mlx_clear_window(void *mlx, void *win);
int		mlx_pixel_put(void *mlx, void *win, int x, int y, int color);
void	*mlx_new_image(void *mlx, int w, int h);
char	*mlx_get_data_addr(void *img, int *bpp, int *line, int *end);
int		mlx_put_image_to_window(void *mlx, void *win, void *img, int x, int y);
int		mlx_string_put(void *mlx, void *win, int x, int y, int color, char *s);
void	*mlx_xpm_file_to_image(void *mlx, char *file, int *w, int *h);
int		mlx_destroy_image(void *mlx, void *img);
int		mlx_destroy_window(void *mlx, void *win);
int		mlx_destroy_display(void *mlx);
int		mlx_key_hook(void *win, int (*fn)(), void *param);
int		mlx_hook(void *win, int event, int mask, int (*fn)(), void *param);
int		mlx_loop_hook(void *mlx, int (*fn)(), void *param);
int		mlx_loop(void *mlx);
int		mlx_loop_end(void *mlx);
int		mlx_expose_hook(void *win, int (*fn)(), void *param);

#endif
