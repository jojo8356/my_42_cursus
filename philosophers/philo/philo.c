/*
** philo.c - Main program for the mandatory part (threads & mutexes)
** Arguments: number_of_philosophers time_to_die time_to_eat time_to_sleep
**            [number_of_times_each_philosopher_must_eat]
*/

#include "philo.h"

/* Initialize all data structures and mutexes */
int    init_data(t_data *data, int nbr, long ttd, long tte, long tts, int meals)
{
    int                i;

    data->nbr_philo = nbr;
    data->time_to_die = ttd;
    data->time_to_eat = tte;
    data->time_to_sleep = tts;
    data->meals_target = meals;
    data->stop = 0;
    data->completed_philos = 0;

    if (pthread_mutex_init(&data->m_stop, NULL) != 0)
        return (0);
    if (pthread_mutex_init(&data->m_completed, NULL) != 0)
        return (0);

    data->forks = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t) * nbr);
    if (data->forks == NULL)
        return (0);
    i = 0;
    while (i < nbr)
    {
        if (pthread_mutex_init(&data->forks[i], NULL) != 0)
            return (0);
        i++;
    }

    data->philos = (t_philo *)malloc(sizeof(t_philo) * nbr);
    if (data->philos == NULL)
        return (0);
    i = 0;
    while (i < nbr)
    {
        data->philos[i].id = i + 1;
        {
            int    left = i;
            int    right = (i + 1) % nbr;
            int    first = left < right ? left : right;
            int    second = left < right ? right : left;
            data->philos[i].fork_first = &data->forks[first];
            data->philos[i].fork_second = &data->forks[second];
        }
        if (pthread_mutex_init(&data->philos[i].m_meal, NULL) != 0)
            return (0);
        data->philos[i].last_meal_time = get_time_ms();
        data->philos[i].meals_eaten = 0;
        data->philos[i].meals_target = meals;
        data->philos[i].target_reached = 0;
        data->philos[i].data = data;
        i++;
    }
    return (1);
}

/* Destroy all mutexes and free allocated memory */
void    cleanup_data(t_data *data)
{
    int    i;

    if (data->forks != NULL)
    {
        i = 0;
        while (i < data->nbr_philo)
        {
            pthread_mutex_destroy(&data->forks[i]);
            i++;
        }
        free(data->forks);
    }
    if (data->philos != NULL)
    {
        i = 0;
        while (i < data->nbr_philo)
        {
            pthread_mutex_destroy(&data->philos[i].m_meal);
            i++;
        }
        free(data->philos);
    }
    pthread_mutex_destroy(&data->m_stop);
    pthread_mutex_destroy(&data->m_completed);
}

/* Print usage error and return 1 */
static int    print_error(void)
{
    write(2, "Error: invalid arguments\n", 25);
    return (1);
}

int    main(int argc, char **argv)
{
    t_data            data;
    pthread_t        monitor_thread;
    pthread_t        *threads;
    int                nbr;
    long            ttd;
    long            tte;
    long            tts;
    int                meals;
    int                i;

    setvbuf(stdout, NULL, _IOLBF, 0);
    if (argc < 5 || argc > 6)
        return (print_error());
    nbr = (int)ft_atoi(argv[1]);
    ttd = ft_atoi(argv[2]);
    tte = ft_atoi(argv[3]);
    tts = ft_atoi(argv[4]);
    meals = -1;
    if (argc == 6)
        meals = (int)ft_atoi(argv[5]);
    if (nbr <= 0 || ttd <= 0 || tte <= 0 || tts <= 0
        || (argc == 6 && meals <= 0))
        return (print_error());

    if (!init_data(&data, nbr, ttd, tte, tts, meals))
    {
        write(2, "Error: memory allocation failed\n", 32);
        return (1);
    }

    if (pthread_create(&monitor_thread, NULL, monitor, &data) != 0)
    {
        cleanup_data(&data);
        return (1);
    }

    threads = (pthread_t *)malloc(sizeof(pthread_t) * nbr);
    if (threads == NULL)
    {
        cleanup_data(&data);
        return (1);
    }
    i = 0;
    while (i < nbr)
    {
        if (pthread_create(&threads[i], NULL, philosopher,
            &data.philos[i]) != 0)
        {
            free(threads);
            cleanup_data(&data);
            return (1);
        }
        i++;
    }

    i = 0;
    while (i < nbr)
    {
        pthread_join(threads[i], NULL);
        i++;
    }
    pthread_join(monitor_thread, NULL);
    free(threads);
    cleanup_data(&data);
    return (0);
}
