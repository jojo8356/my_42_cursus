/*
** test_args.c — Unit tests for argument parsing and fractal name resolution.
**
** Verifies:
**   •  All fractal type names resolve correctly
**   •  Julia accepts numeric parameters
**   •  Phoenix accepts real, imag, and delta
**   •  Multibrot accepts power argument
**   •  Invalid types are rejected
**   •  Case-insensitive matching
**   •  Default values when parameters are missing
*/



# include "fract-ol.h"
# include "test_framework.h"

# include <string.h>

int main(void)
{
    TEST_BEGIN();

    /* ═══════════════════════════════════════════════════════════
    ** SUITE: Fractal name resolution
    ** ═══════════════════════════════════════════════════════════ */
    TEST_SUITE("Fractal name resolution");

    TEST_ASSERT_EQ(fractal_from_name("mandelbrot"), FRACTAL_MANDELBROT);
    TEST_ASSERT_EQ(fractal_from_name("julia"), FRACTAL_JULIA);
    TEST_ASSERT_EQ(fractal_from_name("burning_ship"), FRACTAL_BURNING_SHIP);
    TEST_ASSERT_EQ(fractal_from_name("burning ship"), FRACTAL_BURNING_SHIP);
    TEST_ASSERT_EQ(fractal_from_name("tricorn"), FRACTAL_TRICORN);
    TEST_ASSERT_EQ(fractal_from_name("mandelbar"), FRACTAL_TRICORN);
    TEST_ASSERT_EQ(fractal_from_name("phoenix"), FRACTAL_PHOENIX);
    TEST_ASSERT_EQ(fractal_from_name("phoenix variant"), FRACTAL_PHOENIX_VARIANT);
    TEST_ASSERT_EQ(fractal_from_name("multibrot"), FRACTAL_MULTIBROT);

    /* Invalid names. */
    TEST_ASSERT_EQ(fractal_from_name("invalid"), -1);
    TEST_ASSERT_EQ(fractal_from_name(""), -1);
    TEST_ASSERT_EQ(fractal_from_name("mandelbrot2"), -1);

    /* Case insensitivity. */
    TEST_ASSERT_EQ(fractal_from_name("Mandelbrot"), FRACTAL_MANDELBROT);
    TEST_ASSERT_EQ(fractal_from_name("JULIA"), FRACTAL_JULIA);
    TEST_ASSERT_EQ(fractal_from_name("Burning_Ship"), FRACTAL_BURNING_SHIP);
    TEST_ASSERT_EQ(fractal_from_name("TriCorn"), FRACTAL_TRICORN);

    /* ═══════════════════════════════════════════════════════════
    ** SUITE: Name → string
    ** ═══════════════════════════════════════════════════════════ */
    TEST_SUITE("Name → string");

    TEST_ASSERT_STR(fractal_name(FRACTAL_MANDELBROT), "mandelbrot");
    TEST_ASSERT_STR(fractal_name(FRACTAL_JULIA), "julia");
    TEST_ASSERT_STR(fractal_name(FRACTAL_BURNING_SHIP), "burning_ship");
    TEST_ASSERT_STR(fractal_name(FRACTAL_TRICORN), "tricorn");
    TEST_ASSERT_STR(fractal_name(FRACTAL_PHOENIX), "phoenix");
    TEST_ASSERT_STR(fractal_name(FRACTAL_PHOENIX_VARIANT), "phoenix variant");
    TEST_ASSERT_STR(fractal_name(FRACTAL_MULTIBROT), "multibrot");

    /* ═══════════════════════════════════════════════════════════
    ** SUITE: Argument parsing
    ** ═══════════════════════════════════════════════════════════ */
    TEST_SUITE("Argument parsing");

    t_fractal f;
    memset(&f, 0, sizeof(f));

    /* No args → returns -1 (usage printed). */
    char *argv_noargs[] = { "./fractol" };
    TEST_ASSERT_EQ(parse_args(1, argv_noargs, &f), -1);

    /* Mandelbrot. */
    char *argv_m[] = { "./fractol", "mandelbrot" };
    TEST_ASSERT_EQ(parse_args(2, argv_m, &f), 0);
    TEST_ASSERT_EQ(f.type, FRACTAL_MANDELBROT);

    /* Julia with parameters. */
    char *argv_j[] = { "./fractol", "julia", "-0.8", "0.156" };
    memset(&f, 0, sizeof(f));
    TEST_ASSERT_EQ(parse_args(4, argv_j, &f), 0);
    TEST_ASSERT_EQ(f.type, FRACTAL_JULIA);
    TEST_ASSERT_NEAR(f.c_r, -0.8, 1e-10);
    TEST_ASSERT_NEAR(f.c_i, 0.156, 1e-10);

    /* Julia without parameters → defaults. */
    char *argv_j_def[] = { "./fractol", "julia" };
    memset(&f, 0, sizeof(f));
    TEST_ASSERT_EQ(parse_args(2, argv_j_def, &f), 0);
    TEST_ASSERT_NEAR(f.c_r, -0.8, 1e-10);
    TEST_ASSERT_NEAR(f.c_i, 0.156, 1e-10);

    /* Burning Ship. */
    char *argv_bs[] = { "./fractol", "burning_ship" };
    memset(&f, 0, sizeof(f));
    TEST_ASSERT_EQ(parse_args(2, argv_bs, &f), 0);
    TEST_ASSERT_EQ(f.type, FRACTAL_BURNING_SHIP);

    /* Tricorn. */
    char *argv_t[] = { "./fractol", "tricorn" };
    memset(&f, 0, sizeof(f));
    TEST_ASSERT_EQ(parse_args(2, argv_t, &f), 0);
    TEST_ASSERT_EQ(f.type, FRACTAL_TRICORN);

    /* Phoenix with parameters. */
    char *argv_p[] = { "./fractol", "phoenix", "0.5667", "0.0", "-0.5" };
    memset(&f, 0, sizeof(f));
    TEST_ASSERT_EQ(parse_args(5, argv_p, &f), 0);
    TEST_ASSERT_EQ(f.type, FRACTAL_PHOENIX);
    TEST_ASSERT_NEAR(f.c_r, 0.5667, 1e-10);
    TEST_ASSERT_NEAR(f.c_i, 0.0, 1e-10);
    TEST_ASSERT_NEAR(f.delta, -0.5, 1e-10);

    /* Phoenix without parameters → defaults. */
    char *argv_p_def[] = { "./fractol", "phoenix" };
    memset(&f, 0, sizeof(f));
    TEST_ASSERT_EQ(parse_args(2, argv_p_def, &f), 0);
    TEST_ASSERT_EQ(f.type, FRACTAL_PHOENIX);
    TEST_ASSERT_NEAR(f.c_r, 0.5667, 1e-10);
    TEST_ASSERT_NEAR(f.delta, -0.5, 1e-10);

    /* Phoenix variant. */
    char *argv_pv[] = { "./fractol", "phoenix variant", "0.5667", "0.0", "-0.5" };
    memset(&f, 0, sizeof(f));
    TEST_ASSERT_EQ(parse_args(5, argv_pv, &f), 0);
    TEST_ASSERT_EQ(f.type, FRACTAL_PHOENIX_VARIANT);

    /* Multibrot with power. */
    char *argv_mb[] = { "./fractol", "multibrot", "4" };
    memset(&f, 0, sizeof(f));
    TEST_ASSERT_EQ(parse_args(3, argv_mb, &f), 0);
    TEST_ASSERT_EQ(f.type, FRACTAL_MULTIBROT);
    TEST_ASSERT_EQ(f.multibrot_power, 4);

    /* Multibrot without power → default 3. */
    char *argv_mb_def[] = { "./fractol", "multibrot" };
    memset(&f, 0, sizeof(f));
    TEST_ASSERT_EQ(parse_args(2, argv_mb_def, &f), 0);
    TEST_ASSERT_EQ(f.multibrot_power, 3);

    /* Multibrot with invalid power. */
    char *argv_mb_bad[] = { "./fractol", "multibrot", "abc" };
    memset(&f, 0, sizeof(f));
    TEST_ASSERT_EQ(parse_args(3, argv_mb_bad, &f), -1);

    /* Multibrot with out-of-range power. */
    char *argv_mb_oob[] = { "./fractol", "multibrot", "11" };
    memset(&f, 0, sizeof(f));
    TEST_ASSERT_EQ(parse_args(3, argv_mb_oob, &f), -1);

    /* Invalid fractal type. */
    char *argv_bad[] = { "./fractol", "invalid_type" };
    memset(&f, 0, sizeof(f));
    TEST_ASSERT_EQ(parse_args(2, argv_bad, &f), -1);

    /* Invalid Julia parameters. */
    char *argv_j_bad[] = { "./fractol", "julia", "notanumber", "0.156" };
    memset(&f, 0, sizeof(f));
    TEST_ASSERT_EQ(parse_args(4, argv_j_bad, &f), -1);

    /* Invalid Phoenix parameters. */
    char *argv_p_bad[] = { "./fractol", "phoenix", "0.5", "abc", "-0.5" };
    memset(&f, 0, sizeof(f));
    TEST_ASSERT_EQ(parse_args(5, argv_p_bad, &f), -1);

    return TEST_END();
}
