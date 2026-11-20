/*
** philo.h - Philosophers (mandatory part: threads & mutexes)
** 42 School project - Version 12.0 subject
*/

#ifndef PHILO_H
#define PHILO_H

#include <pthread.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

typedef struct s_data
{
    int                nbr_philo;
    int                stop;
    pthread_mutex_t    m_stop;
    pthread_mutex_t    *forks;
    struct s_philo     *philos;
    long               time_to_die;
    long               time_to_eat;
    long               time_to_sleep;
    int                meals_target;
    int                completed_philos;
    pthread_mutex_t    m_completed;
}                       t_data;

typedef struct s_philo
{
    int                id;
    pthread_mutex_t    *fork_first;
    pthread_mutex_t    *fork_second;
    pthread_mutex_t    m_meal;
    long               last_meal_time;
    int                meals_eaten;
    int                meals_target;
    int                target_reached;
    t_data             *data;
}                       t_philo;

/* utils.c */
long        get_time_ms(void);
void        philo_log(int id, const char *msg);
int         is_stopped(t_data *data);
void        set_stop(t_data *data);
int         increment_meals(t_philo *philo);
long        ft_atoi(const char *str);

/* philosopher.c */
void        *philosopher(void *arg);

/* monitor.c */
void        *monitor(void *arg);

/* philo.c */
int         init_data(t_data *data, int nbr, long ttd, long tte, long tts, int meals);
void        cleanup_data(t_data *data);
int         main(int argc, char **argv);

#endif /* PHILO_H */
