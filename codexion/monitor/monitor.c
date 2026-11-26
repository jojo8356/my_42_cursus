/* ************************************************************************** */
/* ** */
/* ** */
/*   monitor.c */
/* ** */
/*   By: jojo8356 */
/* ** */
/*   Created: 8 Nov by jojo8356 */
/*   Updated: 8 Nov by jojo8356 */
/* ** */
/* ************************************************************************** */
/* ** monitor.c - detects burnout or completion, wakes waiters, stops ** */
#include "monitor.h"
#include "utils.h"
#include "log.h"
#include <unistd.h>
#include <stdio.h>

void	set_stop(t_sim *sim)
{
	int	i;

	pthread_mutex_lock(&sim->stop_mutex);
	sim->stop = 1;
	pthread_mutex_unlock(&sim->stop_mutex);
	i = 0;
	while (i < sim->n_coders)
	{
		pthread_cond_broadcast(&sim->dongles[i]->cond);
		i++;
	}
	pthread_cond_broadcast(&sim->stop_cond);
}

int	all_done(t_sim *sim)
{
	int	i;
	int	d;

	i = 0;
	while (i < sim->n_coders)
	{
		pthread_mutex_lock(&sim->coders[i]->state_mutex);
		if (sim->coders[i]->compile_count >= sim->n_compiles)
			d = 1;
		else
			d = 0;
		pthread_mutex_unlock(&sim->coders[i]->state_mutex);
		if (d == 0)
			return (0);
		i++;
	}
	return (1);
}

static int	monitor_check_burnout(t_sim *sim)
{
	t_coder		*c;
	long long	dl;
	int			i;

	i = 0;
	while (i < sim->n_coders)
	{
		c = sim->coders[i];
		pthread_mutex_lock(&c->state_mutex);
		dl = c->last_compile_start + sim->tb;
		if (c->compile_count < sim->n_compiles && rel_now_ms(sim) > dl)
		{
			pthread_mutex_unlock(&c->state_mutex);
			set_stop(sim);
			pthread_mutex_lock(&sim->log_mutex);
			sim->burned_out = 1;
			printf("%lld %d %s\n", rel_now_ms(sim), c->id, "burned out");
			pthread_mutex_unlock(&sim->log_mutex);
			return (1);
		}
		pthread_mutex_unlock(&c->state_mutex);
		i++;
	}
	return (0);
}

static void	monitor_wake_waiters(t_sim *sim)
{
	int			i;
	t_dongle	*d;

	i = 0;
	while (i < sim->n_coders)
	{
		d = sim->dongles[i];
		pthread_mutex_lock(&d->mutex);
		if (d->heap_size > 0)
			pthread_cond_broadcast(&d->cond);
		pthread_mutex_unlock(&d->mutex);
		i++;
	}
}

void	*monitor_routine(void *arg)
{
	t_sim	*sim;

	sim = (t_sim *)arg;
	while (1)
	{
		if (stop_is_set(sim) != 0)
			return (NULL);
		if (all_done(sim) != 0)
		{
			set_stop(sim);
			return (NULL);
		}
		if (monitor_check_burnout(sim) != 0)
			return (NULL);
		monitor_wake_waiters(sim);
		usleep(1000);
	}
	return (NULL);
}
