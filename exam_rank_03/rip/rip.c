/*
** rip.c - Exam Rank 03, level 2 (bonus)
** Given a string of parentheses, remove the minimum number of parentheses
** (replace them with spaces) so the expression becomes balanced. Print all
** solutions.
** Allowed functions: puts, write
*/
#include <stdio.h>

/* Compute the minimum number of removals needed to balance the parentheses. */
static int    min_removals(const char *s)
{
    int    balance;
    int    min_right;
    int    i;

    balance = 0;
    min_right = 0;
    for (i = 0; s[i] != '\0'; i++)
    {
        if (s[i] == '(')
            balance++;
        else if (s[i] == ')')
        {
            if (balance > 0)
                balance--;
            else
                min_right++;
        }
    }
    return (min_right + balance);
}

/* Depth-first search over the string. At each parenthesis we either keep it
** or remove it (replace with space). We maintain the running balance of kept
** parentheses and prune invalid branches (balance would go negative). */
static void    dfs(char *s, int pos, int balance, int removals, int min_rem)
{
    if (s[pos] == '\0')
    {
        if (balance == 0 && removals == min_rem)
            puts(s);
        return;
    }
    if (s[pos] == '(')
    {
        /* Keep */
        dfs(s, pos + 1, balance + 1, removals, min_rem);
        /* Remove */
        if (removals < min_rem)
        {
            s[pos] = ' ';
            dfs(s, pos + 1, balance, removals + 1, min_rem);
            s[pos] = '(';
        }
    }
    else if (s[pos] == ')')
    {
        /* Keep only if it does not make the balance negative */
        if (balance > 0)
            dfs(s, pos + 1, balance - 1, removals, min_rem);
        /* Remove */
        if (removals < min_rem)
        {
            s[pos] = ' ';
            dfs(s, pos + 1, balance, removals + 1, min_rem);
            s[pos] = ')';
        }
    }
    else
    {
        /* Space or other character: always keep */
        dfs(s, pos + 1, balance, removals, min_rem);
    }
}

int    main(int argc, char **argv)
{
    int    min_rem;

    if (argc != 2)
        return (1);
    min_rem = min_removals(argv[1]);
    dfs(argv[1], 0, 0, 0, min_rem);
    return (0);
}
