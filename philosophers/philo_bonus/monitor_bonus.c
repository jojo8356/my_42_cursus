/*
** monitor_bonus.c - Parent (monitor) process
** The main process does not act as a philosopher. It monitors the children:
** - If a philosopher has not started eating within time_to_die ms since its
**   last meal (or simulation start), it dies: log "X died", set stop, and
**   kill every remaining child (some may be blocked in sem_wait and would
**   otherwise never notice the stop flag).
** - If all philosophers have eaten their target number of meals, stop.
** Children are reaped with waitpid(WNOHANG). Checks run every 1 ms so the
** death message is emitted within 10 ms.
*/

#include "philo_bonus.h"

/* Kill every child that has not been reaped yet */
static void    kill_all(pid_t *pids, int *reaped, int nbr)
{
    int    i;

    for (i = 0; i < nbr; i++)
        if (!reaped[i] && pids[i] > 0)
            kill(pids[i], SIGKILL);
}

void    monitor_bonus(t_shared *shared, pid_t *pids, int nbr, sem_t *sem)
{
    long    *last_meal;
    int        *meals;
    int        *reaped;
    int        alive;
    int        i;

    (void)sem;
    last_meal = shared_last_meal(shared, nbr);
    meals = shared_meals(shared, nbr);
    reaped = (int *)malloc(sizeof(int) * nbr);
    if (reaped == NULL)
    {
        for (i = 0; i < nbr; i++)
            if (pids[i] > 0)
                kill(pids[i], SIGKILL);
        exit(1);
    }
    for (i = 0; i < nbr; i++)
        reaped[i] = 0;
    alive = nbr;

    while (alive > 0)
    {
        long    now = get_time_ms();

        /* Starvation check (only while still running, so a death is never
        ** announced twice) */
        if (!shared->stop)
        {
            for (i = 0; i < nbr; i++)
            {
                if (reaped[i])
                    continue;
                if (now - last_meal[i] > shared->time_to_die)
                {
                    philo_log(i + 1, "died");
                    shared->stop = 1;
                    kill_all(pids, reaped, nbr);
                    break;
                }
            }
        }
        /* All-meals-done check */
        if (shared->meals_target > 0 && !shared->stop)
        {
            int    all_done = 1;

            for (i = 0; i < nbr; i++)
                if (meals[i] < shared->meals_target)
                {
                    all_done = 0;
                    break;
                }
            if (all_done)
            {
                shared->stop = 1;
                kill_all(pids, reaped, nbr);
            }
        }
        /* Reap exited children */
        for (i = 0; i < nbr; i++)
        {
            int        status;
            pid_t    r;

            if (reaped[i])
                continue;
            r = waitpid(pids[i], &status, WNOHANG);
            if (r == pids[i] || r == -1)
            {
                reaped[i] = 1;
                alive--;
            }
        }
        usleep(1000);
    }
    free(reaped);
}
