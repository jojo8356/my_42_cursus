#ifndef BSQ_H
#define BSQ_H

typedef struct sMap {
    int rows;
    int cols;
    char empty;
    char obs;
    char full;
    char **grid;
} tMap;

#endif
