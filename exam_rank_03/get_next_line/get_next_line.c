/*
** get_next_line.c - Exam Rank 03, level 1
** Read one line (ending with '\n' or EOF) from a file descriptor.
** Allowed functions: read, free, malloc
** Compiled with -D BUFFER_SIZE=xx
** Memory leak free.
*/
#include <unistd.h>
#include <stdlib.h>
#include "get_next_line.h"

#ifndef BUFFER_SIZE
#define BUFFER_SIZE 4096
#endif

char    *get_next_line(int fd)
{
    static char    buffer[BUFFER_SIZE];
    static int    bufpos = 0;
    static int    buflen = 0;
    static int    eof = 0;

    char    *line;
    int        line_cap;
    int        line_len;
    int        c;
    int        i;

    line_cap = 16;
    line = (char *)malloc((size_t)line_cap);
    if (line == NULL)
        return (NULL);
    line_len = 0;

    while (1)
    {
        /* Consume characters currently buffered */
        while (bufpos < buflen)
        {
            c = buffer[bufpos];
            bufpos++;
            if (c == '\n')
            {
                if (line_len + 1 >= line_cap)
                {
                    int        new_cap;
                    char    *tmp;

                    new_cap = line_cap * 2;
                    tmp = (char *)malloc((size_t)new_cap);
                    if (tmp == NULL)
                    {
                        free(line);
                        return (NULL);
                    }
                    for (i = 0; i < line_len; i++)
                        tmp[i] = line[i];
                    free(line);
                    line = tmp;
                    line_cap = new_cap;
                }
                line[line_len] = '\n';
                line_len++;
                line[line_len] = '\0';
                return (line);
            }
            if (line_len + 1 >= line_cap)
            {
                int        new_cap;
                char    *tmp;

                new_cap = line_cap * 2;
                tmp = (char *)malloc((size_t)new_cap);
                if (tmp == NULL)
                {
                    free(line);
                    return (NULL);
                }
                for (i = 0; i < line_len; i++)
                    tmp[i] = line[i];
                free(line);
                line = tmp;
                line_cap = new_cap;
            }
            line[line_len] = (char)c;
            line_len++;
        }

        /* Buffer exhausted */
        if (eof)
        {
            if (line_len == 0)
            {
                free(line);
                return (NULL);
            }
            line[line_len] = '\0';
            return (line);
        }

        /* Read more from the file descriptor */
        bufpos = 0;
        buflen = (int)read(fd, buffer, BUFFER_SIZE);
        if (buflen < 0)
        {
            free(line);
            buflen = 0;
            return (NULL);
        }
        if (buflen == 0)
            eof = 1;
    }
}
