/*
** test_math.c — Unit tests for fractal iteration logic.
**
** Verifies the mathematical correctness of every fractal type against known
** properties:
**   •  Points inside the set → return max_iter (bounded)
**   •  Points outside the set → return < max_iter (escaped)
**   •  Known constants produce expected escape times
**   •  Complex arithmetic follows the correct recurrence
**   •  Phoenix recurrence correctly uses z_{n-1}
**   •  Multibrot power expansion is correct
**
** Note: the escape condition is |z|² > 4.0 (strict). Points with |z|² = 4.0
** exactly do NOT trigger escape — they continue iterating.
*/

# include "fract-ol.h"
# include "test_framework.h"
# include <math.h>

int main(void)
{
    TEST_BEGIN();

    t_fractal f;

    /* ═══════════════════════════════════════════════════════════
    ** SUITE: Mandelbrot
    ** ═══════════════════════════════════════════════════════════ */
    TEST_SUITE("Mandelbrot iteration");

    /* The origin (0,0) is in the Mandelbrot set → bounded. */
    test_init_fractal(&f, FRACTAL_MANDELBROT, 0, 0);
    f.z_r = 0.0; f.z_i = 0.0;
    TEST_ASSERT_EQ(mandelbrot_iter(&f), f.max_iter);

    /* The point (2, 2) is far outside → escapes immediately. */
    test_init_fractal(&f, FRACTAL_MANDELBROT, 0, 0);
    f.z_r = 2.0; f.z_i = 2.0;
    int iter = mandelbrot_iter(&f);
    TEST_ASSERT(iter < 5);

    /* Point in the main cardioid (0.1, 0.1) → bounded. */
    test_init_fractal(&f, FRACTAL_MANDELBROT, 0, 0);
    f.z_r = 0.1; f.z_i = 0.1;
    TEST_ASSERT_EQ(mandelbrot_iter(&f), f.max_iter);

    /* Point in a period-2 bulb (-1, 0) → bounded. */
    test_init_fractal(&f, FRACTAL_MANDELBROT, 0, 0);
    f.z_r = -1.0; f.z_i = 0.0;
    TEST_ASSERT_EQ(mandelbrot_iter(&f), f.max_iter);

    /* Point (0.5, 0.5) → escapes. */
    test_init_fractal(&f, FRACTAL_MANDELBROT, 0, 0);
    f.z_r = 0.5; f.z_i = 0.5;
    iter = mandelbrot_iter(&f);
    TEST_ASSERT(iter < 100);

    /* Verify recurrence: with max_iter=1, z₁ = c. */
    test_init_fractal(&f, FRACTAL_MANDELBROT, 0, 0);
    f.max_iter = 1;
    f.z_r = 0.3; f.z_i = 0.5;
    mandelbrot_iter(&f);
    TEST_ASSERT_NEAR(f.z_r, 0.3, 1e-10);
    TEST_ASSERT_NEAR(f.z_i, 0.5, 1e-10);

    /* ═══════════════════════════════════════════════════════════
    ** SUITE: Julia
    ** ═══════════════════════════════════════════════════════════ */
    TEST_SUITE("Julia iteration");

    /* Julia with c = 0: z = z². Origin → bounded. */
    test_init_fractal(&f, FRACTAL_JULIA, 0, 0);
    f.z_r = 0.0; f.z_i = 0.0;
    TEST_ASSERT_EQ(julia_iter(&f), f.max_iter);

    /* Julia with c = 0, z = (2, 0): z₁ = 4 → escapes at iter 1. */
    test_init_fractal(&f, FRACTAL_JULIA, 0, 0);
    f.z_r = 2.0; f.z_i = 0.0;
    TEST_ASSERT_EQ(julia_iter(&f), 1);

    /* Julia with c = -0.8 + 0.156i (famous constant). */
    test_init_fractal(&f, FRACTAL_JULIA, -0.8, 0.156);
    f.z_r = 0.0; f.z_i = 0.0;
    iter = julia_iter(&f);
    TEST_ASSERT(iter > 0);

    /* Verify Julia recurrence with max_iter=1: z₁ = z₀² + c.
    ** z₀ = (1, 1), c = (-0.8, 0.156).
    ** z₁ = (1+i)² + (-0.8+0.156i) = (0, 2) + (-0.8, 0.156) = (-0.8, 2.156) */
    test_init_fractal(&f, FRACTAL_JULIA, -0.8, 0.156);
    f.max_iter = 1;
    f.z_r = 1.0; f.z_i = 1.0;
    julia_iter(&f);
    TEST_ASSERT_NEAR(f.z_r, -0.8, 1e-10);
    TEST_ASSERT_NEAR(f.z_i, 2.156, 1e-10);

    /* Julia with c = i (0, 1): z₀ = (1, 0) → z₁ = (1, 1). */
    test_init_fractal(&f, FRACTAL_JULIA, 0, 1);
    f.max_iter = 1;
    f.z_r = 1.0; f.z_i = 0.0;
    julia_iter(&f);
    TEST_ASSERT_NEAR(f.z_r, 1.0, 1e-10);
    TEST_ASSERT_NEAR(f.z_i, 1.0, 1e-10);

    /* ═══════════════════════════════════════════════════════════
    ** SUITE: Burning Ship
    ** ═══════════════════════════════════════════════════════════ */
    TEST_SUITE("Burning Ship iteration");

    /* Origin → bounded. */
    test_init_fractal(&f, FRACTAL_BURNING_SHIP, 0, 0);
    f.z_r = 0.0; f.z_i = 0.0;
    TEST_ASSERT_EQ(burning_ship_iter(&f), f.max_iter);

    /* Point (-2, -2) → escapes quickly. */
    test_init_fractal(&f, FRACTAL_BURNING_SHIP, 0, 0);
    f.z_r = -2.0; f.z_i = -2.0;
    iter = burning_ship_iter(&f);
    TEST_ASSERT(iter < 10);

    /* Burning Ship with max_iter=1: z₀ = 0, c = pixel.
    ** z₁ = (|0|+i|0|)² + c = c = pixel. Set pixel = (-0.5, 1.5). */
    test_init_fractal(&f, FRACTAL_BURNING_SHIP, -0.5, -0.5);
    f.max_iter = 1;
    f.z_r = -0.5; f.z_i = 1.5;
    burning_ship_iter(&f);
    TEST_ASSERT_NEAR(f.z_r, -0.5, 1e-10);
    TEST_ASSERT_NEAR(f.z_i, 1.5, 1e-10);

    /* ═══════════════════════════════════════════════════════════
    ** SUITE: Tricorn (Mandelbar)
    ** ═══════════════════════════════════════════════════════════ */
    TEST_SUITE("Tricorn iteration");

    /* Origin → bounded. */
    test_init_fractal(&f, FRACTAL_TRICORN, 0, 0);
    f.z_r = 0.0; f.z_i = 0.0;
    TEST_ASSERT_EQ(tricorn_iter(&f), f.max_iter);

    /* Tricorn: z₀ = 0, c = pixel = (1, 1).
    ** z₁ = conj(0)² + c = c = (1, 1). |z₁|² = 2 < 4.
    ** z₂ = conj(1+i)² + c = (1-i)² + (1,1) = (0,-2)+(1,1) = (1,-1). |z₂|² = 2 < 4.
    ** z₃ = conj(1-i)² + c = (1+i)² + (1,1) = (0,2)+(1,1) = (1,3). |z₃|² = 10 > 4. Escape.
    ** Returns 3. */
    test_init_fractal(&f, FRACTAL_TRICORN, 0, 0);
    f.z_r = 1.0; f.z_i = 1.0;
    TEST_ASSERT_EQ(tricorn_iter(&f), 3);

    /* Tricorn with max_iter=1: z₁ = c = pixel. */
    test_init_fractal(&f, FRACTAL_TRICORN, 0.5, 0.5);
    f.max_iter = 1;
    f.z_r = 0.5; f.z_i = 0.5;
    tricorn_iter(&f);
    TEST_ASSERT_NEAR(f.z_r, 0.5, 1e-10);
    TEST_ASSERT_NEAR(f.z_i, 0.5, 1e-10);

    /* ═══════════════════════════════════════════════════════════
    ** SUITE: Phoenix
    ** ═══════════════════════════════════════════════════════════ */
    TEST_SUITE("Phoenix iteration");

    /* Phoenix with δ=0, max_iter=1: reduces to Mandelbrot.
    ** z₀ = 0, c = pixel = (0.3, 0.5). z₁ = c = (0.3, 0.5). */
    test_init_fractal(&f, FRACTAL_PHOENIX, 0.3, 0.5);
    f.delta = 0.0;
    f.max_iter = 1;
    f.z_r = 0.3; f.z_i = 0.5;
    phoenix_iter(&f);
    TEST_ASSERT_NEAR(f.z_r, 0.3, 1e-10);
    TEST_ASSERT_NEAR(f.z_i, 0.5, 1e-10);

    /* Phoenix with δ=0.5, max_iter=2:
    ** z₀ = 0, c = pixel = (0.3, 0.5), δ = 0.5, z_{-1} = 0.
    ** z₁ = 0 + c + δ·0 = c = (0.3, 0.5). prev = z₀ = (0, 0).
    ** z₂ = z₁² + c + δ·prev = (0.09-0.25, 0.3) + (0.3, 0.5) + 0 = (0.14, 0.8). */
    test_init_fractal(&f, FRACTAL_PHOENIX, 0.3, 0.5);
    f.delta = 0.5;
    f.max_iter = 2;
    f.z_r = 0.3; f.z_i = 0.5;
    phoenix_iter(&f);
    TEST_ASSERT_NEAR(f.z_r, 0.14, 1e-10);
    TEST_ASSERT_NEAR(f.z_i, 0.8, 1e-10);

    /* Phoenix with large δ → escapes faster. */
    test_init_fractal(&f, FRACTAL_PHOENIX, 0.5, 0.5);
    f.delta = 0.8;
    f.z_r = 1.0; f.z_i = 1.0;
    iter = phoenix_iter(&f);
    TEST_ASSERT(iter < 50);

    /* ═══════════════════════════════════════════════════════════
    ** SUITE: Phoenix variant (Julia-style, fixed c)
    ** ═══════════════════════════════════════════════════════════ */
    TEST_SUITE("Phoenix variant iteration");

    /* δ=0, max_iter=1: z₀ = (0, 0), c = (0.3, 0.5).
    ** z₁ = z₀² + c = (0.3, 0.5). Same as Julia. */
    test_init_fractal(&f, FRACTAL_PHOENIX_VARIANT, 0.3, 0.5);
    f.delta = 0.0;
    f.max_iter = 1;
    f.z_r = 0.0; f.z_i = 0.0;
    phoenix_variant_iter(&f);
    TEST_ASSERT_NEAR(f.z_r, 0.3, 1e-10);
    TEST_ASSERT_NEAR(f.z_i, 0.5, 1e-10);

    /* ═══════════════════════════════════════════════════════════
    ** SUITE: Multibrot (Mandelbrot-style: z₀ = 0)
    ** ═══════════════════════════════════════════════════════════ */
    TEST_SUITE("Multibrot iteration");

    /* Multibrot power 2 = standard Mandelbrot. Origin → bounded. */
    test_init_fractal(&f, FRACTAL_MULTIBROT, 0, 0);
    f.multibrot_power = 2;
    f.z_r = 0.0; f.z_i = 0.0;
    TEST_ASSERT_EQ(multibrot_iter(&f), f.max_iter);

    /* Multibrot power 2, c = (2, 0): z₁ = 0² + 2 = 2. |z₁|² = 4.0.
    ** NOT > 4.0 → continues. z₂ = 2² + 2 = 6. |z₂|² = 36 > 4 → escape.
    ** Returns 2. */
    test_init_fractal(&f, FRACTAL_MULTIBROT, 0, 0);
    f.multibrot_power = 2;
    f.z_r = 2.0; f.z_i = 0.0;
    TEST_ASSERT_EQ(multibrot_iter(&f), 2);

    /* Multibrot power 3, c = (1, 0), max_iter=1: z₁ = 0³ + 1 = 1. */
    test_init_fractal(&f, FRACTAL_MULTIBROT, 0, 0);
    f.multibrot_power = 3;
    f.max_iter = 1;
    f.z_r = 1.0; f.z_i = 0.0;
    multibrot_iter(&f);
    TEST_ASSERT_NEAR(f.z_r, 1.0, 1e-10);
    TEST_ASSERT_NEAR(f.z_i, 0.0, 1e-10);

    /* Multibrot power 3, c = (2, 0): z₁ = 2, |z₁|² = 4.0 (not > 4).
    ** z₂ = 2³ + 2 = 10. |z₂|² = 100 > 4 → escape. Returns 2. */
    test_init_fractal(&f, FRACTAL_MULTIBROT, 0, 0);
    f.multibrot_power = 3;
    f.z_r = 2.0; f.z_i = 0.0;
    TEST_ASSERT_EQ(multibrot_iter(&f), 2);

    /* Multibrot power 3, c = (1, 1), max_iter=2:
    ** z₀ = 0. z₁ = 0³ + (1,1) = (1, 1).
    ** z₂ = (1+i)³ + (1,1) = (-2+2i) + (1+i) = (-1, 3).
    ** |z₂|² = 10 > 4, but max_iter=2 so we stop at iter 2. */
    test_init_fractal(&f, FRACTAL_MULTIBROT, 0, 0);
    f.multibrot_power = 3;
    f.max_iter = 2;
    f.z_r = 1.0; f.z_i = 1.0;
    multibrot_iter(&f);
    TEST_ASSERT_NEAR(f.z_r, -1.0, 1e-10);
    TEST_ASSERT_NEAR(f.z_i, 3.0, 1e-10);

    /* ═══════════════════════════════════════════════════════════
    ** SUITE: Edge cases & robustness
    ** ═══════════════════════════════════════════════════════════ */
    TEST_SUITE("Edge cases");

    /* Zero max_iter → return 0 immediately. */
    test_init_fractal(&f, FRACTAL_MANDELBROT, 0, 0);
    f.max_iter = 0;
    f.z_r = 0.0; f.z_i = 0.0;
    TEST_ASSERT_EQ(mandelbrot_iter(&f), 0);

    /* Very small max_iter. */
    test_init_fractal(&f, FRACTAL_JULIA, -0.7, 0.27015);
    f.max_iter = 1;
    f.z_r = 0.0; f.z_i = 0.0;
    TEST_ASSERT_EQ(julia_iter(&f), 1);

    /* Deep zoom: point inside the set → bounded (max_iter).
    /* (-0.5, 0.5) is inside the period-2 bulb → bounded. */
    test_init_fractal(&f, FRACTAL_MANDELBROT, 0, 0);
    f.max_iter = 5000;
    f.z_r = -0.5; f.z_i = 0.5;
    iter = mandelbrot_iter(&f);
    TEST_ASSERT(iter >= 4000);   /* deep structure, near max_iter */

    /* NaN/Inf safety: extreme values should not crash. */
    test_init_fractal(&f, FRACTAL_MANDELBROT, 0, 0);
    f.z_r = 1e300; f.z_i = 1e300;
    iter = mandelbrot_iter(&f);
    TEST_ASSERT(iter == 0 || iter == 1);

    return TEST_END();
}
