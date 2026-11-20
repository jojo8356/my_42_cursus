/*
** n_queens.c - Exam Rank 03, level 2 (bonus)
** Print all solutions to the N-Queens problem.
** Format: <p1> <p2> ... <pn>\n where pn is the row index (0-based) of the
** queen in column n.
** Allowed functions: atoi, fprintf, write
*/
#include <stdio.h>
#include <stdlib.h>

/* Return 1 if placing a queen at (col, row) conflicts with placed queens. */
static int    conflict(int *queens, int col, int row)
{
    int    i;
    int    d;

    for (i = 0; i < col; i++)
    {
        if (queens[i] == row)
            return (1);
        d = queens[i] - row;
        if (d < 0)
            d = -d;
        if (d == col - i)
            return (1);
    }
    return (0);
}

static void    solve(int *queens, int n, int col)
{
    int    row;

    if (col == n)
    {
        int    i;

        for (i = 0; i < n; i++)
        {
            fprintf(stdout, "%d", queens[i]);
            if (i != n - 1)
                fprintf(stdout, " ");
        }
        fprintf(stdout, "\n");
        return;
    }
    for (row = 0; row < n; row++)
    {
        if (!conflict(queens, col, row))
        {
            queens[col] = row;
            solve(queens, n, col + 1);
        }
    }
}

int    main(int argc, char **argv)
{
    int    n;
    int    queens[32];
    int    i;

    if (argc != 2)
        return (1);
    n = atoi(argv[1]);
    if (n <= 0)
        return (0);
    if (n > 32)
        return (1);
    for (i = 0; i < n; i++)
        queens[i] = 0;
    solve(queens, n, 0);
    return (0);
}
