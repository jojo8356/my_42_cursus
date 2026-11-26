/* ** log.c - thread-safe logging through a single mutex ** */
#include "log.h"
#include "utils.h"
#include <stdio.h>

void log_event(t_sim *sim, int id, const char *msg)
{
	pthread_mutex_lock(&sim->log_mutex);
	if (sim->burned_out != 0)
	{
		pthread_mutex_unlock(&sim->log_mutex);
		return;
	}
	printf("%lld %d %s\n", rel_now_ms(sim), id, msg);
	pthread_mutex_unlock(&sim->log_mutex);
}
