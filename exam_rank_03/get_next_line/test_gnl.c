/*
** test_gnl.c - Test driver for get_next_line.
** Usage: ./test_gnl <file>
** Reads the file line by line using get_next_line and prints each line.
*/
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include "get_next_line.h"

int    main(int argc, char **argv)
{
    int    fd;
    char    *line;

    if (argc != 2)
        return (1);
    fd = open(argv[1], O_RDONLY);
    if (fd < 0)
    {
        perror("open");
        return (1);
    }
    while ((line = get_next_line(fd)) != NULL)
    {
        printf("%s", line);
        free(line);
    }
    close(fd);
    return (0);
}
