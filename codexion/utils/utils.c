/* ************************************************************************** */
/* ** */
/* ** */
/*   utils.c */
/* ** */
/*   By: jojo8356 */
/* ** */
/*   Created: 8 Nov by jojo8356 */
/*   Updated: 8 Nov by jojo8356 */
/* ** */
/* ************************************************************************** */
/* ** utils.c - clock, interruptible sleep, strict parsing, stop flag ** */
#include "utils.h"
#include <time.h>
#include <unistd.h>

long long	now_ms(void)
{
	struct timespec	ts;
	long long		ms;

	clock_gettime(CLOCK_MONOTONIC, &ts);
	ms = (long long)ts.tv_sec * 1000 + (long long)ts.tv_nsec / 1000000;
	return (ms);
}

long long	rel_now_ms(t_sim *sim)
{
	return (now_ms() - sim->start_ms);
}

void	sim_sleep(t_sim *sim, long ms)
{
	long	elapsed;

	elapsed = 0;
	while (elapsed < ms && stop_is_set(sim) == 0)
	{
		usleep(1000);
		elapsed++;
	}
}

int	parse_strict_int(const char *str, long *out)
{
	const char	*p;
	long long	val;

	val = 0;
	if (str == NULL || *str == '\0')
		return (0);
	p = str;
	while (*p != '\0')
	{
		if (*p < '0' || *p > '9')
			return (0);
		p++;
	}
	p = str;
	while (*p != '\0')
	{
		val = val * 10 + (long long)(*p - '0');
		if (val > 2147483647LL)
			return (0);
		p++;
	}
	*out = (long)val;
	return (1);
}

int	stop_is_set(t_sim *sim)
{
	int	r;

	pthread_mutex_lock(&sim->stop_mutex);
	r = sim->stop;
	pthread_mutex_unlock(&sim->stop_mutex);
	return (r);
}
