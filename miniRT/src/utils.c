/* ************************************************************************** */
/*                                                                            */
/*   utils.c                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Couleurs & allocations.                                                */
/* ************************************************************************** */

# include "minrt.h"
# include <stdlib.h>
# include <math.h>

void    *xmalloc(size_t size)
{
    void    *p;

    p = malloc(size);
    if (!p)
    {
        ft_putstr_fd("miniRT: allocation failed\n", 2);
        exit(1);
    }
    return (p);
}

void    *xcalloc(size_t count, size_t size)
{
    void    *p;

    p = calloc(count, size);
    if (!p)
    {
        ft_putstr_fd("miniRT: allocation failed\n", 2);
        exit(1);
    }
    return (p);
}

void    *xrealloc(void *ptr, size_t size)
{
    void    *p;

    if (!ptr)
        return (xmalloc(size));
    p = realloc(ptr, size);
    if (!p)
    {
        ft_putstr_fd("miniRT: reallocation failed\n", 2);
        exit(1);
    }
    return (p);
}

t_color    color_new(double r, double g, double b)
{
    t_color    c;

    c.r = r;
    c.g = g;
    c.b = b;
    return (c);
}

t_color    color_scale(t_color c, double s)
{
    return (color_new(c.r * s, c.g * s, c.b * s));
}

t_color    color_multiply(t_color a, t_color b)
{
    return (color_new(a.r * b.r, a.g * b.g, a.b * b.b));
}

t_color    color_add(t_color a, t_color b)
{
    return (color_new(a.r + b.r, a.g + b.g, a.b + b.b));
}

t_color    color_clamp(t_color c)
{
    return (color_new(fmin(fmax(c.r, 0.0), 1.0),
                      fmin(fmax(c.g, 0.0), 1.0),
                      fmin(fmax(c.b, 0.0), 1.0)));
}

int        color_to_int(t_color c)
{
    int        r, g, b;

    r = (int)(fmin(fmax(c.r, 0.0), 1.0) * 255.0 + 0.5);
    g = (int)(fmin(fmax(c.g, 0.0), 1.0) * 255.0 + 0.5);
    b = (int)(fmin(fmax(c.b, 0.0), 1.0) * 255.0 + 0.5);
    return ((r << 16) | (g << 8) | b);
}
