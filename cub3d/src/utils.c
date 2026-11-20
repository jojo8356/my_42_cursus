/* ************************************************************************** */
/*                                                                            */
/*   utils.c                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/20 by jojo8356            #+#    #+#             */
/*   Updated: 2026/11/20 by jojo8356           ###   ########.fr           */
/*                                                                            */
/* ************************************************************************** */

#include "cub3d.h"
# include <stdlib.h>
# include <string.h>
# include <stdio.h>

/* ---------------------------------------------------------------- */
/* xmalloc / xcalloc — allocations vérifiées                        */
/* ---------------------------------------------------------------- */

void    *xmalloc(size_t size)
{
    void    *p;

    p = malloc(size);
    if (!p)
    {
        fprintf(stderr, "cub3d: malloc failed (%zu bytes)\n", size);
        exit(EXIT_FAILURE);
    }
    memset(p, 0, size);
    return (p);
}

void    *xcalloc(size_t n, size_t size)
{
    return (xmalloc(n * size));
}

/* ---------------------------------------------------------------- */
/* xrealloc — allocation redimensionée vérifiée                     */
/* ---------------------------------------------------------------- */

void    *xrealloc(void *ptr, size_t size)
{
    void    *p;

    if (!ptr)
        return (xmalloc(size));
    p = realloc(ptr, size);
    if (!p)
    {
        fprintf(stderr, "cub3d: realloc failed (%zu bytes)\n", size);
        exit(EXIT_FAILURE);
    }
    return (p);
}

/* ---------------------------------------------------------------- */
/* Packing / unpacking des couleurs RGBA                            */
/* ---------------------------------------------------------------- */

t_color        color(int r, int g, int b, int a)
{
    return ((((t_color)(r & 0xFF) << 24)
        | ((t_color)(g & 0xFF) << 16)
        | ((t_color)(b & 0xFF) << 8)
        | ((t_color)(a & 0xFF))));
}

int            color_r(t_color c)
{
    return ((int)((c >> 24) & 0xFF));
}

int            color_g(t_color c)
{
    return ((int)((c >> 16) & 0xFF));
}

int            color_b(t_color c)
{
    return ((int)((c >> 8) & 0xFF));
}

int            color_a(t_color c)
{
    return ((int)(c & 0xFF));
}
