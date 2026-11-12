/*
** args.c — Command-line argument parsing and fractal name resolution.
**
** Supported invocations:
**   ./fractol mandelbrot
**   ./fractol julia <real> <imag>
**   ./fractol burning_ship
**   ./fractol tricorn
**   ./fractol phoenix <real> <imag> <delta>
**   ./fractol "phoenix variant" <real> <imag> <delta>
**   ./fractol multibrot [power]
**
** No arguments → print usage and exit 0 (common 42 convention).
*/

# include "fract-ol.h"
# include <stdlib.h>

/* Fractal name table — index matches t_fractal_type. */
static const char *names[FRACTAL_COUNT] = {
    "mandelbrot", "julia", "burning_ship", "tricorn",
    "phoenix", "phoenix variant", "multibrot"
};

/* ----------------------------------------------------------------------- */
char *fractal_name(t_fractal_type t)
{
    if (t >= 0 && t < FRACTAL_COUNT)
        return (char *)names[t];
    return (char *)"unknown";
}

/* ----------------------------------------------------------------------- */
/* Compare a case-insensitive string against a fractal name. */
static int name_match(const char *input, const char *name)
{
    if (!input || !name)
        return 0;
    while (*input && *name)
    {
        if (ft_tolower((unsigned char)*input) != ft_tolower((unsigned char)*name))
            return 0;
        input++;
        name++;
    }
    return *input == '\0' && *name == '\0';
}

/* ----------------------------------------------------------------------- */
int fractal_from_name(const char *name)
{
    if (!name)
        return -1;
    if (name_match(name, "mandelbrot"))
        return FRACTAL_MANDELBROT;
    if (name_match(name, "julia"))
        return FRACTAL_JULIA;
    if (name_match(name, "burning_ship") || name_match(name, "burning ship"))
        return FRACTAL_BURNING_SHIP;
    if (name_match(name, "tricorn") || name_match(name, "mandelbar"))
        return FRACTAL_TRICORN;
    if (name_match(name, "phoenix") && !name_match(name, "phoenix variant"))
        return FRACTAL_PHOENIX;
    if (name_match(name, "phoenix variant"))
        return FRACTAL_PHOENIX_VARIANT;
    if (name_match(name, "multibrot"))
        return FRACTAL_MULTIBROT;
    return -1;
}

/* ----------------------------------------------------------------------- */
void print_usage(void)
{
    ft_putstr_fd("Usage:\n", 2);
    ft_putstr_fd("  ./fractol mandelbrot\n", 2);
    ft_putstr_fd("  ./fractol julia <real> <imag>\n", 2);
    ft_putstr_fd("  ./fractol burning_ship\n", 2);
    ft_putstr_fd("  ./fractol tricorn\n", 2);
    ft_putstr_fd("  ./fractol phoenix <real> <imag> <delta>\n", 2);
    ft_putstr_fd("  ./fractol \"phoenix variant\" <real> <imag> <delta>\n", 2);
    ft_putstr_fd("  ./fractol multibrot [power]\n", 2);
}

/* ----------------------------------------------------------------------- */
static int parse_double(const char *s, double *out)
{
    if (!s || !*s)
        return -1;
    char *end = NULL;
    double v = strtod(s, &end);
    if (end == s || (end && *end != '\0'))
        return -1;
    *out = v;
    return 0;
}

/* ----------------------------------------------------------------------- */
int parse_args(int argc, char **argv, t_fractal *f)
{
    if (argc < 2)
    {
        print_usage();
        return -1;
    }

    int type = fractal_from_name(argv[1]);
    if (type < 0)
    {
        ft_putstr_fd("Error: unknown fractal type.\n", 2);
        print_usage();
        return -1;
    }

    f->type = (t_fractal_type)type;
    f->name = fractal_name(f->type);
    f->delta = 0.0;
    f->multibrot_power = 3;

    /* ---- fractal-specific argument handling ---- */
    switch (f->type)
    {
        case FRACTAL_MANDELBROT:
        case FRACTAL_BURNING_SHIP:
        case FRACTAL_TRICORN:
        case FRACTAL_MULTIBROT:
            /* No extra arguments required. */
            if (f->type == FRACTAL_MULTIBROT && argc >= 3)
            {
                double power;
                if (parse_double(argv[2], &power) != 0 ||
                    power < 2 || power > 10)
                {
                    ft_putstr_fd("Error: multibrot power must be 2-10.\n", 2);
                    return -1;
                }
                f->multibrot_power = (int)power;
            }
            break;

        case FRACTAL_JULIA:
            if (argc < 4)
            {
                /* Default Julia constant if not provided. */
                f->c_r = -0.8;
                f->c_i =  0.156;
            }
            else
            {
                if (parse_double(argv[2], &f->c_r) != 0 ||
                    parse_double(argv[3], &f->c_i) != 0)
                {
                    ft_putstr_fd("Error: invalid Julia parameters.\n", 2);
                    return -1;
                }
            }
            break;

        case FRACTAL_PHOENIX:
        case FRACTAL_PHOENIX_VARIANT:
            if (argc < 5)
            {
                f->c_r  = 0.5667;
                f->c_i  = 0.0;
                f->delta = -0.5;
            }
            else
            {
                if (parse_double(argv[2], &f->c_r) != 0 ||
                    parse_double(argv[3], &f->c_i) != 0 ||
                    parse_double(argv[4], &f->delta) != 0)
                {
                    ft_putstr_fd("Error: invalid Phoenix parameters.\n", 2);
                    return -1;
                }
            }
            break;

        default:
            break;
    }

    return 0;
}
