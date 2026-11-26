/* ** monitor.c - detects burnout or completion, wakes waiters, stops ** */
#include "monitor.h"
#include "utils.h"
#include "log.h"
#include <unistd.h>
#include <stdio.h>

void set_stop(t_sim *sim)
{
	int i;

	pthread_mutex_lock(&sim->stop_mutex);
	sim->stop = 1;
	pthread_mutex_unlock(&sim->stop_mutex);
	for (i = 0; i < sim->n_coders; i++)
		pthread_cond_broadcast(&sim->dongles[i]->cond);
	pthread_cond_broadcast(&sim->stop_cond);
}

int all_done(t_sim *sim)
{
	int i;
	int d;

	for (i = 0; i < sim->n_coders; i++)
	{
		pthread_mutex_lock(&sim->coders[i]->state_mutex);
		d = (sim->coders[i]->compile_count >= sim->n_compiles) ? 1 : 0;
		pthread_mutex_unlock(&sim->coders[i]->state_mutex);
		if (d == 0)
			return 0;
	}
	return 1;
}

void *monitor_routine(void *arg)
{
	t_sim *sim = (t_sim *)arg;
	t_coder *c;
	long long now;
	long long dl;
	int i;
	int done;
	int burned;

	while (1)
	{
		if (stop_is_set(sim) != 0)
			return NULL;
		if (all_done(sim) != 0)
		{
			set_stop(sim);
			return NULL;
		}
		for (i = 0; i < sim->n_coders; i++)
		{
			c = sim->coders[i];
			pthread_mutex_lock(&c->state_mutex);
			done = (c->compile_count >= sim->n_compiles) ? 1 : 0;
			now = rel_now_ms(sim);
			dl = c->last_compile_start + sim->tb;
			pthread_mutex_unlock(&c->state_mutex);
			burned = (done == 0 && now > dl) ? 1 : 0;
			if (burned != 0)
			{
				set_stop(sim);
				pthread_mutex_lock(&sim->log_mutex);
				sim->burned_out = 1;
				printf("%lld %d %s\n", rel_now_ms(sim), c->id, "burned out");
				pthread_mutex_unlock(&sim->log_mutex);
				return NULL;
			}
		}
		for (i = 0; i < sim->n_coders; i++)
		{
			t_dongle *d = sim->dongles[i];

			pthread_mutex_lock(&d->mutex);
			if (d->heap_size > 0)
				pthread_cond_broadcast(&d->cond);
			pthread_mutex_unlock(&d->mutex);
		}
		usleep(1000);
	}
	return NULL;
}
