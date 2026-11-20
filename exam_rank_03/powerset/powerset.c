/*
** powerset.c - Exam Rank 03, level 2 (bonus)
** Take an integer n followed by a set of integers; print all subsets whose
** sum of elements is n. Elements of a subset keep their input order.
** Allowed functions: atoi, printf, malloc, calloc, realloc, free
*/
#include <stdio.h>
#include <stdlib.h>

static void    backtrack(int *arr, int count, int n, int idx, int sum,
    int *subset, int sub_count)
{
    if (idx == count)
    {
        int    i;

        if (sum == n && sub_count > 0)
        {
            for (i = 0; i < sub_count; i++)
            {
                printf("%d", subset[i]);
                if (i != sub_count - 1)
                    printf(" ");
            }
            printf("\n");
        }
        return;
    }
    /* Exclude arr[idx] */
    backtrack(arr, count, n, idx + 1, sum, subset, sub_count);
    /* Include arr[idx] */
    subset[sub_count] = arr[idx];
    backtrack(arr, count, n, idx + 1, sum + arr[idx], subset, sub_count + 1);
}

int    main(int argc, char **argv)
{
    int    n;
    int    count;
    int    i;
    int    *arr;
    int    *subset;

    if (argc < 2)
        return (1);
    n = atoi(argv[1]);
    count = argc - 2;
    if (count <= 0)
        return (0);

    arr = (int *)malloc((size_t)count * sizeof(int));
    subset = (int *)malloc((size_t)count * sizeof(int));
    if (arr == NULL || subset == NULL)
    {
        free(arr);
        free(subset);
        return (1);
    }
    for (i = 0; i < count; i++)
        arr[i] = atoi(argv[i + 2]);

    backtrack(arr, count, n, 0, 0, subset, 0);

    free(arr);
    free(subset);
    return (0);
}
