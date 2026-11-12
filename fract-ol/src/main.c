/*
** main.c — Fract'ol entry point.
**
** Lifecycle:
**   1. Zero-initialise the t_fractal struct.
**   2. Parse command-line arguments → fractal type + constants.
**   3. Initialise viewport defaults.
**   4. Initialise miniLibX (window, image, hooks).
**   5. Render the first frame.
**   6. Enter the mlx event loop.
**
** If argument parsing fails (no args, unknown type), print usage and exit 0
** — the standard 42 convention for "help" output.
*/

# include "fract-ol.h"
# include <stdlib.h>
# include <string.h>

int main(int argc, char **argv)
{
    t_fractal f;
    memset(&f, 0, sizeof(f));

    /* ---- argument parsing ---- */
    if (parse_args(argc, argv, &f) != 0)
        return (0);   /* usage already printed */

    /* ---- defaults ---- */
    init_fractal(&f);

    /* ---- miniLibX ---- */
    if (init_mlx(&f) != 0)
    {
        ft_putstr_fd("Error: could not initialise miniLibX.\n", 2);
        return (1);
    }

    /* ---- first render ---- */
    render_image(&f);

    /* ---- event loop (never returns until window closed) ---- */
    mlx_loop(f.mlx_ptr);

    /* ---- cleanup ---- */
    cleanup_and_exit(&f, 0);
    return (0);
}
