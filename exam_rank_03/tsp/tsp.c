/*
** tsp.c - Exam Rank 03, level 2 (bonus)
** Read city coordinates as '%f, %f\n' from stdin and print the length of the
** shortest closed tour visiting all cities, formatted as '%.2f'.
** At most 11 cities are tested.
** Allowed functions: write, sqrtf, getline, fseek, fscanf, ferror, feof,
**                    fabsf, memcpy, fprintf, fclose, malloc, calloc, realloc,
**                    free, fopen, errno, stderr, stdin, stdout
** Compile with -lm.
*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define MAX 12

/* Lexicographic next_permutation for an array of ints.
** Return 1 if a next permutation exists, 0 otherwise. */
static int    next_permutation(int *a, int n)
{
    int    i;
    int    j;
    int    tmp;

    i = n - 2;
    while (i >= 0 && a[i] >= a[i + 1])
        i--;
    if (i < 0)
        return (0);
    j = n - 1;
    while (a[j] <= a[i])
        j--;
    tmp = a[i];
    a[i] = a[j];
    a[j] = tmp;
    i = i + 1;
    j = n - 1;
    while (i < j)
    {
        tmp = a[i];
        a[i] = a[j];
        a[j] = tmp;
        i++;
        j--;
    }
    return (1);
}

int    main(void)
{
    float    x[MAX];
    float    y[MAX];
    float    dist[MAX][MAX];
    int        perm[MAX];
    int        n;
    float    min_len;
    float    len;
    int        i;
    int        j;

    n = 0;
    while (n < MAX && fscanf(stdin, "%f, %f", &x[n], &y[n]) == 2)
        n++;

    if (n == 0)
    {
        fprintf(stdout, "0.00\n");
        return (0);
    }
    if (n == 1)
    {
        fprintf(stdout, "0.00\n");
        return (0);
    }

    /* Precompute the distance matrix */
    for (i = 0; i < n; i++)
    {
        for (j = 0; j < n; j++)
        {
            float    dx;
            float    dy;

            dx = x[i] - x[j];
            dy = y[i] - y[j];
            dist[i][j] = sqrtf(dx * dx + dy * dy);
        }
    }

    /* Fix city 0 as the start; permute cities 1..n-1 */
    for (i = 1; i < n; i++)
        perm[i] = i;

    min_len = -1.0f;
    do
    {
        len = dist[0][perm[1]];
        for (i = 1; i < n - 1; i++)
            len += dist[perm[i]][perm[i + 1]];
        len += dist[perm[n - 1]][0];
        if (min_len < 0.0f || len < min_len)
            min_len = len;
    } while (next_permutation(perm + 1, n - 1));

    fprintf(stdout, "%.2f\n", min_len);
    return (0);
}
