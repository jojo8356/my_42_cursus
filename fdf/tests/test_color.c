/*
** test_color.c — Unit tests for color computation and interpolation.
**
** Verifies:
**   • color_lerp with percent = 0 returns c1
**   • color_lerp with percent = 1 returns c2
**   • color_lerp with percent = 0.5 returns midpoint
**   • color_lerp handles edge cases (same colors, boundary values)
**   • compute_color maps heights correctly for each color mode
**   • cycle_color_mode cycles through all modes
**   • Color extraction (R, G, B channels) is correct
*/

# include "fdf.h"
# include "test_framework.h"
# include <string.h>
# include <math.h>

int main(void)
{
    TEST_BEGIN();

    t_env env;
    memset(&env, 0, sizeof(env));
    env.map = malloc(sizeof(t_map));
    memset(env.map, 0, sizeof(t_map));
    env.map->width = 3;
    env.map->height = 3;
    env.map->min_z = 0;
    env.map->max_z = 10;
    env.color_mode = MODE_GRADIENT;

    /* ═══════════════════════════════════════════════════════════
    ** SUITE: color_lerp
    ** ═══════════════════════════════════════════════════════════ */
    TEST_SUITE("color_lerp");

    /* percent = 0 → c1. */
    TEST_ASSERT_EQ(color_lerp(COLOR_RED, COLOR_GREEN, 0.0), COLOR_RED);
    TEST_ASSERT_EQ(color_lerp(0xFF0000, 0x00FF00, 0.0), 0xFF0000);

    /* percent = 1 → c2. */
    TEST_ASSERT_EQ(color_lerp(COLOR_RED, COLOR_GREEN, 1.0), COLOR_GREEN);
    TEST_ASSERT_EQ(color_lerp(0xFF0000, 0x00FF00, 1.0), 0x00FF00);

    /* percent = 0.5 → midpoint.
    ** Red (255,0,0) → Green (0,255,0): midpoint = (127, 127, 0). */
    int mid = color_lerp(COLOR_RED, COLOR_GREEN, 0.5);
    int r = (mid >> 16) & 0xFF;
    int g = (mid >> 8) & 0xFF;
    int b = mid & 0xFF;
    TEST_ASSERT(r == 127 || r == 128);   /* rounding */
    TEST_ASSERT(g == 127 || g == 128);
    TEST_ASSERT_EQ(b, 0);

    /* Same colors → same color regardless of percent. */
    TEST_ASSERT_EQ(color_lerp(COLOR_WHITE, COLOR_WHITE, 0.3), COLOR_WHITE);
    TEST_ASSERT_EQ(color_lerp(COLOR_BLUE, COLOR_BLUE, 0.7), COLOR_BLUE);

    /* Negative percent → c1. */
    TEST_ASSERT_EQ(color_lerp(COLOR_RED, COLOR_GREEN, -0.5), COLOR_RED);

    /* percent > 1 → c2. */
    TEST_ASSERT_EQ(color_lerp(COLOR_RED, COLOR_GREEN, 1.5), COLOR_GREEN);

    /* ═══════════════════════════════════════════════════════════
    ** SUITE: compute_color — gradient mode
    ** ═══════════════════════════════════════════════════════════ */
    TEST_SUITE("compute_color gradient");

    env.color_mode = MODE_GRADIENT;
    env.map->min_z = 0;
    env.map->max_z = 10;

    /* z = min_z → color_min (red). */
    int c_low = compute_color(0, &env);
    TEST_ASSERT_EQ(c_low, COLOR_RED);

    /* z = max_z → color_max (green). */
    int c_high = compute_color(10, &env);
    TEST_ASSERT_EQ(c_high, COLOR_GREEN);

    /* z = middle → midpoint. */
    int c_mid = compute_color(5, &env);
    int mr = (c_mid >> 16) & 0xFF;
    int mg = (c_mid >> 8) & 0xFF;
    int mb = c_mid & 0xFF;
    TEST_ASSERT(mr > 0 && mr < 255);
    TEST_ASSERT(mg > 0 && mg < 255);
    TEST_ASSERT_EQ(mb, 0);

    /* z outside range → clamped. */
    TEST_ASSERT_EQ(compute_color(-5, &env), COLOR_RED);
    TEST_ASSERT_EQ(compute_color(15, &env), COLOR_GREEN);

    /* ═══════════════════════════════════════════════════════════
    ** SUITE: compute_color — grayscale mode
    ** ═══════════════════════════════════════════════════════════ */
    TEST_SUITE("compute_color grayscale");

    env.color_mode = MODE_GRAYSCALE;

    /* z = min_z → white. */
    TEST_ASSERT_EQ(compute_color(0, &env), COLOR_WHITE);
    /* z = max_z → black. */
    TEST_ASSERT_EQ(compute_color(10, &env), COLOR_BLACK);

    /* z = middle → gray. */
    int gray = compute_color(5, &env);
    int gr = (gray >> 16) & 0xFF;
    int gg = (gray >> 8) & 0xFF;
    int gb = gray & 0xFF;
    TEST_ASSERT(gr > 0 && gr < 255);
    TEST_ASSERT_EQ(gr, gg);
    TEST_ASSERT_EQ(gg, gb);

    /* ═══════════════════════════════════════════════════════════
    ** SUITE: compute_color — blue mode
    ** ═══════════════════════════════════════════════════════════ */
    TEST_SUITE("compute_color blue");

    env.color_mode = MODE_BLUE;
    TEST_ASSERT_EQ(compute_color(0, &env), COLOR_BLUE);
    TEST_ASSERT_EQ(compute_color(10, &env), COLOR_WHITE);

    /* ═══════════════════════════════════════════════════════════
    ** SUITE: compute_color — cyan mode
    ** ═══════════════════════════════════════════════════════════ */
    TEST_SUITE("compute_color cyan");

    env.color_mode = MODE_CYAN;
    TEST_ASSERT_EQ(compute_color(0, &env), COLOR_CYAN);
    TEST_ASSERT_EQ(compute_color(10, &env), COLOR_WHITE);

    /* ═══════════════════════════════════════════════════════════
    ** SUITE: compute_color — rainbow mode
    ** ═══════════════════════════════════════════════════════════ */
    TEST_SUITE("compute_color rainbow");

    env.color_mode = MODE_RAINBOW;

    /* z = min_z → red. */
    int c_r = compute_color(0, &env);
    int rr = (c_r >> 16) & 0xFF;
    int rg = (c_r >> 8) & 0xFF;
    int rb = c_r & 0xFF;
    TEST_ASSERT(rr == 255 && rg == 0 && rb == 0);

    /* z = max_z → magenta (last rainbow color). */
    int c_m = compute_color(10, &env);
    int mr2 = (c_m >> 16) & 0xFF;
    int mg2 = (c_m >> 8) & 0xFF;
    int mb2 = c_m & 0xFF;
    TEST_ASSERT(mr2 > 0 && mg2 == 0 && mb2 > 0);

    /* z = middle → should be different from both ends. */
    int c_mid2 = compute_color(5, &env);
    TEST_ASSERT(c_mid2 != c_r);
    TEST_ASSERT(c_mid2 != c_m);

    /* ═══════════════════════════════════════════════════════════
    ** SUITE: compute_color — flat map (min == max)
    ** ═══════════════════════════════════════════════════════════ */
    TEST_SUITE("compute_color flat map");

    env.map->min_z = 5;
    env.map->max_z = 5;  /* flat */
    env.color_mode = MODE_GRADIENT;

    /* All z values should give the same color (color_min). */
    TEST_ASSERT_EQ(compute_color(5, &env), COLOR_RED);
    TEST_ASSERT_EQ(compute_color(0, &env), COLOR_RED);
    TEST_ASSERT_EQ(compute_color(10, &env), COLOR_RED);

    /* ═══════════════════════════════════════════════════════════
    ** SUITE: cycle_color_mode
    ** ═══════════════════════════════════════════════════════════ */
    TEST_SUITE("cycle_color_mode");

    env.color_mode = MODE_GRADIENT;
    cycle_color_mode(&env);
    TEST_ASSERT_EQ(env.color_mode, MODE_GRAYSCALE);

    cycle_color_mode(&env);
    TEST_ASSERT_EQ(env.color_mode, MODE_BLUE);

    cycle_color_mode(&env);
    TEST_ASSERT_EQ(env.color_mode, MODE_CYAN);

    cycle_color_mode(&env);
    TEST_ASSERT_EQ(env.color_mode, MODE_RAINBOW);

    /* Should wrap back to GRADIENT. */
    cycle_color_mode(&env);
    TEST_ASSERT_EQ(env.color_mode, MODE_GRADIENT);

    free(env.map);

    return TEST_END();
}
