#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

typedef struct sMap {
    int rows;
    int cols;
    char empty;
    char obs;
    char full;
    char **grid;
} tMap;

static void initMap(tMap *map)
{
    map->rows = 0;
    map->cols = 0;
    map->empty = 0;
    map->obs = 0;
    map->full = 0;
    map->grid = NULL;
}

static int parseHead(FILE *file, tMap *map)
{
    char sep1, sep2, sep3, end;

    if (fscanf(file, "%d%c%c%c%c%c%c%c",
               &map->rows, &sep1, &map->empty, &sep2,
               &map->obs, &sep3, &map->full, &end) != 8)
        return 0;
    if (sep1 != ' ' || sep2 != ' ' || sep3 != ' ')
        return 0;
    if (end != '\n')
        return 0;
    if (map->rows <= 0)
        return 0;
    if (map->empty == map->obs || map->empty == map->full
        || map->obs == map->full)
        return 0;
    return 1;
}

static int saveLine(tMap *map, char *line, int row, int len)
{
    int col;

    if (len <= 1 || line[len - 1] != '\n')
        return 0;
    if (row == 0) {
        map->cols = len - 1;
        if (map->cols <= 0)
            return 0;
    } else if (len - 1 != map->cols)
        return 0;
    map->grid[row] = calloc(map->cols + 1, sizeof(char));
    if (map->grid[row] == NULL)
        return 0;
    col = 0;
    while (col < map->cols) {
        if (line[col] != map->empty && line[col] != map->obs)
            return 0;
        map->grid[row][col] = line[col];
        col++;
    }
    return 1;
}

static int readMap(FILE *file, tMap *map)
{
    char *line = NULL;
    size_t size = 0;
    ssize_t len;
    int row = 0;

    if (parseHead(file, map) == 0)
        return 0;
    map->grid = calloc(map->rows, sizeof(char *));
    if (map->grid == NULL)
        return 0;
    while (row < map->rows) {
        len = getline(&line, &size, file);
        if (len == -1 || saveLine(map, line, row, len) == 0) {
            free(line);
            return 0;
        }
        row++;
    }
    len = getline(&line, &size, file);
    free(line);
    if (len != -1)
        return 0;
    return 1;
}

static int min3(int a, int b, int c)
{
    int min = a;
    if (b < min)
        min = b;
    if (c < min)
        min = c;
    return min;
}

static void fillSquare(tMap *map, int bestRow, int bestCol, int bestSize)
{
    int row = bestRow - bestSize + 1;
    while (row <= bestRow) {
        int col = bestCol - bestSize + 1;
        while (col <= bestCol) {
            map->grid[row][col] = map->full;
            col++;
        }
        row++;
    }
}

static int solveMap(tMap *map)
{
    int *dp;
    int row = 1;
    int bestSize = 0;
    int bestRow = 0;
    int bestCol = 0;

    dp = calloc(map->cols + 1, sizeof(int));
    if (dp == NULL)
        return 0;
    while (row <= map->rows) {
        int diag = 0;
        int col = 1;
        while (col <= map->cols) {
            int oldTop = dp[col];
            if (map->grid[row - 1][col - 1] == map->obs)
                dp[col] = 0;
            else {
                dp[col] = min3(dp[col], dp[col - 1], diag) + 1;
                if (dp[col] > bestSize) {
                    bestSize = dp[col];
                    bestRow = row - 1;
                    bestCol = col - 1;
                }
            }
            diag = oldTop;
            col++;
        }
        row++;
    }
    free(dp);
    if (bestSize > 0)
        fillSquare(map, bestRow, bestCol, bestSize);
    return 1;
}

static void printMap(tMap *map)
{
    int row = 0;
    while (row < map->rows) {
        fputs(map->grid[row], stdout);
        fputs("\n", stdout);
        row++;
    }
}

static void freeMap(tMap *map)
{
    int row = 0;
    if (map->grid == NULL)
        return;
    while (row < map->rows) {
        free(map->grid[row]);
        row++;
    }
    free(map->grid);
    map->grid = NULL;
}

static int processMap(FILE *file)
{
    tMap map;

    initMap(&map);
    if (readMap(file, &map) == 0) {
        fputs("map error\n", stderr);
        freeMap(&map);
        return 1;
    }
    solveMap(&map);
    printMap(&map);
    freeMap(&map);
    return 0;
}

int main(int argc, char **argv)
{
    FILE *file;
    int i;

    if (argc == 1)
        return processMap(stdin);
    i = 1;
    while (i < argc) {
        file = fopen(argv[i], "r");
        if (file == NULL)
            fputs("map error\n", stderr);
        else {
            processMap(file);
            fclose(file);
        }
        if (i + 1 < argc)
            fputs("\n", stdout);
        i++;
    }
    return 0;
}
