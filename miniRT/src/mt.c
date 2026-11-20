/* ************************************************************************** */
/*                                                                            */
/*   mt.c                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Multithreading du rendu (pthreads, lignes disjointes).                */
/* ************************************************************************** */

# include "minrt.h"
# include <pthread.h>
# include <unistd.h>

typedef struct s_mt_worker
{
    const t_scene    *scene;
    const t_camera    *cam;
    int                y0;
    int                y1;
    int                aa;
    t_color            *out;
    int                width;
}                   t_mt_worker;

static void    *worker(void *arg)
{
    t_mt_worker    *w;

    w = (t_mt_worker *)arg;
    render_region(w->scene, w->cam, w->y0, w->y1, w->aa,
        w->out, w->width);
    return (NULL);
}

int    get_num_threads(void)
{
    long    n;

    n = sysconf(_SC_NPROCESSORS_ONLN);
    if (n < 1)
        n = 1;
    return ((int)n);
}

void    render_multithreaded(const t_scene *scene, const t_camera *cam,
                int height, int aa, t_color *out, int width, int threads)
{
    pthread_t        *tids;
    t_mt_worker       *workers;
    int                i;
    int                base;

    if (threads < 1)
        threads = 1;
    if (threads > height)
        threads = height;
    tids = xcalloc((size_t)threads, sizeof(pthread_t));
    workers = xcalloc((size_t)threads, sizeof(t_mt_worker));
    base = height / threads;
    i = 0;
    while (i < threads)
    {
        workers[i].scene = scene;
        workers[i].cam = cam;
        workers[i].y0 = i * base;
        workers[i].y1 = (i == threads - 1) ? height : (i + 1) * base;
        workers[i].aa = aa;
        workers[i].out = out;
        workers[i].width = width;
        pthread_create(&tids[i], NULL, worker, &workers[i]);
        i++;
    }
    i = 0;
    while (i < threads)
    {
        pthread_join(tids[i], NULL);
        i++;
    }
    free(tids);
    free(workers);
}
