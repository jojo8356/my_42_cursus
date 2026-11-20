/*
** filter.c - Exam Rank 03, level 1
** Read stdin and replace every occurrence of the argument string s with
** '*' characters (one '*' per character of s).
** Allowed functions: read, strlen, malloc, calloc, realloc, free, printf, perror
*/
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int    main(int argc, char **argv)
{
    char    *s;
    char    *buf;
    size_t    slen;
    size_t    bufsize;
    size_t    buflen;
    size_t    i;
    size_t    j;
    int        c;

    if (argc != 2 || argv[1][0] == '\0')
        return (1);
    s = argv[1];
    slen = strlen(s);

    bufsize = 4096;
    buf = (char *)malloc(bufsize);
    if (buf == NULL)
    {
        perror("Error");
        return (1);
    }
    buflen = 0;

    /* Read the whole stdin */
    while (1)
    {
        if (buflen == bufsize)
        {
            char    *tmp;

            tmp = (char *)realloc(buf, bufsize * 2);
            if (tmp == NULL)
            {
                perror("Error");
                free(buf);
                return (1);
            }
            buf = tmp;
            bufsize *= 2;
        }
        c = (int)read(0, buf + buflen, bufsize - buflen);
        if (c < 0)
        {
            perror("Error");
            free(buf);
            return (1);
        }
        if (c == 0)
            break;
        buflen += (size_t)c;
    }

    /* Replace every occurrence of s with '*' (one per char of s) */
    i = 0;
    while (i + slen <= buflen)
    {
        int        match;

        match = 1;
        j = 0;
        while (j < slen)
        {
            if (buf[i + j] != s[j])
            {
                match = 0;
                break;
            }
            j++;
        }
        if (match)
        {
            j = 0;
            while (j < slen)
            {
                buf[i + j] = '*';
                j++;
            }
            i += slen;
        }
        else
        {
            i++;
        }
    }

    write(1, buf, buflen);
    free(buf);
    return (0);
}
