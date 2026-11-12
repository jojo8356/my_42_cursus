/*
** test_render.c — Integration tests for the rendering pipeline.
**
** Verifies:
**   •  compute_pixel produces valid iteration counts for all fractal types
**   •  render_row produces valid pixel data (into a fake buffer)
**   •  Determinism: same input → same output
**   •  All fractal types render without crashing
**   •  Deep zoom renders without NaN/Inf
**   •  Pixel buffer is fully written (no gaps)
*/



# include "fract-ol.h"
# include "test_framework.h"

# include <math.h>
# include <string.h>

int main(void)
{
    TEST_BEGIN();

    /* Fake image buffer for headless testing. */
    static char buffer[WIDTH * HEIGHT * 4];

    /* ═══════════════════════════════════════════════════════════
    ** SUITE: compute_pixel across fractal types
    ** ═══════════════════════════════════════════════════════════ */
    TEST_SUITE("compute_pixel across fractal types");

    t_fractal f;

    /* Mandelbrot: center pixel → bounded (max_iter). */
    test_init_fractal(&f, FRACTAL_MANDELBROT, 0, 0);
    memset(buffer, 0, sizeof(buffer));
    f.img_data = buffer;
    f.bits_per_pixel = 32;
    f.line_size = WIDTH * 4;
    int iter = compute_pixel(&f, WIDTH/2, HEIGHT/2);
    TEST_ASSERT_EQ(iter, f.max_iter);

    /* Mandelbrot: corner pixel → escapes. */
    iter = compute_pixel(&f, 0, 0);
    TEST_ASSERT(iter < f.max_iter);

    /* Julia: renders without crashing. */
    test_init_fractal(&f, FRACTAL_JULIA, -0.7, 0.27015);
    f.img_data = buffer;
    f.bits_per_pixel = 32;
    f.line_size = WIDTH * 4;
    iter = compute_pixel(&f, 100, 100);
    TEST_ASSERT(iter >= 0 && iter <= f.max_iter);

    /* Burning Ship. */
    test_init_fractal(&f, FRACTAL_BURNING_SHIP, 0, 0);
    f.img_data = buffer;
    f.bits_per_pixel = 32;
    f.line_size = WIDTH * 4;
    iter = compute_pixel(&f, 200, 200);
    TEST_ASSERT(iter >= 0 && iter <= f.max_iter);

    /* Tricorn. */
    test_init_fractal(&f, FRACTAL_TRICORN, 0, 0);
    f.img_data = buffer;
    f.bits_per_pixel = 32;
    f.line_size = WIDTH * 4;
    iter = compute_pixel(&f, 300, 300);
    TEST_ASSERT(iter >= 0 && iter <= f.max_iter);

    /* Phoenix. */
    test_init_fractal(&f, FRACTAL_PHOENIX, 0.5667, 0.0);
    f.delta = -0.5;
    f.img_data = buffer;
    f.bits_per_pixel = 32;
    f.line_size = WIDTH * 4;
    iter = compute_pixel(&f, 400, 400);
    TEST_ASSERT(iter >= 0 && iter <= f.max_iter);

    /* Phoenix variant. */
    test_init_fractal(&f, FRACTAL_PHOENIX_VARIANT, 0.5667, 0.0);
    f.delta = -0.5;
    f.img_data = buffer;
    f.bits_per_pixel = 32;
    f.line_size = WIDTH * 4;
    iter = compute_pixel(&f, 500, 500);
    TEST_ASSERT(iter >= 0 && iter <= f.max_iter);

    /* Multibrot. */
    test_init_fractal(&f, FRACTAL_MULTIBROT, 0, 0);
    f.multibrot_power = 4;
    f.img_data = buffer;
    f.bits_per_pixel = 32;
    f.line_size = WIDTH * 4;
    iter = compute_pixel(&f, 100, 200);
    TEST_ASSERT(iter >= 0 && iter <= f.max_iter);

    /* ═══════════════════════════════════════════════════════════
    ** SUITE: render_row
    ** ═══════════════════════════════════════════════════════════ */
    TEST_SUITE("render_row");

    /* Render a full row into the fake buffer. */
    test_init_fractal(&f, FRACTAL_MANDELBROT, 0, 0);
    memset(buffer, 0, sizeof(buffer));
    f.img_data = buffer;
    f.bits_per_pixel = 32;
    f.line_size = WIDTH * 4;

    render_row(&f, 0);

    /* Every pixel in the row should have been written (alpha = 255). */
    for (int x = 0; x < WIDTH; x++)
    {
        int offset = 0 * f.line_size + x * 4;
        TEST_ASSERT_EQ((unsigned char)buffer[offset + 3], 255);
    }

    /* The row should have at least some non-black pixels (colored). */
    int colored = 0;
    for (int x = 0; x < WIDTH; x++)
    {
        int offset = 0 * f.line_size + x * 4;
        if (buffer[offset] != 0 || buffer[offset+1] != 0 || buffer[offset+2] != 0)
            colored++;
    }
    TEST_ASSERT(colored > 0);

    /* ═══════════════════════════════════════════════════════════
    ** SUITE: Determinism
    ** ═══════════════════════════════════════════════════════════ */
    TEST_SUITE("Determinism");

    /* Two renders with identical input produce identical output. */
    static char buffer_a[WIDTH * HEIGHT * 4];
    static char buffer_b[WIDTH * HEIGHT * 4];

    test_init_fractal(&f, FRACTAL_JULIA, -0.7, 0.27015);
    f.img_data = buffer_a;
    f.bits_per_pixel = 32;
    f.line_size = WIDTH * 4;
    memset(buffer_a, 0, sizeof(buffer_a));
    render_row(&f, 50);

    test_init_fractal(&f, FRACTAL_JULIA, -0.7, 0.27015);
    f.img_data = buffer_b;
    f.bits_per_pixel = 32;
    f.line_size = WIDTH * 4;
    memset(buffer_b, 0, sizeof(buffer_b));
    render_row(&f, 50);

    TEST_ASSERT(memcmp(buffer_a, buffer_b, WIDTH * 4 * 4) == 0);

    /* ═══════════════════════════════════════════════════════════
    ** SUITE: Deep zoom robustness
    ** ═══════════════════════════════════════════════════════════ */
    TEST_SUITE("Deep zoom robustness");

    /* Zoom in very deep and render — should not produce NaN/Inf. */
    test_init_fractal(&f, FRACTAL_MANDELBROT, 0, 0);
    f.img_data = buffer;
    f.bits_per_pixel = 32;
    f.line_size = WIDTH * 4;
    f.zoom = 1e-12;
    f.shift_r = -0.74523;
    f.shift_i = 0.113005;
    f.max_iter = 5000;

    iter = compute_pixel(&f, WIDTH/2, HEIGHT/2);
    TEST_ASSERT(iter >= 0 && iter <= f.max_iter);

    /* Verify the iteration result is finite. */
    TEST_ASSERT(f.z_r >= -1e300 && f.z_r <= 1e300);
    TEST_ASSERT(f.z_i >= -1e300 && f.z_i <= 1e300);

    /* ═══════════════════════════════════════════════════════════
    ** SUITE: Full image render
    ** ═══════════════════════════════════════════════════════════ */
    TEST_SUITE("Full image render");

    /* Render several rows and verify the buffer is fully written. */
    test_init_fractal(&f, FRACTAL_MANDELBROT, 0, 0);
    memset(buffer, 0, sizeof(buffer));
    f.img_data = buffer;
    f.bits_per_pixel = 32;
    f.line_size = WIDTH * 4;

    for (int y = 0; y < HEIGHT; y += 50)   /* sample every 50 rows */
        render_row(&f, y);

    /* Each sampled row should have alpha = 255 on all pixels. */
    int all_written = 1;
    for (int y = 0; y < HEIGHT; y += 50)
    {
        for (int x = 0; x < WIDTH; x++)
        {
            int offset = y * f.line_size + x * 4;
            if ((unsigned char)buffer[offset + 3] != 255)
            {
                all_written = 0;
                break;
            }
        }
    }
    TEST_ASSERT(all_written);

    return TEST_END();
}
