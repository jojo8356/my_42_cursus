/* ************************************************************************** */
/*                                                                            */
/*   parser.c                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/20 by jojo8356            #+#    #+#             */
/*   Updated: 2026/11/20 by jojo8356           ###   ########.fr           */
/*                                                                            */
/*   Lecture et validation d'un fichier .cub : textures, couleurs,        */
/*   carte. La carte doit être fermée par des murs, le joueur doit être   */
/*   présent, les portes doivent être entourées de 2 murs.                 */
/* ************************************************************************** */

#include "cub3d.h"
# include <stdlib.h>
# include <string.h>
# include <stdio.h>

/* Lire une ligne (sans \\n) avec fgets. */
static char    *read_line(FILE *f, char *buf, size_t size)
{
    size_t    n;

    if (!fgets(buf, (int)size, f))
        return (NULL);
    n = strlen(buf);
    while (n > 0 && (buf[n - 1] == '\n' || buf[n - 1] == '\r'
        || buf[n - 1] == ' ' || buf[n - 1] == '\t'))
        buf[--n] = '\0';
    return (buf);
}

/* Vérifier qu'une ligne est la carte (ne contient que 0/1/N/S/E/W/D/P/A). */
static int    is_map_line(const char *line)
{
    int    i;

    if (!line || !line[0])
        return (0);
    for (i = 0; line[i]; i++)
    {
        if (line[i] != '0' && line[i] != '1' && line[i] != 'N'
            && line[i] != 'S' && line[i] != 'E' && line[i] != 'W'
            && line[i] != 'D' && line[i] != 'P' && line[i] != 'A')
            return (0);
    }
    return (1);
}

/* Extraire une valeur après un identifiant (ex: "NO path"). */
static char    *extract_value(const char *line, const char *id, int id_len)
{
    if (strncmp(line, id, id_len) != 0)
        return (NULL);
    line += id_len;
    while (*line == ' ' || *line == '\t')
        line++;
    if (!*line)
        return (NULL);
    return ((char *)line);
}

/* Parse une couleur "R,G,B" (0-255). */
static int    parse_color(const char *s, t_color *out)
{
    char    *end;
    long    r, g, b;

    while (*s == ' ' || *s == '\t')
        s++;
    r = strtol(s, &end, 10);
    if (*end != ',')
        return (0);
    s = end + 1;
    g = strtol(s, &end, 10);
    if (*end != ',')
        return (0);
    s = end + 1;
    b = strtol(s, &end, 10);
    while (*end == ' ' || *end == '\t' || *end == '\r')
        end++;
    if (*end != '\0')
        return (0);
    if (r < 0 || r > 255 || g < 0 || g > 255 || b < 0 || b > 255)
        return (0);
    *out = color((int)r, (int)g, (int)b, 255);
    return (1);
}

/* Flood-fill itérative : marquer les cellules accessibles depuis (y,x). */
static void    flood_fill(char **grid, int rows, int cols, int y, int x)
{
    int    stack[100000];
    int    top = 0;
    int    dirs[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
    int    d;

    if (y < 0 || y >= rows || x < 0 || x >= cols)
        return;
    stack[top++] = y * 1000 + x;
    while (top > 0)
    {
        int    idx = stack[--top];
        int    cy = idx / 1000;
        int    cx = idx % 1000;
        if (cy < 0 || cy >= rows || cx < 0 || cx >= cols)
            continue;
        if (grid[cy][cx] == '1' || grid[cy][cx] == 'F')
            continue;
        grid[cy][cx] = 'F';
        for (d = 0; d < 4; d++)
        {
            int    ny = cy + dirs[d][0];
            int    nx = cx + dirs[d][1];
            if (ny >= 0 && ny < rows && nx >= 0 && nx < cols)
                stack[top++] = ny * 1000 + nx;
        }
    }
}

/* Vérifier que la carte est fermée. */
static int    check_enclosed(char **grid, int rows, int cols, int py, int px)
{
    char    **copy;
    int        i;
    int        j;

    copy = (char **)xcalloc((size_t)rows, sizeof(char *));
    for (i = 0; i < rows; i++)
    {
        copy[i] = (char *)xcalloc((size_t)cols + 1, sizeof(char));
        strncpy(copy[i], grid[i], (size_t)cols);
    }
    flood_fill(copy, rows, cols, py, px);
    for (j = 0; j < cols; j++)
        if (copy[0][j] == 'F' || copy[rows - 1][j] == 'F')
            return (0);
    for (i = 0; i < rows; i++)
        if (copy[i][0] == 'F' || copy[i][cols - 1] == 'F')
            return (0);
    for (i = 0; i < rows; i++)
        for (j = 0; j < cols; j++)
            if (grid[i][j] != '1' && copy[i][j] != 'F')
                return (0);
    for (i = 0; i < rows; i++)
        free(copy[i]);
    free(copy);
    return (1);
}

/* Compter les voisins muraux d'une cellule (D doit avoir 2 murs). */
static int    count_wall_neighbors(char **grid, int rows, int cols, int y, int x)
{
    int    count;
    int    dirs[4][2] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};
    int    d;

    count = 0;
    for (d = 0; d < 4; d++)
    {
        int     ny = y + dirs[d][0];
        int     nx = x + dirs[d][1];
        if (ny >= 0 && ny < rows && nx >= 0 && nx < cols && grid[ny][nx] == '1')
            count++;
    }
    return (count);
}

/* Validation complète de la carte. */
int            map_validate(t_map *map)
{
    int    i;
    int    j;
    int    player_found = 0;

    if (!map->grid || map->rows <= 0 || map->cols <= 0)
    {
        fprintf(stderr, "cub3d: empty map\n");
        return (0);
    }
    if (!map->has_no || !map->has_so || !map->has_we || !map->has_ea)
    {
        fprintf(stderr, "cub3d: missing mandatory texture (NO/SO/WE/EA)\n");
        return (0);
    }
    if (!map->tex_no || !map->tex_so || !map->tex_we || !map->tex_ea)
    {
        fprintf(stderr, "cub3d: missing texture path\n");
        return (0);
    }
    for (i = 0; i < map->rows; i++)
        for (j = 0; j < map->cols; j++)
        {
            char    c = map->grid[i][j];
            if (c == 'N' || c == 'S' || c == 'E' || c == 'W')
            {
                if (player_found)
                {
                    fprintf(stderr, "cub3d: multiple player starts\n");
                    return (0);
                }
                player_found = 1;
                map->player_y = i;
                map->player_x = j;
                if (c == 'N')
                    map->dir = DIR_N;
                else if (c == 'S')
                    map->dir = DIR_S;
                else if (c == 'E')
                    map->dir = DIR_E;
                else
                    map->dir = DIR_W;
            }
        }
    if (!player_found)
    {
        fprintf(stderr, "cub3d: no player start (N/S/E/W)\n");
        return (0);
    }
    for (i = 0; i < map->rows; i++)
        for (j = 0; j < map->cols; j++)
        {
            if (map->grid[i][j] == 'D')
            {
                if (count_wall_neighbors(map->grid, map->rows, map->cols, i, j) != 2)
                {
                    fprintf(stderr, "cub3d: door at (%d,%d) not next to 2 walls\n", i, j);
                    return (0);
                }
                map->doors_count++;
            }
            else if (map->grid[i][j] == 'P')
                map->sprites_count++;
            else if (map->grid[i][j] == 'A')
                map->anim_sprites_count++;
        }
    if (!check_enclosed(map->grid, map->rows, map->cols, map->player_y, map->player_x))
    {
        fprintf(stderr, "cub3d: map not enclosed\n");
        return (0);
    }
    if (map->has_do && !map->tex_do)
    {
        fprintf(stderr, "cub3d: DO texture missing path\n");
        return (0);
    }
    if (map->has_sp && !map->tex_sp)
    {
        fprintf(stderr, "cub3d: SP texture missing path\n");
        return (0);
    }
    return (1);
}

/* Libération de la carte. */
void        map_free(t_map *map)
{
    int    i;

    if (!map->grid)
        return;
    for (i = 0; i < map->rows; i++)
        free(map->grid[i]);
    free(map->grid);
    map->grid = NULL;
    map->rows = 0;
    map->cols = 0;
    free(map->tex_no);
    free(map->tex_so);
    free(map->tex_we);
    free(map->tex_ea);
    free(map->tex_do);
    free(map->tex_sp);
    free(map->tex_a1);
    free(map->tex_a2);
    map->tex_no = map->tex_so = map->tex_we = map->tex_ea = NULL;
    map->tex_do = map->tex_sp = map->tex_a1 = map->tex_a2 = NULL;
}

/* Parseur principal. La carte est toujours la dernière section. */
int            parse_cub(const char *path, t_map *map)
{
    FILE        *f;
    char        line[8192];
    char        **grid = NULL;
    int            rows = 0;
    int            cols = 0;
    int            in_map = 0;

    f = fopen(path, "r");
    if (!f)
    {
        fprintf(stderr, "cub3d: cannot open '%s'\n", path);
        return (0);
    }
    memset(map, 0, sizeof(*map));
    map->floor = 0;
    map->ceiling = 0;

    while (read_line(f, line, sizeof(line)) != NULL)
    {
        char    *v;
        t_color    c;

        if (is_map_line(line))
        {
            in_map = 1;
            grid = (char **)xrealloc(grid, (size_t)(rows + 1) * sizeof(char *));
            grid[rows] = strdup(line);
            if ((int)strlen(line) > cols)
                cols = (int)strlen(line);
            rows++;
            continue;
        }
        if (in_map)
            break;
        v = extract_value(line, "NO", 2);
        if (v)
        {
            free(map->tex_no);
            map->tex_no = strdup(v);
            map->has_no = 1;
            continue;
        }
        v = extract_value(line, "SO", 2);
        if (v)
        {
            free(map->tex_so);
            map->tex_so = strdup(v);
            map->has_so = 1;
            continue;
        }
        v = extract_value(line, "WE", 2);
        if (v)
        {
            free(map->tex_we);
            map->tex_we = strdup(v);
            map->has_we = 1;
            continue;
        }
        v = extract_value(line, "EA", 2);
        if (v)
        {
            free(map->tex_ea);
            map->tex_ea = strdup(v);
            map->has_ea = 1;
            continue;
        }
        v = extract_value(line, "DO", 2);
        if (v)
        {
            free(map->tex_do);
            map->tex_do = strdup(v);
            map->has_do = 1;
            map->bonus_textures = 1;
            continue;
        }
        v = extract_value(line, "SP", 2);
        if (v)
        {
            free(map->tex_sp);
            map->tex_sp = strdup(v);
            map->has_sp = 1;
            map->bonus_textures = 1;
            continue;
        }
        v = extract_value(line, "A1", 2);
        if (v)
        {
            free(map->tex_a1);
            map->tex_a1 = strdup(v);
            map->has_a1 = 1;
            map->bonus_textures = 1;
            continue;
        }
        v = extract_value(line, "A2", 2);
        if (v)
        {
            free(map->tex_a2);
            map->tex_a2 = strdup(v);
            map->has_a2 = 1;
            map->bonus_textures = 1;
            continue;
        }
        if (line[0] == 'F' && (line[1] == ' ' || line[1] == '\t' || line[1] == '\0'))
        {
            if (!parse_color(line + 1, &c))
            {
                fprintf(stderr, "cub3d: invalid floor color '%s'\n", line);
                fclose(f);
                return (0);
            }
            map->floor = c;
            continue;
        }
        if (line[0] == 'C' && (line[1] == ' ' || line[1] == '\t' || line[1] == '\0'))
        {
            if (!parse_color(line + 1, &c))
            {
                fprintf(stderr, "cub3d: invalid ceiling color '%s'\n", line);
                fclose(f);
                return (0);
            }
            map->ceiling = c;
            continue;
        }
        /* Ligne vide ou inconnu avant la carte : ignorer */
    }
    fclose(f);
    /* égaliser la largeur des lignes avec des espaces */
    for (int i = 0; i < rows; i++)
    {
        if ((int)strlen(grid[i]) < cols)
        {
            grid[i] = (char *)xrealloc(grid[i], (size_t)cols + 1);
            memset(grid[i] + strlen(grid[i]), ' ', (size_t)(cols - strlen(grid[i])));
            grid[i][cols] = '\0';
        }
    }
    map->grid = grid;
    map->rows = rows;
    map->cols = cols;
    if (!grid || rows == 0)
    {
        fprintf(stderr, "cub3d: no map found\n");
        return (0);
    }
    return (1);
}
