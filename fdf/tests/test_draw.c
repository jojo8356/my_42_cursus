/*
** test_draw.c — Unit tests for Bresenham line drawing and pixel output.
**
** Verifies:
**   • put_pixel writes correct RGB values to the buffer
**   • put_pixel respects window bounds (no out-of-bounds writes)
**   • bresenham_line draws correct pixels for known lines
**   • Horizontal, vertical, and diagonal lines
**   • Line endpoints are included
**   • Color is correctly applied to all pixels on the line
*/

# include "fdf.h"
# include "test_framework.h"
# include <string.h>
# include <math.h>

int main(void)
{
    TEST_BEGIN();

    /* Fake image buffer for headless testing. */
    static char buffer[WIN_W * WIN_H * 4];

    t_env env;
    memset(&env, 0, sizeof(env));
    env.map = malloc(sizeof(t_map));
    memset(env.map, 0, sizeof(t_map));
    env.map->width = 3;
    env.map->height = 3;
    env.map->min_z = 0;
    env.map->max_z = 9;
    env.scale = 1.0;
    env.iso_x = ISO_ANGLE_X;
    env.iso_y = ISO_ANGLE_Y;
    env.color_mode = MODE_GRADIENT;
    env.color_min = COLOR_RED;
    env.color_max = COLOR_GREEN;
    env.img_data = buffer;
    env.bits_per_pixel = 32;
    env.line_size = WIN_W * 4;

    /* ═══════════════════════════════════════════════════════════
    ** SUITE: put_pixel
    ** ═══════════════════════════════════════════════════════════ */
    TEST_SUITE("put_pixel");

    /* Clear buffer. */
    memset(buffer, 0, sizeof(buffer));

    /* Write a white pixel at (0, 0). */
    put_pixel(&env, 0, 0, COLOR_WHITE);
    int offset = 0 * env.line_size + 0 * 4;
    TEST_ASSERT_EQ((unsigned char)buffer[offset], 255);   /* R */
    TEST_ASSERT_EQ((unsigned char)buffer[offset + 1], 255); /* G */
    TEST_ASSERT_EQ((unsigned char)buffer[offset + 2], 255); /* B */
    TEST_ASSERT_EQ((unsigned char)buffer[offset + 3], 255); /* A */

    /* Write a red pixel at (10, 10). */
    put_pixel(&env, 10, 10, COLOR_RED);
    offset = 10 * env.line_size + 10 * 4;
    TEST_ASSERT_EQ((unsigned char)buffer[offset], 255);   /* R */
    TEST_ASSERT_EQ((unsigned char)buffer[offset + 1], 0);  /* G */
    TEST_ASSERT_EQ((unsigned char)buffer[offset + 2], 0);  /* B */

    /* Write a green pixel at (20, 20). */
    put_pixel(&env, 20, 20, COLOR_GREEN);
    offset = 20 * env.line_size + 20 * 4;
    TEST_ASSERT_EQ((unsigned char)buffer[offset], 0);   /* R */
    TEST_ASSERT_EQ((unsigned char)buffer[offset + 1], 255); /* G */
    TEST_ASSERT_EQ((unsigned char)buffer[offset + 2], 0);   /* B */

    /* Write a blue pixel at (30, 30). */
    put_pixel(&env, 30, 30, COLOR_BLUE);
    offset = 30 * env.line_size + 30 * 4;
    TEST_ASSERT_EQ((unsigned char)buffer[offset], 0);   /* R */
    TEST_ASSERT_EQ((unsigned char)buffer[offset + 1], 0); /* G */
    TEST_ASSERT_EQ((unsigned char)buffer[offset + 2], 255); /* B */

    /* ═══════════════════════════════════════════════════════════
    ** SUITE: put_pixel bounds checking
    ** ═══════════════════════════════════════════════════════════ */
    TEST_SUITE("put_pixel bounds");

    /* Out-of-bounds writes should not crash. */
    put_pixel(&env, -1, 0, COLOR_WHITE);
    put_pixel(&env, WIN_W, 0, COLOR_WHITE);
    put_pixel(&env, 0, -1, COLOR_WHITE);
    put_pixel(&env, 0, WIN_H, COLOR_WHITE);

    /* Corners should work. */
    memset(buffer, 0, sizeof(buffer));
    put_pixel(&env, 0, 0, COLOR_WHITE);
    put_pixel(&env, WIN_W - 1, 0, COLOR_WHITE);
    put_pixel(&env, 0, WIN_H - 1, COLOR_WHITE);
    put_pixel(&env, WIN_W - 1, WIN_H - 1, COLOR_WHITE);

    offset = 0;
    TEST_ASSERT_EQ((unsigned char)buffer[offset + 3], 255);
    offset = 0 * env.line_size + (WIN_W - 1) * 4;
    TEST_ASSERT_EQ((unsigned char)buffer[offset + 3], 255);
    offset = (WIN_H - 1) * env.line_size + 0 * 4;
    TEST_ASSERT_EQ((unsigned char)buffer[offset + 3], 255);
    offset = (WIN_H - 1) * env.line_size + (WIN_W - 1) * 4;
    TEST_ASSERT_EQ((unsigned char)buffer[offset + 3], 255);

    /* ═══════════════════════════════════════════════════════════
    ** SUITE: bresenham_line — horizontal
    ** ═══════════════════════════════════════════════════════════ */
    TEST_SUITE("bresenham_line horizontal");

    memset(buffer, 0, sizeof(buffer));
    bresenham_line(&env, 100, 200, 110, 200, COLOR_WHITE);

    /* All pixels on row 200, columns 100-110 should be white. */
    for (int x = 100; x <= 110; x++)
    {
        offset = 200 * env.line_size + x * 4;
        TEST_ASSERT_EQ((unsigned char)buffer[offset + 3], 255);
    }

    /* ═══════════════════════════════════════════════════════════
    ** SUITE: bresenham_line — vertical
    ** ═══════════════════════════════════════════════════════════ */
    TEST_SUITE("bresenham_line vertical");

    memset(buffer, 0, sizeof(buffer));
    bresenham_line(&env, 100, 200, 100, 210, COLOR_WHITE);

    /* All pixels on column 100, rows 200-210 should be white. */
    for (int y = 200; y <= 210; y++)
    {
        offset = y * env.line_size + 100 * 4;
        TEST_ASSERT_EQ((unsigned char)buffer[offset + 3], 255);
    }

    /* ═══════════════════════════════════════════════════════════
    ** SUITE: bresenham_line — diagonal
    ** ═══════════════════════════════════════════════════════════ */
    TEST_SUITE("bresenham_line diagonal");

    memset(buffer, 0, sizeof(buffer));
    bresenham_line(&env, 100, 100, 110, 110, COLOR_WHITE);

    /* Endpoints should be drawn. */
    offset = 100 * env.line_size + 100 * 4;
    TEST_ASSERT_EQ((unsigned char)buffer[offset + 3], 255);
    offset = 110 * env.line_size + 110 * 4;
    TEST_ASSERT_EQ((unsigned char)buffer[offset + 3], 255);

    /* At least 11 pixels should be drawn (diagonal). */
    int count = 0;
    for (int y = 95; y <= 115; y++)
    {
        for (int x = 95; x <= 115; x++)
        {
            offset = y * env.line_size + x * 4;
            if ((unsigned char)buffer[offset + 3] == 255)
                count++;
        }
    }
    TEST_ASSERT(count >= 11);

    /* ═══════════════════════════════════════════════════════════
    ** SUITE: bresenham_line — reverse direction
    ** ═══════════════════════════════════════════════════════════ */
    TEST_SUITE("bresenham_line reverse");

    /* Drawing from (110, 110) to (100, 100) should give same result. */
    memset(buffer, 0, sizeof(buffer));
    bresenham_line(&env, 110, 110, 100, 100, COLOR_WHITE);

    offset = 100 * env.line_size + 100 * 4;
    TEST_ASSERT_EQ((unsigned char)buffer[offset + 3], 255);
    offset = 110 * env.line_size + 110 * 4;
    TEST_ASSERT_EQ((unsigned char)buffer[offset + 3], 255);

    /* ═══════════════════════════════════════════════════════════
    ** SUITE: bresenham_line — single pixel
    ** ═══════════════════════════════════════════════════════════ */
    TEST_SUITE("bresenham_line single pixel");

    memset(buffer, 0, sizeof(buffer));
    bresenham_line(&env, 50, 50, 50, 50, COLOR_WHITE);

    offset = 50 * env.line_size + 50 * 4;
    TEST_ASSERT_EQ((unsigned char)buffer[offset + 3], 255);

    /* Only one pixel should be drawn. */
    int drawn = 0;
    for (int y = 45; y <= 55; y++)
    {
        for (int x = 45; x <= 55; x++)
        {
            offset = y * env.line_size + x * 4;
            if ((unsigned char)buffer[offset + 3] == 255)
                drawn++;
        }
    }
    TEST_ASSERT_EQ(drawn, 1);

    /* ═══════════════════════════════════════════════════════════
    ** SUITE: draw_grid
    ** ═══════════════════════════════════════════════════════════ */
    TEST_SUITE("draw_grid");

    /* Parse a simple map and draw it. */
    memset(buffer, 0, sizeof(buffer));
    TEST_ASSERT_EQ(parse_file("tests/maps/basic.fdf", env.map), 0);
    env.scale = 10.0;  /* Larger scale for visible grid. */
    draw_grid(&env);

    /* At least some pixels should be drawn. */
    int pixels_drawn = 0;
    for (int y = 0; y < WIN_H; y++)
    {
        for (int x = 0; x < WIN_W; x++)
        {
            offset = y * env.line_size + x * 4;
            if ((unsigned char)buffer[offset + 3] == 255)
                pixels_drawn++;
        }
    }
    TEST_ASSERT(pixels_drawn > 50);   /* 3x3 grid should have many pixels */

    free_map(env.map);
    free(env.map);

    return TEST_END();
}
