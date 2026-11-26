/* ** coder.c - coder lifecycle: acquire, compile, debug, refactor ** */
#include "coder.h"
#include "utils.h"
#include "log.h"
#include "dongle.h"

void *coder_routine(void *arg)
{
	t_coder *c = (t_coder *)arg;
	t_sim *sim = c->sim;
	t_dongle *d_first;
	t_dongle *d_second;
	t_waiter *w1;
	t_waiter *w2;
	int first;
	int second;
	int done;

	if (c->left < c->right)
	{
		first = c->left;
		second = c->right;
	}
	else
	{
		first = c->right;
		second = c->left;
	}
	d_first = sim->dongles[first];
	d_second = sim->dongles[second];

	while (1)
	{
		if (stop_is_set(sim) != 0)
			break;
		w1 = dongle_acquire(d_first, c, sim);
		if (w1 == NULL)
			break;
		log_event(sim, c->id, "has taken a dongle");
		w2 = dongle_acquire(d_second, c, sim);
		if (w2 == NULL)
		{
			dongle_release(d_first, w1, sim);
			break;
		}
		log_event(sim, c->id, "has taken a dongle");

		pthread_mutex_lock(&c->state_mutex);
		c->compile_count = c->compile_count + 1;
		c->last_compile_start = rel_now_ms(sim);
		done = (c->compile_count >= sim->n_compiles) ? 1 : 0;
		pthread_mutex_unlock(&c->state_mutex);

		log_event(sim, c->id, "is compiling");
		sim_sleep(sim, sim->tc);
		if (stop_is_set(sim) != 0)
		{
			dongle_release(d_first, w1, sim);
			dongle_release(d_second, w2, sim);
			break;
		}
		dongle_release(d_first, w1, sim);
		dongle_release(d_second, w2, sim);
		w1 = NULL;
		w2 = NULL;

		log_event(sim, c->id, "is debugging");
		sim_sleep(sim, sim->td);
		if (stop_is_set(sim) != 0)
			break;
		log_event(sim, c->id, "is refactoring");
		sim_sleep(sim, sim->tr);
		if (stop_is_set(sim) != 0)
			break;
		if (done != 0)
			break;
	}
	return NULL;
}
