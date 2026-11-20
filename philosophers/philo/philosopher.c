/*
** philosopher.c - Philosopher thread routine
** Cycle: think (log) -> take forks -> eat (update death timer) -> sleep.
** Forks are taken in a fixed order (lower index first) to avoid deadlock.
** The death timer is measured from the start of the last meal (or simulation
** start), per the subject.
*/

#include "philo.h"

/* Take both forks. Returns 1 on success, 0 if stopped while taking */
static int    take_forks(t_philo *philo)
{
    pthread_mutex_lock(philo->fork_first);
    /* Check stop before logging, so a dead philosopher never logs again */
    if (is_stopped(philo->data))
    {
        pthread_mutex_unlock(philo->fork_first);
        return (0);
    }
    philo_log(philo->id, "has taken a fork");
    if (philo->fork_first != philo->fork_second)
    {
        pthread_mutex_lock(philo->fork_second);
        if (is_stopped(philo->data))
        {
            pthread_mutex_unlock(philo->fork_second);
            pthread_mutex_unlock(philo->fork_first);
            return (0);
        }
        philo_log(philo->id, "has taken a fork");
    }
    return (1);
}

/* Release both forks */
static void    release_forks(t_philo *philo)
{
    if (philo->fork_first != philo->fork_second)
        pthread_mutex_unlock(philo->fork_second);
    pthread_mutex_unlock(philo->fork_first);
}

void    *philosopher(void *arg)
{
    t_philo            *philo;
    t_data            *data;

    philo = (t_philo *)arg;
    data = philo->data;
    while (1)
    {
        if (is_stopped(data))
            break;
        /* Think: announce then try to take forks */
        philo_log(philo->id, "is thinking");
        if (!take_forks(philo))
            break;
        /* Eating: reset the death timer */
        pthread_mutex_lock(&philo->m_meal);
        philo->last_meal_time = get_time_ms();
        pthread_mutex_unlock(&philo->m_meal);
        philo_log(philo->id, "is eating");
        usleep((useconds_t)(data->time_to_eat * 1000));
        release_forks(philo);
        if (data->meals_target > 0)
            increment_meals(philo);
        if (is_stopped(data))
            break;
        /* Sleep */
        philo_log(philo->id, "is sleeping");
        usleep((useconds_t)(data->time_to_sleep * 1000));
    }
    return (NULL);
}
