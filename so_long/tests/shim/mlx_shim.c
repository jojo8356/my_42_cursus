#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include "mlx.h"

/*
** Headless stand-in for the minilibX. Every observable action is appended to
** $MLX_SHIM_LOG so the test suite can inspect it; drawing itself is a no-op.
**
** MLX_SHIM_FRAMES controls how many times mlx_loop runs the loop hook before
** returning, which turns the interactive game into a finite, scriptable run.
** MLX_SHIM_KEYS is an optional script of keycodes fed one per frame, so a test
** can play a full winning game and check the move count.
*/

static FILE	*log_file(void)
{
	static FILE	*f;
	const char	*path;

	if (!f)
	{
		path = getenv("MLX_SHIM_LOG");
		f = (path && *path) ? fopen(path, "a") : fopen("/dev/null", "a");
		if (!f)
			f = fopen("/dev/null", "a");
	}
	return (f);
}

static void	logf_(const char *fmt, ...)
{
	FILE		*f;
	va_list		ap;

	f = log_file();
	if (!f)
		return ;
	va_start(ap, fmt);
	vfprintf(f, fmt, ap);
	va_end(ap);
	fputc('\n', f);
	fflush(f);
}

void	*mlx_init(void)
{
	logf_("mlx_init");
	return ((void *)(size_t)1);
}

void	*mlx_new_window(void *mlx, int w, int h, char *title)
{
	(void)mlx;
	logf_("new_window %d %d %s", w, h, title ? title : "");
	return ((void *)(size_t)2);
}

int	mlx_clear_window(void *mlx, void *win)
{
	(void)mlx; (void)win;
	logf_("clear");
	return (0);
}

int	mlx_pixel_put(void *mlx, void *win, int x, int y, int color)
{
	(void)mlx; (void)win; (void)x; (void)y; (void)color;
	return (0);
}

void	*mlx_new_image(void *mlx, int w, int h)
{
	t_shim_image	*img;

	(void)mlx;
	img = malloc(sizeof(*img));
	if (!img)
		return (NULL);
	img->width = w;
	img->height = h;
	img->bpp = 32;
	img->size_line = w * 4;
	img->endian = 0;
	img->data = calloc((size_t)(w * h * 4) + 1, 1);
	logf_("new_image %d %d", w, h);
	return (img);
}

char	*mlx_get_data_addr(void *img, int *bpp, int *line, int *end)
{
	t_shim_image	*i;

	i = (t_shim_image *)img;
	if (!i)
		return (NULL);
	if (bpp)
		*bpp = i->bpp;
	if (line)
		*line = i->size_line;
	if (end)
		*end = i->endian;
	return (i->data);
}

/*
** The image pointer is logged as well as its size: render.c blits nine distinct
** images, and the animation test needs to tell "the player's first frame" from
** "the player's second frame" across two frames of the same tile.
*/

int	mlx_put_image_to_window(void *mlx, void *win, void *img, int x, int y)
{
	t_shim_image	*i;

	(void)mlx; (void)win;
	i = (t_shim_image *)img;
	logf_("put %p %dx%d %d %d", img, i ? i->width : 0, i ? i->height : 0,
		x, y);
	return (0);
}

int	mlx_string_put(void *mlx, void *win, int x, int y, int c, char *s)
{
	(void)mlx; (void)win; (void)c;
	logf_("string %d %d %s", x, y, s ? s : "");
	return (0);
}

/*
** A real allocated image, like mlx_new_image: the game destroys its images at
** exit, and a fake pointer here would make mlx_destroy_image free garbage. The
** file name is kept so the log can prove which XPMs were actually loaded.
*/

void	*mlx_xpm_file_to_image(void *mlx, char *file, int *w, int *h)
{
	void	*img;

	img = mlx_new_image(mlx, 64, 64);
	if (w)
		*w = 64;
	if (h)
		*h = 64;
	logf_("xpm %s", file ? file : "");
	return (img);
}

int	mlx_destroy_image(void *mlx, void *img)
{
	(void)mlx;
	if (img)
	{
		free(((t_shim_image *)img)->data);
		free(img);
	}
	return (0);
}

int	mlx_destroy_window(void *mlx, void *win)
{
	(void)mlx; (void)win;
	logf_("destroy_window");
	return (0);
}

int	mlx_destroy_display(void *mlx)
{
	(void)mlx;
	logf_("destroy_display");
	return (0);
}

int	mlx_hook(void *win, int event, int mask, int (*fn)(), void *param)
{
	(void)win; (void)mask; (void)fn; (void)param;
	logf_("hook %d", event);
	return (0);
}

static int	(*g_loop_fn)(void *);
static void	*g_loop_param;
static int	(*g_key_fn)(int, void *);
static void	*g_key_param;

int	mlx_loop_hook(void *mlx, int (*fn)(), void *param)
{
	(void)mlx;
	g_loop_fn = (int (*)(void *))fn;
	g_loop_param = param;
	logf_("hook_loop");
	return (0);
}

int	mlx_key_hook(void *win, int (*fn)(), void *param)
{
	(void)win;
	g_key_fn = (int (*)(int, void *))fn;
	g_key_param = param;
	logf_("hook_key");
	return (0);
}

int	mlx_expose_hook(void *win, int (*fn)(), void *param)
{
	(void)win; (void)fn; (void)param;
	logf_("hook_expose");
	return (0);
}

int	mlx_loop_end(void *mlx)
{
	(void)mlx;
	logf_("loop_end");
	return (0);
}

/*
** The loop itself. Without a window there is nothing to wait for, so the shim
** drives the game deterministically: it feeds the keys of $MLX_SHIM_KEYS one
** per frame, calls the loop hook $MLX_SHIM_FRAMES times, and returns. A test
** can therefore play a whole game, including a losing one, and inspect the
** resulting state.
*/

/*
** Splits the key script on spaces/commas and returns the next code, or -1.
*/

static int	next_key(const char *seq, int *pos)
{
	int	n;

	while (seq[*pos] == ' ' || seq[*pos] == ',')
		(*pos)++;
	if (!seq[*pos])
		return (-1);
	n = 0;
	while (seq[*pos] >= '0' && seq[*pos] <= '9')
	{
		n = n * 10 + (seq[*pos] - '0');
		(*pos)++;
	}
	if (n == 0)
		(*pos)++;
	return (n);
}

int	mlx_loop(void *mlx)
{
	const char	*seq;
	int			frames;
	int			i;
	int			pos;
	int			key;

	(void)mlx;
	logf_("loop");
	seq = getenv("MLX_SHIM_KEYS");
	if (!seq)
		seq = "";
	frames = 1;
	if (getenv("MLX_SHIM_FRAMES"))
		frames = atoi(getenv("MLX_SHIM_FRAMES"));
	pos = 0;
	i = 0;
	while (i < frames)
	{
		key = next_key(seq, &pos);
		if (key > 0 && g_key_fn)
		{
			logf_("key %d", key);
			g_key_fn(key, g_key_param);
		}
		logf_("frame %d", i);
		if (g_loop_fn)
			g_loop_fn(g_loop_param);
		i++;
	}
	logf_("loop_exit");
	return (0);
}
