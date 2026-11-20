/*
** philo_bonus.c - Main program for the bonus part (processes & semaphores)
** Arguments: number_of_philosophers time_to_die time_to_eat time_to_sleep
**            [number_of_times_each_philosopher_must_eat]
** The main process forks one child per philosopher and acts only as monitor.
*/

#include "philo_bonus.h"

#define SEM_NAME "/philo_forks"

/* Return pointer to the last_meal_time array inside the shared region */
long    *shared_last_meal(t_shared *shared, int nbr)
{
    (void)nbr;
    return ((long *)((char *)shared + sizeof(t_shared)));
}

/* Return pointer to the meals_eaten array inside the shared region */
int        *shared_meals(t_shared *shared, int nbr)
{
    return ((int *)((char *)shared + sizeof(t_shared)
        + sizeof(long) * nbr));
}

/* Print usage error and return 1 */
static int    print_error(void)
{
    write(2, "Error: invalid arguments\n", 25);
    return (1);
}

static int    cleanup(t_shared *shared, int nbr, sem_t *sem, int unlink_sem)
{
    if (sem != NULL)
        sem_close(sem);
    if (unlink_sem)
        sem_unlink(SEM_NAME);
    munmap(shared, sizeof(t_shared) + sizeof(long) * nbr
        + sizeof(int) * nbr);
    return (0);
}

int    main(int argc, char **argv)
{
    t_shared            *shared;
    sem_t                *sem;
    pid_t                *pids;
    long                start;
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

    /* Shared memory: scalars + last_meal_time[] + meals_eaten[] */
    shared = (t_shared *)mmap(NULL, sizeof(t_shared)
        + sizeof(long) * nbr + sizeof(int) * nbr,
        PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (shared == MAP_FAILED)
    {
        write(2, "Error: mmap failed\n", 20);
        return (1);
    }
    memset(shared, 0, sizeof(t_shared) + sizeof(long) * nbr
        + sizeof(int) * nbr);
    shared->nbr_philo = nbr;
    shared->stop = 0;
    shared->time_to_die = ttd;
    shared->time_to_eat = tte;
    shared->time_to_sleep = tts;
    shared->meals_target = meals;
    start = get_time_ms();
    for (i = 0; i < nbr; i++)
        shared_last_meal(shared, nbr)[i] = start;

    /* Fresh named semaphore representing the available forks */
    sem_unlink(SEM_NAME);
    sem = sem_open(SEM_NAME, O_CREAT | O_EXCL, 0666, nbr);
    if (sem == SEM_FAILED)
    {
        write(2, "Error: sem_open failed\n", 23);
        munmap(shared, sizeof(t_shared) + sizeof(long) * nbr
            + sizeof(int) * nbr);
        return (1);
    }

    /* Fork one process per philosopher */
    pids = (pid_t *)malloc(sizeof(pid_t) * nbr);
    if (pids == NULL)
    {
        sem_close(sem);
        sem_unlink(SEM_NAME);
        munmap(shared, sizeof(t_shared) + sizeof(long) * nbr
            + sizeof(int) * nbr);
        return (1);
    }
    i = 0;
    while (i < nbr)
    {
        pids[i] = fork();
        if (pids[i] == -1)
        {
            /* Fork failed: kill already-forked children */
            int    k;

            for (k = 0; k < i; k++)
                kill(pids[k], SIGKILL);
            free(pids);
            cleanup(shared, nbr, sem, 1);
            write(2, "Error: fork failed\n", 20);
            return (1);
        }
        if (pids[i] == 0)
        {
            /* Child process: acts as a philosopher */
            free(pids);
            philosopher_bonus(i, shared, sem, nbr);
            exit(0);
        }
        i++;
    }

    /* Parent: monitor */
    monitor_bonus(shared, pids, nbr, sem);

    free(pids);
    cleanup(shared, nbr, sem, 1);
    return (0);
}
