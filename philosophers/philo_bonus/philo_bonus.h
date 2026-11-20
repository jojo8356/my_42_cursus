/*
** philo_bonus.h - Philosophers (bonus part: processes & semaphores)
** 42 School project - Version 12.0 subject
** All forks are in the middle of the table; the number of available forks
** is represented by a named semaphore. One process per philosopher; the
** main process is the monitor and does not eat.
*/

#ifndef PHILO_BONUS_H
#define PHILO_BONUS_H

#include <fcntl.h>
#include <semaphore.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <unistd.h>

/* Shared memory layout: t_shared scalars, then long last_meal_time[nbr],
** then int meals_eaten[nbr]. All processes map the same region. */
typedef struct s_shared
{
    int                nbr_philo;
    int                stop;
    long               time_to_die;
    long               time_to_eat;
    long               time_to_sleep;
    int                meals_target;
}                       t_shared;

/* utils_bonus.c */
long        get_time_ms(void);
void        philo_log(int id, const char *msg);
long        ft_atoi(const char *str);

/* shared memory helpers (philo_bonus.c) */
long        *shared_last_meal(t_shared *shared, int nbr);
int         *shared_meals(t_shared *shared, int nbr);

/* philosopher_bonus.c - child process */
void        philosopher_bonus(int id, t_shared *shared, sem_t *sem, int nbr);

/* monitor_bonus.c - parent process */
void        monitor_bonus(t_shared *shared, pid_t *pids, int nbr, sem_t *sem);

/* philo_bonus.c */
int         main(int argc, char **argv);

#endif /* PHILO_BONUS_H */
