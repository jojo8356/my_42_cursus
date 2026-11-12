/*
** parse.c — Read and parse .fdf files.
**
** File format: each line contains space-separated integers representing
** height values. All lines must have the same number of values.
**
** Example:
**   0 0 0
**   1 2 0
**   0 0 1
**
** This produces a 3x3 map with height values.
**
** All functions here are pure (no mlx calls), fully unit-testable headless.
*/

# include "fdf.h"
# include <stdlib.h>
# include <stdio.h>

/* ----------------------------------------------------------------------- *
** Parse a single line of integers into an array.
** Returns the count, or -1 on error.
** ----------------------------------------------------------------------- */
static int parse_line(const char *line, int *out, int max_count)
{
    int count = 0;
    const char *p = line;

    while (*p && count < max_count)
    {
        /* Skip whitespace. */
        while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')
            p++;
        if (!*p)
            break;

        /* Parse integer (may be negative). */
        int sign = 1;
        if (*p == '-')
        {
            sign = -1;
            p++;
        }
        if (!*p || *p < '0' || *p > '9')
            return -1;

        int val = 0;
        while (*p >= '0' && *p <= '9')
        {
            val = val * 10 + (*p - '0');
            p++;
        }
        out[count] = sign * val;
        count++;
    }
    return count;
}

/* ----------------------------------------------------------------------- *
** Parse a .fdf file into a t_map struct.
** Returns 0 on success, -1 on error.
** ----------------------------------------------------------------------- */
int parse_file(const char *filename, t_map *map)
{
    FILE *fp = fopen(filename, "r");
    if (!fp)
        return -1;

    /* Read first line to determine width. */
    char line[65536];
    if (!fgets(line, sizeof(line), fp))
    {
        fclose(fp);
        return -1;
    }

    int *first_row = malloc(sizeof(int) * MAX_GRID_SIZE);
    if (!first_row)
    {
        fclose(fp);
        return -1;
    }

    int width = parse_line(line, first_row, MAX_GRID_SIZE);
    if (width <= 0)
    {
        free(first_row);
        fclose(fp);
        return -1;
    }

    /* Count rows. */
    int height = 1;
    int row_vals[MAX_GRID_SIZE];
    while (fgets(line, sizeof(line), fp))
    {
        int count = parse_line(line, row_vals, MAX_GRID_SIZE);
        if (count != width)
        {
            /* Ragged grid — reject. */
            free(first_row);
            fclose(fp);
            return -1;
        }
        height++;
    }
    fclose(fp);

    /* Allocate grid. */
    map->width = width;
    map->height = height;
    map->grid = malloc(sizeof(t_point) * width * height);
    if (!map->grid)
    {
        free(first_row);
        return -1;
    }

    /* Fill grid. Reset file pointer. */
    fp = fopen(filename, "r");
    if (!fp)
    {
        free(first_row);
        map->grid = NULL;
        return -1;
    }

    /* Read and discard first line (already stored in first_row). */
    if (!fgets(line, sizeof(line), fp))
    {
        free(first_row);
        fclose(fp);
        free(map->grid);
        map->grid = NULL;
        return -1;
    }

    /* Find min/max z. */
    map->min_z = first_row[0];
    map->max_z = first_row[0];

    for (int i = 0; i < width; i++)
    {
        map->grid[i].x = i;
        map->grid[i].y = 0;
        map->grid[i].z = first_row[i];
        map->grid[i].color = 0;
        if (first_row[i] < map->min_z)
            map->min_z = first_row[i];
        if (first_row[i] > map->max_z)
            map->max_z = first_row[i];
    }

    int idx = width;
    int y = 1;
    while (fgets(line, sizeof(line), fp) && y < height)
    {
        int vals[MAX_GRID_SIZE];
        int count = parse_line(line, vals, MAX_GRID_SIZE);
        if (count != width)
        {
            free(first_row);
            fclose(fp);
            free(map->grid);
            map->grid = NULL;
            return -1;
        }
        for (int i = 0; i < width; i++)
        {
            map->grid[idx].x = i;
            map->grid[idx].y = y;
            map->grid[idx].z = vals[i];
            map->grid[idx].color = 0;
            if (vals[i] < map->min_z)
                map->min_z = vals[i];
            if (vals[i] > map->max_z)
                map->max_z = vals[i];
            idx++;
        }
        y++;
    }

    free(first_row);
    fclose(fp);
    return 0;
}

/* ----------------------------------------------------------------------- */
void free_map(t_map *map)
{
    if (map->grid)
        free(map->grid);
    map->grid = NULL;
    map->width = 0;
    map->height = 0;
    map->min_z = 0;
    map->max_z = 0;
}
