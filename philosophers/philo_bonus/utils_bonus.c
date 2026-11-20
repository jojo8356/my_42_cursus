/*
** utils_bonus.c - Helper functions for philo_bonus
*/

#include "philo_bonus.h"

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
