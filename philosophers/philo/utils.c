/*
** utils.c - Helper functions for philo
*/

#include "philo.h"

/* Return current time in milliseconds */
long    get_time_ms(void)
{
    struct timeval    tv;

    gettimeofday(&tv, NULL);
    return ((tv.tv_sec * 1000) + (tv.tv_usec / 1000));
}

/* Print a state change message: "timestamp_in_ms X message" */
void    philo_log(int id, const char *msg)
{
    long    now;

    now = get_time_ms();
    printf("%ld %d %s\n", now, id, msg);
}

/* Check if the simulation must stop (thread-safe) */
int     is_stopped(t_data *data)
{
    int    stopped;

    pthread_mutex_lock(&data->m_stop);
    stopped = data->stop;
    pthread_mutex_unlock(&data->m_stop);
    return (stopped);
}

/* Set the stop flag (thread-safe) */
void    set_stop(t_data *data)
{
    pthread_mutex_lock(&data->m_stop);
    data->stop = 1;
    pthread_mutex_unlock(&data->m_stop);
}

/* Increment meals eaten; when a philosopher reaches its target,
** increment the shared completion counter and stop when all are done */
int    increment_meals(t_philo *philo)
{
    int                reached_target;
    t_data            *data;

    reached_target = 0;
    data = philo->data;
    pthread_mutex_lock(&philo->m_meal);
    philo->meals_eaten++;
    if (philo->meals_target > 0 && !philo->target_reached
        && philo->meals_eaten >= philo->meals_target)
    {
        philo->target_reached = 1;
        reached_target = 1;
    }
    pthread_mutex_unlock(&philo->m_meal);
    if (reached_target)
    {
        pthread_mutex_lock(&data->m_completed);
        data->completed_philos++;
        if (data->completed_philos >= data->nbr_philo)
            set_stop(data);
        pthread_mutex_unlock(&data->m_completed);
    }
    return (reached_target);
}

/* Convert a string of digits to a long. Returns -1 on invalid input.
** Uses only array indexing and arithmetic (no libc parsing functions). */
long    ft_atoi(const char *str)
{
    long    result;
    int        i;

    if (str == NULL)
        return (-1);
    i = 0;
    result = 0;
    if (str[i] == '+')
        i++;
    else if (str[i] == '-')
        return (-1);
    while (str[i] >= '0' && str[i] <= '9')
    {
        result = result * 10 + (str[i] - '0');
        i++;
    }
    if (str[i] != '\0')
        return (-1);
    return (result);
}
