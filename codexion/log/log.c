/* ************************************************************************** */
/* ** */
/* ** */
/*   log.c */
/* ** */
/*   By: jojo8356 */
/* ** */
/*   Created: 8 Nov by jojo8356 */
/*   Updated: 8 Nov by jojo8356 */
/* ** */
/* ************************************************************************** */
/* ** log.c - thread-safe logging through a single mutex ** */
#include "log.h"
#include "utils.h"
#include <stdio.h>

void	log_event(t_sim *sim, int id, const char *msg)
{
	pthread_mutex_lock(&sim->log_mutex);
	if (sim->burned_out == 0)
		printf("%lld %d %s\n", rel_now_ms(sim), id, msg);
	pthread_mutex_unlock(&sim->log_mutex);
}
