/*
** test_mlx_stub.c — Minimal mlx function stubs for headless testing.
**
** Provides stub implementations of mlx functions so that source files
** that reference mlx can be compiled and tested without X11.
**
** The signatures match 42Paris/minilibx-linux mlx.h exactly.
*/

# include "fdf.h"
# include <stdlib.h>
# include <string.h>

void *mlx_init(void) { return calloc(1, 16); }
void *mlx_new_window(void *mlx_ptr, int size_x, int size_y, char *title)
{ (void)mlx_ptr; (void)size_x; (void)size_y; (void)title; return calloc(1, 16); }
int mlx_clear_window(void *mlx_ptr, void *win_ptr) { (void)mlx_ptr; (void)win_ptr; return 0; }
void *mlx_new_image(void *mlx_ptr, int width, int height)
{ (void)mlx_ptr; (void)width; (void)height; return calloc(1, 16); }
char *mlx_get_data_addr(void *img_ptr, int *bits_per_pixel,
                        int *line_size, int *endian)
{ (void)img_ptr; *bits_per_pixel = 32; *line_size = 3200; *endian = 0;
  return calloc(3200 * 600, 1); }
int mlx_put_image_to_window(void *mlx_ptr, void *win_ptr, void *img_ptr,
                            int pos_x, int pos_y)
{ (void)mlx_ptr; (void)win_ptr; (void)img_ptr; (void)pos_x; (void)pos_y; return 0; }
int mlx_string_put(void *mlx_ptr, void *win_ptr, int x, int y,
                   int color, char *str)
{ (void)mlx_ptr; (void)win_ptr; (void)x; (void)y; (void)color; (void)str; return 0; }
int mlx_loop_hook(void *mlx_ptr, int (*funct_ptr)(), void *param)
{ (void)mlx_ptr; (void)funct_ptr; (void)param; return 0; }
int mlx_loop(void *mlx_ptr) { (void)mlx_ptr; return 0; }
int mlx_loop_end(void *mlx_ptr) { (void)mlx_ptr; return 0; }
int mlx_destroy_window(void *mlx_ptr, void *win_ptr)
{ (void)mlx_ptr; free(win_ptr); return 0; }
int mlx_destroy_image(void *mlx_ptr, void *img_ptr)
{ (void)mlx_ptr; free(img_ptr); return 0; }
int mlx_destroy_display(void *mlx_ptr) { (void)mlx_ptr; return 0; }
int mlx_hook(void *win_ptr, int x_event, int x_mask,
             int (*funct_ptr)(), void *param)
{ (void)win_ptr; (void)x_event; (void)x_mask; (void)funct_ptr; (void)param; return 0; }

/* Stub implementations for render.c functions. */
int redraw(t_env *env) { (void)env; return 0; }
void cleanup(t_env *env) { (void)env; }
