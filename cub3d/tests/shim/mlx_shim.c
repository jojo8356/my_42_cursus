/* ************************************************************************** */
/*                                                                            */
/*   mlx_shim.c — remplacement headless de miniLibX pour les tests.         */
/*                                                                            */
/*   Toutes les actions observables sont journalisées dans $MLX_SHIM_LOG.   */
/*   Les fichiers XPM sont réellement interprétés pour que le chargement   */
/*   des textures soit testable sans écran.                                  */
/*                                                                            */
/* ************************************************************************** */

#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include "mlx.h"

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
	FILE	*f;
	va_list	ap;

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
	logf_("clear_window");
	return (0);
}

int	mlx_pixel_put(void *mlx, void *win, int x, int y, int color)
{
	(void)mlx; (void)win;
	logf_("pixel_put %d %d %d", x, y, color);
	return (0);
}

void	*mlx_new_image(void *mlx, int w, int h)
{
	t_shim_image	*img;

	(void)mlx;
	img = (t_shim_image *)calloc(1, sizeof(t_shim_image));
	if (!img)
		return (NULL);
	img->width = w;
	img->height = h;
	img->bpp = 32;
	img->size_line = w * 4;
	img->endian = 0;
	img->data = (char *)calloc((size_t)w * (size_t)h * 4, 1);
	return (img);
}

char	*mlx_get_data_addr(void *img, int *bpp, int *line, int *end)
{
	t_shim_image	*im = (t_shim_image *)img;

	if (bpp)
		*bpp = im->bpp;
	if (line)
		*line = im->size_line;
	if (end)
		*end = im->endian;
	return (im->data);
}

int	mlx_put_image_to_window(void *mlx, void *win, void *img, int x, int y)
{
	(void)mlx; (void)win; (void)img;
	logf_("put_image %d %d", x, y);
	return (0);
}

int	mlx_string_put(void *mlx, void *win, int x, int y, int color, char *s)
{
	(void)mlx; (void)win;
	logf_("string_put %d %d %d %s", x, y, color, s ? s : "");
	return (0);
}

/* ---- Lecture XPM réelle (palette + pixels) -> RGBA 32bpp ---- */

static int	hex_val(char c)
{
	if (c >= '0' && c <= '9')
		return (c - '0');
	if (c >= 'a' && c <= 'f')
		return (c - 'a' + 10);
	if (c >= 'A' && c <= 'F')
		return (c - 'A' + 10);
	return (-1);
}

static int	parse_hex_color(const char *s, int *r, int *g, int *b)
{
	int h0, h1, h2, h3, h4, h5;

	if (s[0] != '#')
		return (0);
	h0 = hex_val(s[1]); h1 = hex_val(s[2]);
	h2 = hex_val(s[3]); h3 = hex_val(s[4]);
	h4 = hex_val(s[5]); h5 = hex_val(s[6]);
	if (h0 < 0 || h1 < 0 || h2 < 0 || h3 < 0 || h4 < 0 || h5 < 0)
		return (0);
	*r = (h0 * 16 + h1);
	*g = (h2 * 16 + h3);
	*b = (h4 * 16 + h5);
	return (1);
}

static void	xpm_set_pixel(char *data, int size_line, int x, int y,
	int r, int g, int b, int a)
{
	size_t    idx = (size_t)y * size_line + (size_t)x * 4;
	data[idx] = (char)r;
	data[idx + 1] = (char)g;
	data[idx + 2] = (char)b;
	data[idx + 3] = (char)a;
}

void	*mlx_xpm_file_to_image(void *mlx, char *file, int *w, int *h)
{
	FILE	*f;
	char	buf[65536];
	char	*p;
	char	*tok;
	t_shim_image	*img;
	int	width = 0;
	int	height = 0;
	/* palette : char -> RGBA */
	char	pal_char[256];
	int	pal_r[256], pal_g[256], pal_b[256], pal_a[256];
	int	pal_n = 0;
	int	row = 0;
	int	col = 0;
	(void)mlx;

	memset(pal_char, -1, sizeof(pal_char));
	f = fopen(file, "r");
	if (!f)
	{
		logf_("xpm_fail %s", file ? file : "");
		return (NULL);
	}
	/* lire tout le fichier */
	{
		size_t	n = fread(buf, 1, sizeof(buf) - 1, f);
		buf[n] = '\0';
		fclose(f);
	}
	/* premiere ligne : XPM ; deuxieme : width= height= */
	p = strstr(buf, "width=");
	if (p)
		width = atoi(p + 6);
	p = strstr(buf, "height=");
	if (p)
		height = atoi(p + 7);
	if (width <= 0 || height <= 0)
		width = height = 16;

	img = (t_shim_image *)calloc(1, sizeof(t_shim_image));
	img->width = width;
	img->height = height;
	img->bpp = 32;
	img->size_line = width * 4;
	img->endian = 0;
	img->data = (char *)calloc((size_t)width * (size_t)height * 4, 1);

	/* parcourir les tokens entre { et } */
	p = strchr(buf, '{');
	if (!p)
	{
		if (w) *w = width;
		if (h) *h = height;
		logf_("xpm %s", file ? file : "");
		return (img);
	}
	p++;
	/* tokeniser par '"' */
	while (*p && *p != '}')
	{
		if (*p != '"')
		{
			p++;
			continue;
		}
		p++;
		tok = p;
		while (*p && *p != '"')
			p++;
		if (*p == '"')
			*p++ = '\0';
		/* si le token contient " c ", c'est une palette */
		if (strstr(tok, " c "))
		{
			char	*sp = strstr(tok, " c ");
			int	r, g, b;
			*sp = '\0';
			if (pal_n < 256 && tok[0])
			{
				pal_char[(unsigned char)tok[0]] = pal_n;
				if (parse_hex_color(sp + 3, &r, &g, &b))
				{
					pal_r[pal_n] = r;
					pal_g[pal_n] = g;
					pal_b[pal_n] = b;
					pal_a[pal_n] = 255;
				}
				pal_n++;
			}
		}
		else
		{
			/* pixel : un ou plusieurs caractères (palette key) */
			int	k;
			for (k = 0; tok[k] && pal_char[(unsigned char)tok[k]] >= 0; k++)
			{
				int	idx = (int)pal_char[(unsigned char)tok[k]];
				int	r, g, b, a;
				r = pal_r[idx]; g = pal_g[idx]; b = pal_b[idx]; a = pal_a[idx];
				if (col >= width)
				{
					col = 0;
					row++;
				}
				if (row < height && col < width)
					xpm_set_pixel(img->data, img->size_line, col, row, r, g, b, a);
				col++;
			}
		}
	}
	if (w)
		*w = width;
	if (h)
		*h = height;
	logf_("xpm %s %d %d", file ? file : "", width, height);
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

int	mlx_key_hook(void *win, int (*fn)(), void *param)
{
	(void)win; (void)fn; (void)param;
	logf_("key_hook");
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

int	mlx_loop_hook(void *mlx, int (*fn)(), void *param)
{
	(void)mlx;
	g_loop_fn = fn;
	g_loop_param = param;
	logf_("loop_hook");
	return (0);
}

int	mlx_loop(void *mlx)
{
	int	frames = 60;
	const char	*env = getenv("MLX_SHIM_FRAMES");

	(void)mlx;
	if (env)
		frames = atoi(env);
	logf_("loop %d", frames);
	for (int i = 0; i < frames && g_loop_fn; i++)
		g_loop_fn(g_loop_param);
	return (0);
}

int	mlx_loop_end(void *mlx)
{
	(void)mlx;
	logf_("loop_end");
	return (0);
}

int	mlx_expose_hook(void *win, int (*fn)(), void *param)
{
	(void)win; (void)fn; (void)param;
	logf_("expose_hook");
	return (0);
}
