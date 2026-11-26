/* ** dongle.c - dongle lifecycle: acquire (queued, cooldown) / release ** */
#include "dongle.h"
#include "heap.h"
#include "utils.h"
#include <stdlib.h>

void dongle_init(t_dongle *d, int cap)
{
	d->in_use = 0;
	d->released_at = 0;
	d->heap_cap = cap;
	d->heap_size = 0;
	d->seq = 0;
	d->heap = malloc(sizeof(t_waiter *) * (size_t)cap);
	pthread_mutex_init(&d->mutex, NULL);
	pthread_cond_init(&d->cond, NULL);
}

t_waiter *dongle_acquire(t_dongle *d, t_coder *c, t_sim *sim)
{
	t_waiter *w;
	long long deadline;

	d->seq = d->seq + 1;
	w = malloc(sizeof(t_waiter));
	if (w == NULL)
		return NULL;
	w->id = c->id;
	w->coder = c;
	w->seq = d->seq;
	pthread_mutex_lock(&c->state_mutex);
	deadline = c->last_compile_start + sim->tb;
	pthread_mutex_unlock(&c->state_mutex);
	if (sim->edf != 0)
		w->priority = deadline;
	else
		w->priority = d->seq;

	pthread_mutex_lock(&d->mutex);
	heap_push(d, w);
	while (1)
	{
		if (stop_is_set(sim) != 0)
		{
			heap_remove(d, w);
			pthread_mutex_unlock(&d->mutex);
			free(w);
			return NULL;
		}
		if (d->in_use == 0 && now_ms() >= d->released_at)
		{
			if (heap_peek(d) == w)
			{
				heap_pop(d);
				d->in_use = 1;
				pthread_mutex_unlock(&d->mutex);
				return w;
			}
		}
		pthread_cond_wait(&d->cond, &d->mutex);
	}
}

void dongle_release(t_dongle *d, t_waiter *w, t_sim *sim)
{
	pthread_mutex_lock(&d->mutex);
	d->in_use = 0;
	d->released_at = now_ms() + sim->cd;
	pthread_cond_broadcast(&d->cond);
	pthread_mutex_unlock(&d->mutex);
	free(w);
}
