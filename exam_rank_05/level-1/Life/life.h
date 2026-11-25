#ifndef LIFE_H
#define LIFE_H

typedef struct s_life {
    int width;
    int height;
    int iterations;
    char *board;
    char *next;
} t_life;

#endif
