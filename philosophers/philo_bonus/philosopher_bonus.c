/*
** philosopher_bonus.c - Philosopher child process
** Each philosopher is a separate process. Forks are in the middle of the
** table, represented by a semaphore. The child takes forks with sem_wait,
** eats, releases them with sem_post, then sleeps, and repeats.
*/

#include "philo_bonus.h"

void    philosopher_bonus(int id, t_shared *shared, sem_t *sem, int nbr)
{
    long    *last_meal;
    int        *meals;
    int        forks_needed;
    int        taken;

    last_meal = shared_last_meal(shared, nbr);
    meals = shared_meals(shared, nbr);
    forks_needed = (nbr == 1) ? 1 : 2;
    taken = 0;

    while (1)
    {
        if (shared->stop)
            break;
        /* Think */
        philo_log(id + 1, "is thinking");
        /* Take forks */
        taken = 0;
        for (int j = 0; j < forks_needed; j++)
        {
            sem_wait(sem);
            if (shared->stop)
            {
                while (taken > 0)
                {
                    sem_post(sem);
                    taken--;
                }
                break;
            }
            philo_log(id + 1, "has taken a fork");
            taken++;
        }
        if (shared->stop)
            break;
        /* Eating: reset the death timer, count the meal */
        last_meal[id] = get_time_ms();
        meals[id]++;
        philo_log(id + 1, "is eating");
        usleep((useconds_t)(shared->time_to_eat * 1000));
        /* Release forks */
        while (taken > 0)
        {
            sem_post(sem);
            taken--;
        }
        if (shared->stop)
            break;
        /* Sleep */
        philo_log(id + 1, "is sleeping");
        usleep((useconds_t)(shared->time_to_sleep * 1000));
    }
    exit(0);
}
