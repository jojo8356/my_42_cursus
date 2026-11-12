/*
** test_transform.c — Unit tests for coordinate transforms and viewport helpers.
**
** Verifies:
**   •  Center pixel maps to origin (0, 0) in complex plane
**   •  Edge pixels map to ±zoom/2
**   •  Imaginary axis is inverted (top of screen = +imag)
**   •  zoom_at keeps the cursor point fixed
**   •  shift_view moves the viewport correctly
**   •  Zoom clamping prevents overflow/underflow
*/



# include "fract-ol.h"
# include "test_framework.h"

# include <math.h>

int main(void)
{
    TEST_BEGIN();

    t_fractal f;

    /* ═══════════════════════════════════════════════════════════
    ** SUITE: Screen → complex mapping
    ** ═══════════════════════════════════════════════════════════ */
    TEST_SUITE("Screen → complex mapping");

    /* Default viewport: zoom = 4, shift = 0. */
    test_init_fractal(&f, FRACTAL_MANDELBROT, 0, 0);

    /* Center of screen → origin. */
    TEST_ASSERT_NEAR(screen_to_complex_r(WIDTH / 2, &f), 0.0, 1e-10);
    TEST_ASSERT_NEAR(screen_to_complex_i(HEIGHT / 2, &f), 0.0, 1e-10);

    /* Left edge → -zoom/2 = -2. */
    TEST_ASSERT_NEAR(screen_to_complex_r(0, &f), -2.0, 1e-10);

    /* Right edge → +zoom/2 = +2. */
    TEST_ASSERT_NEAR(screen_to_complex_r(WIDTH - 1, &f), 2.0 - 4.0/WIDTH, 1e-10);

    /* Top edge → +imag (inverted). */
    TEST_ASSERT_NEAR(screen_to_complex_i(0, &f), 2.0, 1e-10);

    /* Bottom edge → -imag (inverted). */
    TEST_ASSERT_NEAR(screen_to_complex_i(HEIGHT - 1, &f), -2.0 + 4.0/HEIGHT, 1e-10);

    /* ═══════════════════════════════════════════════════════════
    ** SUITE: Shifted viewport
    ** ═══════════════════════════════════════════════════════════ */
    TEST_SUITE("Shifted viewport");

    f.shift_r = 0.5;
    f.shift_i = -0.3;

    /* Center now maps to (shift_r, shift_i). */
    TEST_ASSERT_NEAR(screen_to_complex_r(WIDTH / 2, &f), 0.5, 1e-10);
    TEST_ASSERT_NEAR(screen_to_complex_i(HEIGHT / 2, &f), -0.3, 1e-10);

    /* ═══════════════════════════════════════════════════════════
    ** SUITE: Zoomed viewport
    ** ═══════════════════════════════════════════════════════════ */
    TEST_SUITE("Zoomed viewport");

    f.shift_r = 0.0;
    f.shift_i = 0.0;
    f.zoom = 0.5;   /* zoomed in */

    /* Center still at origin. */
    TEST_ASSERT_NEAR(screen_to_complex_r(WIDTH / 2, &f), 0.0, 1e-10);

    /* Edges now at ±0.25. */
    TEST_ASSERT_NEAR(screen_to_complex_r(0, &f), -0.25, 1e-10);
    TEST_ASSERT_NEAR(screen_to_complex_r(WIDTH - 1, &f), 0.25 - 0.5/WIDTH, 1e-10);

    /* ═══════════════════════════════════════════════════════════
    ** SUITE: shift_view
    ** ═══════════════════════════════════════════════════════════ */
    TEST_SUITE("shift_view");

    test_init_fractal(&f, FRACTAL_MANDELBROT, 0, 0);
    shift_view(&f, 0.5, 0.3);
    TEST_ASSERT_NEAR(f.shift_r, 0.5, 1e-10);
    TEST_ASSERT_NEAR(f.shift_i, 0.3, 1e-10);

    /* Multiple shifts accumulate. */
    shift_view(&f, 0.2, -0.1);
    TEST_ASSERT_NEAR(f.shift_r, 0.7, 1e-10);
    TEST_ASSERT_NEAR(f.shift_i, 0.2, 1e-10);

    /* Negative shifts work. */
    shift_view(&f, -1.0, -1.0);
    TEST_ASSERT_NEAR(f.shift_r, -0.3, 1e-10);
    TEST_ASSERT_NEAR(f.shift_i, -0.8, 1e-10);

    /* ═══════════════════════════════════════════════════════════
    ** SUITE: zoom_at (zoom toward cursor)
    ** ═══════════════════════════════════════════════════════════ */
    TEST_SUITE("zoom_at");

    test_init_fractal(&f, FRACTAL_MANDELBROT, 0, 0);

    /* Record the complex coordinate under a pixel. */
    double x = 450, y = 350;
    double before_r = screen_to_complex_r((int)x, &f);
    double before_i = screen_to_complex_i((int)y, &f);

    /* Zoom in toward that pixel. */
    zoom_at(&f, x, y, 0.5);

    /* The complex coordinate under that pixel should be the same. */
    double after_r = screen_to_complex_r((int)x, &f);
    double after_i = screen_to_complex_i((int)y, &f);
    TEST_ASSERT_NEAR(after_r, before_r, 1e-10);
    TEST_ASSERT_NEAR(after_i, before_i, 1e-10);

    /* Zoom was reduced (magnified). */
    TEST_ASSERT(f.zoom < 4.0);

    /* Zoom out: zoom factor > 1. */
    double z0 = f.zoom;
    zoom_at(&f, x, y, 2.0);
    TEST_ASSERT(f.zoom > z0);
    /* Point still fixed. */
    after_r = screen_to_complex_r((int)x, &f);
    after_i = screen_to_complex_i((int)y, &f);
    TEST_ASSERT_NEAR(after_r, before_r, 1e-10);
    TEST_ASSERT_NEAR(after_i, before_i, 1e-10);

    /* ═══════════════════════════════════════════════════════════
    ** SUITE: Zoom clamping
    ** ═══════════════════════════════════════════════════════════ */
    TEST_SUITE("Zoom clamping");

    test_init_fractal(&f, FRACTAL_MANDELBROT, 0, 0);

    /* Excessive zoom-in should clamp to 1e-12. */
    for (int i = 0; i < 100; i++)
        zoom_at(&f, WIDTH/2, HEIGHT/2, 0.1);
    TEST_ASSERT(f.zoom >= 1e-12);
    TEST_ASSERT(f.zoom <= 1e-11);

    /* Excessive zoom-out should clamp to 4.0. */
    test_init_fractal(&f, FRACTAL_MANDELBROT, 0, 0);
    for (int i = 0; i < 100; i++)
        zoom_at(&f, WIDTH/2, HEIGHT/2, 2.0);
    TEST_ASSERT(f.zoom <= 4.0);
    TEST_ASSERT(f.zoom >= 3.9);

    /* ═══════════════════════════════════════════════════════════
    ** SUITE: Aspect ratio
    ** ═══════════════════════════════════════════════════════════ */
    TEST_SUITE("Aspect ratio preservation");

    test_init_fractal(&f, FRACTAL_MANDELBROT, 0, 0);

    /* The real span and imag span should both equal zoom. */
    double real_span = screen_to_complex_r(WIDTH - 1, &f) - screen_to_complex_r(0, &f);
    double imag_span = screen_to_complex_i(0, &f) - screen_to_complex_i(HEIGHT - 1, &f);
    TEST_ASSERT_NEAR(real_span, 4.0, 0.1);
    TEST_ASSERT_NEAR(imag_span, 4.0, 0.1);

    return TEST_END();
}
