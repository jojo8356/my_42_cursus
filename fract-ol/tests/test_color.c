/*
** test_color.c — Unit tests for color palette and pixel-color computation.
**
** Verifies:
**   •  Points inside the set → black
**   •  Colored points have valid RGB ranges (0–255)
**   •  Palette cycling changes the color
**   •  Grayscale mode produces equal R, G, B
**   •  Color shift offsets the palette index
**   •  No undefined behavior at palette boundaries
*/



# include "fract-ol.h"
# include "test_framework.h"

# include <math.h>

int main(void)
{
    TEST_BEGIN();

    /* ═══════════════════════════════════════════════════════════
    ** SUITE: Color mapping
    ** ═══════════════════════════════════════════════════════════ */
    TEST_SUITE("Color mapping");

    t_fractal f;
    test_init_fractal(&f, FRACTAL_MANDELBROT, 0, 0);

    /* Point inside the set (iter == max_iter) → black. */
    t_color c = get_color(f.max_iter, &f);
    TEST_ASSERT_EQ(c.r, 0);
    TEST_ASSERT_EQ(c.g, 0);
    TEST_ASSERT_EQ(c.b, 0);

    /* Colored point → valid RGB range. */
    c = get_color(100, &f);
    TEST_ASSERT(c.r >= 0 && c.r <= 255);
    TEST_ASSERT(c.g >= 0 && c.g <= 255);
    TEST_ASSERT(c.b >= 0 && c.b <= 255);

    /* Different iteration counts → (usually) different colors.
    ** Not guaranteed for all palettes, but the rainbow palette should vary. */
    t_color c1 = get_color(10, &f);
    t_color c2 = get_color(100, &f);
    int same = (c1.r == c2.r && c1.g == c2.g && c1.b == c2.b);
    TEST_ASSERT(!same);

    /* Color at iter 0 is a valid color. */
    c = get_color(0, &f);
    TEST_ASSERT(c.r >= 0 && c.r <= 255);

    /* ═══════════════════════════════════════════════════════════
    ** SUITE: Color shift
    ** ═══════════════════════════════════════════════════════════ */
    TEST_SUITE("Color shift");

    /* Shifting the color offset changes the resulting color. */
    test_init_fractal(&f, FRACTAL_MANDELBROT, 0, 0);
    t_color base = get_color(50, &f);

    f.color_shift = 100;
    t_color shifted = get_color(50, &f);
    int shifted_same = (base.r == shifted.r && base.g == shifted.g && base.b == shifted.b);
    TEST_ASSERT(!shifted_same);

    /* Shift wraps around at 256. */
    f.color_shift = 0;
    t_color c_a = get_color(100, &f);
    f.color_shift = 256;
    t_color c_b = get_color(100, &f);
    TEST_ASSERT_EQ(c_a.r, c_b.r);
    TEST_ASSERT_EQ(c_a.g, c_b.g);
    TEST_ASSERT_EQ(c_a.b, c_b.b);

    /* Negative shift wraps correctly. */
    f.color_shift = -1;
    c_b = get_color(100, &f);
    /* -1 should give the same as 255. */
    f.color_shift = 255;
    t_color c_c = get_color(100, &f);
    TEST_ASSERT_EQ(c_b.r, c_c.r);
    TEST_ASSERT_EQ(c_b.g, c_c.g);
    TEST_ASSERT_EQ(c_b.b, c_c.b);

    /* ═══════════════════════════════════════════════════════════
    ** SUITE: Grayscale
    ** ═══════════════════════════════════════════════════════════ */
    TEST_SUITE("Grayscale mode");

    test_init_fractal(&f, FRACTAL_MANDELBROT, 0, 0);
    f.grayscale = 1;

    /* Grayscale: R == G == B. */
    c = get_color(200, &f);
    TEST_ASSERT_EQ(c.r, c.g);
    TEST_ASSERT_EQ(c.g, c.b);

    /* Grayscale: value proportional to iteration. */
    t_color c_low = get_color(100, &f);
    t_color c_high = get_color(500, &f);
    TEST_ASSERT(c_high.r >= c_low.r);

    /* Grayscale: inside the set is still black. */
    c = get_color(f.max_iter, &f);
    TEST_ASSERT_EQ(c.r, 0);
    TEST_ASSERT_EQ(c.g, 0);
    TEST_ASSERT_EQ(c.b, 0);

    /* ═══════════════════════════════════════════════════════════
    ** SUITE: Palette cycling
    ** ═══════════════════════════════════════════════════════════ */
    TEST_SUITE("Palette cycling");

    /* Different palettes produce different colors. */
    test_init_fractal(&f, FRACTAL_MANDELBROT, 0, 0);
    f.palette = 0;
    t_color p0 = get_color(128, &f);

    f.palette = 1;
    t_color p1 = get_color(128, &f);

    /* Palettes 1–5 are procedurally generated with different hue offsets,
    ** so they should differ from palette 0. */
    int diff = (p0.r != p1.r || p0.g != p1.g || p0.b != p1.b);
    TEST_ASSERT(diff);

    /* ═══════════════════════════════════════════════════════════
    ** SUITE: put_pixel bounds checking
    ** ═══════════════════════════════════════════════════════════ */
    TEST_SUITE("put_pixel bounds");

    /* put_pixel with out-of-bounds coordinates should not crash.
    ** We can't easily test the buffer content without mlx, but we verify
    ** it doesn't write out of bounds by checking it doesn't crash. */
    test_init_fractal(&f, FRACTAL_MANDELBROT, 0, 0);
    /* Set up a fake image buffer. */
    static char buffer[WIDTH * HEIGHT * 4];
    f.img_data = buffer;
    f.bits_per_pixel = 32;
    f.line_size = WIDTH * 4;

    t_color test_color = { 255, 128, 64 };
    put_pixel(&f, 0, 0, test_color);
    put_pixel(&f, WIDTH - 1, HEIGHT - 1, test_color);
    put_pixel(&f, -1, 0, test_color);    /* out of bounds — should not crash */
    put_pixel(&f, WIDTH, 0, test_color); /* out of bounds — should not crash */
    put_pixel(&f, 0, -1, test_color);    /* out of bounds — should not crash */
    put_pixel(&f, 0, HEIGHT, test_color); /* out of bounds — should not crash */

    /* Verify the first pixel was written correctly. */
    TEST_ASSERT_EQ((unsigned char)buffer[0], 255);
    TEST_ASSERT_EQ((unsigned char)buffer[1], 128);
    TEST_ASSERT_EQ((unsigned char)buffer[2], 64);
    TEST_ASSERT_EQ((unsigned char)buffer[3], 255);

    /* Verify the last pixel was written correctly. */
    int last_offset = (HEIGHT - 1) * f.line_size + (WIDTH - 1) * 4;
    TEST_ASSERT_EQ((unsigned char)buffer[last_offset], 255);
    TEST_ASSERT_EQ((unsigned char)buffer[last_offset + 1], 128);
    TEST_ASSERT_EQ((unsigned char)buffer[last_offset + 2], 64);

    return TEST_END();
}
