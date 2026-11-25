#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct s_life {
    int width;
    int height;
    int iterations;
    char *board;
    char *next;
} t_life;

static int getindex(int x, int y, int width)
{
    return y * width + x;
}

static int inside(int x, int y, int width, int height)
{
    if (x < 0 || y < 0)
        return 0;
    if (x >= width || y >= height)
        return 0;
    return 1;
}

static void drawcell(t_life *life, int x, int y)
{
    if (inside(x, y, life->width, life->height))
        life->board[getindex(x, y, life->width)] = 1;
}

static void readinput(t_life *life)
{
    char c;
    int x = 0;
    int y = 0;
    int pen = 0;  /* pen starts UP (not drawing) */

    while (read(0, &c, 1) > 0) {
        if (c == 'x') {
            pen = !pen;
            if (pen)
                drawcell(life, x, y);
        } else {
            if (c == 'w')
                y--;
            else if (c == 's')
                y++;
            else if (c == 'a')
                x--;
            else if (c == 'd')
                x++;
            if (pen)
                drawcell(life, x, y);
        }
    }
}

static int countneighbors(t_life *life, int x, int y)
{
    int count = 0;
    int dy = -1;
    while (dy <= 1) {
        int dx = -1;
        while (dx <= 1) {
            if (!(dx == 0 && dy == 0)) {
                int nx = x + dx;
                int ny = y + dy;
                if (inside(nx, ny, life->width, life->height))
                    count += life->board[getindex(nx, ny, life->width)];
            }
            dx++;
        }
        dy++;
    }
    return count;
}

static void step(t_life *life)
{
    int y = 0;
    while (y < life->height) {
        int x = 0;
        while (x < life->width) {
            int pos = getindex(x, y, life->width);
            int neighbors = countneighbors(life, x, y);
            if (life->board[pos]) {
                if (neighbors == 2 || neighbors == 3)
                    life->next[pos] = 1;
                else
                    life->next[pos] = 0;
            } else {
                if (neighbors == 3)
                    life->next[pos] = 1;
                else
                    life->next[pos] = 0;
            }
            x++;
        }
        y++;
    }
}

static void swapboards(t_life *life)
{
    char *tmp = life->board;
    life->board = life->next;
    life->next = tmp;
}

static void clearboard(char *board, int size)
{
    int i = 0;
    while (i < size) {
        board[i] = 0;
        i++;
    }
}

static void simulate(t_life *life)
{
    int i = 0;
    int size = life->width * life->height;
    while (i < life->iterations) {
        clearboard(life->next, size);
        step(life);
        swapboards(life);
        i++;
    }
}

static void printboard(t_life *life)
{
    int y = 0;
    while (y < life->height) {
        int x = 0;
        while (x < life->width) {
            if (life->board[getindex(x, y, life->width)])
                putchar('O');
            else
                putchar(' ');
            x++;
        }
        putchar('\n');
        y++;
    }
}

int main(int argc, char **argv)
{
    t_life life;
    int size;

    if (argc != 4)
        return 1;
    life.width = atoi(argv[1]);
    life.height = atoi(argv[2]);
    life.iterations = atoi(argv[3]);
    if (life.width <= 0 || life.height <= 0 || life.iterations < 0)
        return 1;
    size = life.width * life.height;
    life.board = calloc(size, sizeof(char));
    life.next = calloc(size, sizeof(char));
    if (!life.board || !life.next) {
        free(life.board);
        free(life.next);
        return 1;
    }
    readinput(&life);
    simulate(&life);
    printboard(&life);
    free(life.board);
    free(life.next);
    return 0;
}
