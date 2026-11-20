/*
** permutations.c - Exam Rank 03, level 2 (bonus)
** Print all permutations of a string in alphabetical order.
** Allowed functions: puts, malloc, calloc, realloc, free, write
*/
#include <stdio.h>
#include <stdlib.h>

/* Manual string length (strlen is not in the allowed functions). */
static int    strlen_m(const char *s)
{
    int    i;

    i = 0;
    while (s[i] != '\0')
        i++;
    return (i);
}

/* Selection sort to sort the string alphabetically. */
static void    sort_str(char *s, int n)
{
    int    i;
    int    j;
    int    k;
    char    tmp;

    for (i = 0; i < n - 1; i++)
    {
        k = i;
        for (j = i + 1; j < n; j++)
        {
            if (s[j] < s[k])
                k = j;
        }
        if (k != i)
        {
            tmp = s[i];
            s[i] = s[k];
            s[k] = tmp;
        }
    }
}

static void    permute(const char *s, int n, char *out, int *used, int depth)
{
    int    i;

    if (depth == n)
    {
        out[depth] = '\0';
        puts(out);
        return;
    }
    for (i = 0; i < n; i++)
    {
        if (used[i])
            continue;
        used[i] = 1;
        out[depth] = s[i];
        permute(s, n, out, used, depth + 1);
        used[i] = 0;
    }
}

int    main(int argc, char **argv)
{
    char    *s;
    int    n;
    int    i;
    char    *out;
    int    *used;

    if (argc != 2)
        return (1);
    n = strlen_m(argv[1]);
    s = (char *)malloc((size_t)n + 1);
    if (s == NULL)
        return (1);
    i = 0;
    while (argv[1][i] != '\0')
    {
        s[i] = argv[1][i];
        i++;
    }
    s[i] = '\0';
    sort_str(s, n);

    out = (char *)malloc((size_t)n + 1);
    used = (int *)calloc((size_t)n, sizeof(int));
    if (out == NULL || used == NULL)
    {
        free(s);
        free(out);
        free(used);
        return (1);
    }
    permute(s, n, out, used, 0);
    free(s);
    free(out);
    free(used);
    return (0);
}
