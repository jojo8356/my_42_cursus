/*
** main.c — FDF entry point.
**
** Lifecycle:
**   1. Zero-initialise the t_env struct.
**   2. Parse command-line argument (filename).
**   3. Read and parse the .fdf file.
**   4. Initialise miniLibX.
**   5. Render the first frame.
**   6. Enter the mlx event loop.
*/

# include "fdf.h"
# include <stdlib.h>
# include <string.h>

int main(int argc, char **argv)
{
    t_env env;
    memset(&env, 0, sizeof(env));

    /* ---- argument check ---- */
    if (argc != 2)
    {
        ft_putstr_fd("Usage: ./fdf <filename.fdf>\n", 2);
        return (1);
    }

    env.file_name = argv[1];

    /* ---- parse file ---- */
    env.map = malloc(sizeof(t_map));
    if (!env.map)
    {
        ft_putstr_fd("Error: memory allocation failed.\n", 2);
        return (1);
    }
    memset(env.map, 0, sizeof(t_map));

    if (parse_file(argv[1], env.map) != 0)
    {
        ft_putstr_fd("Error: could not parse file.\n", 2);
        free(env.map);
        return (1);
    }

    /* ---- default view ---- */
    env.scale = 1.0;
    env.offset_x = 0.0;
    env.offset_y = 0.0;
    env.rot_x = 0.0;
    env.rot_y = 0.0;
    env.rot_z = 0.0;
    env.iso_x = ISO_ANGLE_X;
    env.iso_y = ISO_ANGLE_Y;
    env.color_mode = MODE_GRADIENT;
    env.color_min = COLOR_RED;
    env.color_max = COLOR_GREEN;

    /* ---- miniLibX ---- */
    if (init_mlx(&env) != 0)
    {
        ft_putstr_fd("Error: could not initialise miniLibX.\n", 2);
        free_map(env.map);
        free(env.map);
        return (1);
    }

    /* ---- first render ---- */
    render(&env);

    /* ---- event loop ---- */
    mlx_loop(env.mlx);

    /* ---- cleanup ---- */
    cleanup(&env);
    free(env.map);
    return (0);
}
