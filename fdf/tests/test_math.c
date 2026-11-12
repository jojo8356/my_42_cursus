/*
** test_math.c — Unit tests for 3D math: rotation and isometric projection.
**
** Verifies:
**   • radians() converts degrees to radians correctly
**   • rotate_x/y/z produce correct results for known angles
**   • 90° rotations swap/negate coordinates correctly
**   • 180° rotations negate coordinates correctly
**   • 360° rotations return to original position
**   • project_iso produces correct screen coordinates
**   • apply_transform centers the map correctly
*/

# include "fdf.h"
# include "test_framework.h"
# include <math.h>
# include <string.h>

int main(void)
{
    TEST_BEGIN();

    /* ═══════════════════════════════════════════════════════════
    ** SUITE: radians conversion
    ** ═══════════════════════════════════════════════════════════ */
    TEST_SUITE("radians conversion");

    TEST_ASSERT_NEAR(radians(0), 0.0, 1e-10);
    TEST_ASSERT_NEAR(radians(90), M_PI / 2.0, 1e-10);
    TEST_ASSERT_NEAR(radians(180), M_PI, 1e-10);
    TEST_ASSERT_NEAR(radians(360), 2.0 * M_PI, 1e-10);
    TEST_ASSERT_NEAR(radians(45), M_PI / 4.0, 1e-10);

    /* ═══════════════════════════════════════════════════════════
    ** SUITE: rotate_x
    ** ═══════════════════════════════════════════════════════════ */
    TEST_SUITE("rotate_x");

    /* Rotate (0, 1, 0) by 90° around X → (0, 0, 1). */
    double x = 0, y = 1, z = 0;
    rotate_x(&x, &y, &z, radians(90));
    TEST_ASSERT_NEAR(x, 0.0, 1e-10);
    TEST_ASSERT_NEAR(y, 0.0, 1e-10);
    TEST_ASSERT_NEAR(z, 1.0, 1e-10);

    /* Rotate (0, 1, 0) by 180° around X → (0, -1, 0). */
    x = 0; y = 1; z = 0;
    rotate_x(&x, &y, &z, radians(180));
    TEST_ASSERT_NEAR(x, 0.0, 1e-10);
    TEST_ASSERT_NEAR(y, -1.0, 1e-10);
    TEST_ASSERT_NEAR(z, 0.0, 1e-10);

    /* Rotate (0, 1, 0) by 360° → back to (0, 1, 0). */
    x = 0; y = 1; z = 0;
    rotate_x(&x, &y, &z, radians(360));
    TEST_ASSERT_NEAR(x, 0.0, 1e-10);
    TEST_ASSERT_NEAR(y, 1.0, 1e-10);
    TEST_ASSERT_NEAR(z, 0.0, 1e-10);

    /* Rotate (1, 2, 3) by 90° around X.
    ** y' = y*cos(90) - z*sin(90) = 0 - 3 = -3
    ** z' = y*sin(90) + z*cos(90) = 2 + 0 = 2
    ** x' = 1 (unchanged) */
    x = 1; y = 2; z = 3;
    rotate_x(&x, &y, &z, radians(90));
    TEST_ASSERT_NEAR(x, 1.0, 1e-10);
    TEST_ASSERT_NEAR(y, -3.0, 1e-10);
    TEST_ASSERT_NEAR(z, 2.0, 1e-10);

    /* ═══════════════════════════════════════════════════════════
    ** SUITE: rotate_y
    ** ═══════════════════════════════════════════════════════════ */
    TEST_SUITE("rotate_y");

    /* Rotate (1, 0, 0) by 90° around Y → (0, 0, -1). */
    x = 1; y = 0; z = 0;
    rotate_y(&x, &y, &z, radians(90));
    TEST_ASSERT_NEAR(x, 0.0, 1e-10);
    TEST_ASSERT_NEAR(y, 0.0, 1e-10);
    TEST_ASSERT_NEAR(z, -1.0, 1e-10);

    /* Rotate (1, 0, 0) by 180° around Y → (-1, 0, 0). */
    x = 1; y = 0; z = 0;
    rotate_y(&x, &y, &z, radians(180));
    TEST_ASSERT_NEAR(x, -1.0, 1e-10);
    TEST_ASSERT_NEAR(y, 0.0, 1e-10);
    TEST_ASSERT_NEAR(z, 0.0, 1e-10);

    /* Rotate (1, 0, 0) by 360° → back to (1, 0, 0). */
    x = 1; y = 0; z = 0;
    rotate_y(&x, &y, &z, radians(360));
    TEST_ASSERT_NEAR(x, 1.0, 1e-10);
    TEST_ASSERT_NEAR(y, 0.0, 1e-10);
    TEST_ASSERT_NEAR(z, 0.0, 1e-10);

    /* ═══════════════════════════════════════════════════════════
    ** SUITE: rotate_z
    ** ═══════════════════════════════════════════════════════════ */
    TEST_SUITE("rotate_z");

    /* Rotate (1, 0, 0) by 90° around Z → (0, 1, 0). */
    x = 1; y = 0; z = 0;
    rotate_z(&x, &y, &z, radians(90));
    TEST_ASSERT_NEAR(x, 0.0, 1e-10);
    TEST_ASSERT_NEAR(y, 1.0, 1e-10);
    TEST_ASSERT_NEAR(z, 0.0, 1e-10);

    /* Rotate (1, 0, 0) by 180° around Z → (-1, 0, 0). */
    x = 1; y = 0; z = 0;
    rotate_z(&x, &y, &z, radians(180));
    TEST_ASSERT_NEAR(x, -1.0, 1e-10);
    TEST_ASSERT_NEAR(y, 0.0, 1e-10);
    TEST_ASSERT_NEAR(z, 0.0, 1e-10);

    /* Rotate (1, 1, 0) by 90° around Z → (-1, 1, 0).
    ** x' = x*cos(90) - y*sin(90) = 0 - 1 = -1
    ** y' = x*sin(90) + y*cos(90) = 1 + 0 = 1 */
    x = 1; y = 1; z = 0;
    rotate_z(&x, &y, &z, radians(90));
    TEST_ASSERT_NEAR(x, -1.0, 1e-10);
    TEST_ASSERT_NEAR(y, 1.0, 1e-10);
    TEST_ASSERT_NEAR(z, 0.0, 1e-10);

    /* ═══════════════════════════════════════════════════════════
    ** SUITE: Combined rotations
    ** ═══════════════════════════════════════════════════════════ */
    TEST_SUITE("Combined rotations");

    /* Rotate 90° around X then 90° around Y should be different
    ** from rotating 90° around Y then 90° around X (non-commutative). */
    /* X then Y: */
    x = 1; y = 0; z = 0;
    rotate_x(&x, &y, &z, radians(90));
    rotate_y(&x, &y, &z, radians(90));
    double xy_x = x, xy_y = y, xy_z = z;

    /* Y then X: */
    x = 1; y = 0; z = 0;
    rotate_y(&x, &y, &z, radians(90));
    rotate_x(&x, &y, &z, radians(90));
    TEST_ASSERT(!(fabs(x - xy_x) < 1e-10 && fabs(y - xy_y) < 1e-10 &&
                  fabs(z - xy_z) < 1e-10));

    /* 360° around all axes → back to start. */
    x = 5; y = 7; z = 3;
    rotate_x(&x, &y, &z, radians(360));
    rotate_y(&x, &y, &z, radians(360));
    rotate_z(&x, &y, &z, radians(360));
    TEST_ASSERT_NEAR(x, 5.0, 1e-10);
    TEST_ASSERT_NEAR(y, 7.0, 1e-10);
    TEST_ASSERT_NEAR(z, 3.0, 1e-10);

    /* ═══════════════════════════════════════════════════════════
    ** SUITE: Isometric projection
    ** ═══════════════════════════════════════════════════════════ */
    TEST_SUITE("Isometric projection");

    double sx, sy;
    double iso_x = ISO_ANGLE_X;
    double iso_y = ISO_ANGLE_Y;
    double scale = 1.0;

    /* Project (0, 0, 0) → should be (0, 0). */
    project_iso(0, 0, 0, iso_x, iso_y, scale, &sx, &sy);
    TEST_ASSERT_NEAR(sx, 0.0, 1e-10);
    TEST_ASSERT_NEAR(sy, 0.0, 1e-10);

    /* Project (1, 0, 0) → positive x, symmetric y. */
    project_iso(1, 0, 0, iso_x, iso_y, scale, &sx, &sy);
    TEST_ASSERT(sx > 0);

    /* Project (0, 1, 0) → should give different result from (1, 0, 0). */
    double sx2, sy2;
    project_iso(0, 1, 0, iso_x, iso_y, scale, &sx2, &sy2);
    TEST_ASSERT(fabs(sx - sx2) > 0.01 || fabs(sy - sy2) > 0.01);

    /* Project (0, 0, 1) → higher z means smaller screen y (up). */
    double sx_low, sy_low, sx_high, sy_high;
    project_iso(0, 0, 0, iso_x, iso_y, scale, &sx_low, &sy_low);
    project_iso(0, 0, 10, iso_x, iso_y, scale, &sx_high, &sy_high);
    TEST_ASSERT(sy_high < sy_low);  /* higher z → smaller y (up on screen) */

    /* Scale affects output. */
    project_iso(1, 0, 0, iso_x, iso_y, 2.0, &sx, &sy);
    double sx_orig, sy_orig;
    project_iso(1, 0, 0, iso_x, iso_y, 1.0, &sx_orig, &sy_orig);
    TEST_ASSERT(fabs(sx - 2.0 * sx_orig) < 0.01);

    /* ═══════════════════════════════════════════════════════════
    ** SUITE: apply_transform
    ** ═══════════════════════════════════════════════════════════ */
    TEST_SUITE("apply_transform");

    t_env env;
    memset(&env, 0, sizeof(env));
    env.map = malloc(sizeof(t_map));
    memset(env.map, 0, sizeof(t_map));
    env.map->width = 3;
    env.map->height = 3;
    env.scale = 1.0;
    env.iso_x = ISO_ANGLE_X;
    env.iso_y = ISO_ANGLE_Y;

    /* Transform (0, 0, 0) — should be centered in window. */
    int tx, ty;
    apply_transform(&env, 0, 0, 0, &tx, &ty);
    TEST_ASSERT(tx > 0 && tx < WIN_W);
    TEST_ASSERT(ty > 0 && ty < WIN_H);

    /* Transform center of 3x3 map (1, 1, 0) — should be near center. */
    apply_transform(&env, 1, 1, 0, &tx, &ty);
    TEST_ASSERT(tx > WIN_W / 2 - 50 && tx < WIN_W / 2 + 50);
    TEST_ASSERT(ty > WIN_H / 2 - 50 && ty < WIN_H / 2 + 50);

    /* Transform corner (0, 0, 10) — higher z should be higher on screen. */
    apply_transform(&env, 0, 0, 0, &tx, &ty);
    int ty_low = ty;
    apply_transform(&env, 0, 0, 10, &tx, &ty);
    TEST_ASSERT(ty < ty_low);  /* higher z → smaller y (up) */

    /* Transform with offset. */
    env.offset_x = 100;
    apply_transform(&env, 0, 0, 0, &tx, &ty);
    int tx_no_offset;
    env.offset_x = 0;
    apply_transform(&env, 0, 0, 0, &tx_no_offset, &ty);
    TEST_ASSERT(tx == tx_no_offset + 100);

    /* Transform with rotation. */
    env.rot_y = radians(90);
    apply_transform(&env, 2, 0, 0, &tx, &ty);
    env.rot_y = 0;
    apply_transform(&env, 2, 0, 0, &tx_no_offset, &ty);
    TEST_ASSERT(fabs((double)tx - (double)tx_no_offset) > 1.0);

    free(env.map);

    return TEST_END();
}
