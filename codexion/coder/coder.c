/* ************************************************************************** */
/* ** */
/* ** */
/*   coder.c */
/* ** */
/*   By: jojo8356 */
/* ** */
/*   Created: 8 Nov by jojo8356 */
/*   Updated: 8 Nov by jojo8356 */
/* ** */
/* ************************************************************************** */
/* ** coder.c - coder lifecycle: acquire, compile, debug, refactor ** */
#include "coder.h"
#include "utils.h"
#include "log.h"
#include "dongle.h"

static int	coder_compile(t_coder *c, t_sim *sim)
{
	int	done;

	pthread_mutex_lock(&c->state_mutex);
	c->compile_count = c->compile_count + 1;
	c->last_compile_start = rel_now_ms(sim);
	if (c->compile_count >= sim->n_compiles)
		done = 1;
	else
		done = 0;
	pthread_mutex_unlock(&c->state_mutex);
	log_event(sim, c->id, "is compiling");
	sim_sleep(sim, sim->tc);
	return (done);
}

static t_waiter	*coder_take(t_coder *c, t_sim *sim, t_dongle *d)
{
	t_waiter	*w;

	w = dongle_acquire(d, c, sim);
	if (w == NULL)
		return (NULL);
	log_event(sim, c->id, "has taken a dongle");
	return (w);
}

static void	release_pair(t_dongle *d1, t_waiter *w1, t_dongle *d2, t_waiter *w2)
{
	t_sim	*sim;

	sim = w1->coder->sim;
	dongle_release(d1, w1, sim);
	dongle_release(d2, w2, sim);
}

static int	coder_iterate(t_coder *c, t_sim *sim, t_dongle *d1, t_dongle *d2)
{
	t_waiter	*w1;
	t_waiter	*w2;

	if (stop_is_set(sim) != 0)
		return (-1);
	w1 = coder_take(c, sim, d1);
	if (w1 == NULL)
		return (-1);
	w2 = coder_take(c, sim, d2);
	if (w2 == NULL)
	{
		dongle_release(d1, w1, sim);
		return (-1);
	}
	coder_compile(c, sim);
	release_pair(d1, w1, d2, w2);
	if (stop_is_set(sim) != 0)
		return (-1);
	log_event(sim, c->id, "is debugging");
	sim_sleep(sim, sim->td);
	if (stop_is_set(sim) != 0)
		return (-1);
	log_event(sim, c->id, "is refactoring");
	sim_sleep(sim, sim->tr);
	return (0);
}

void	*coder_routine(void *arg)
{
	t_coder		*c;
	t_sim		*sim;
	t_dongle	*d_first;
	t_dongle	*d_second;

	c = (t_coder *)arg;
	sim = c->sim;
	if (c->left < c->right)
	{
		d_first = sim->dongles[c->left];
		d_second = sim->dongles[c->right];
	}
	else
	{
		d_first = sim->dongles[c->right];
		d_second = sim->dongles[c->left];
	}
	while (1)
	{
		if (coder_iterate(c, sim, d_first, d_second) < 0)
			break ;
		if (c->compile_count >= sim->n_compiles)
			break ;
	}
	return (NULL);
}
