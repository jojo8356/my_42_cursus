/*
** monitor.c - Monitor thread
** Periodically checks every philosopher. If a philosopher has not started
** eating within time_to_die milliseconds since the start of its last meal (or
** the start of the simulation), it dies. Checks every 1 ms so the death
** message is emitted within 10 ms.
*/

#include "philo.h"

void    *monitor(void *arg)
{
    t_data            *data;
    long            now;
    int                i;

    data = (t_data *)arg;
    while (1)
    {
        if (is_stopped(data))
            break;
        now = get_time_ms();
        i = 0;
        while (i < data->nbr_philo)
        {
            t_philo    *philo = &data->philos[i];

            pthread_mutex_lock(&philo->m_meal);
            if (now - philo->last_meal_time > data->time_to_die)
            {
                pthread_mutex_unlock(&philo->m_meal);
                philo_log(philo->id, "died");
                set_stop(data);
                break;
            }
            pthread_mutex_unlock(&philo->m_meal);
            i++;
        }
        if (is_stopped(data))
            break;
        usleep(1000);
    }
    return (NULL);
}
