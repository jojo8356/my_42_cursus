/*
** test_hooks.c — Unit tests for event handler logic (zoom, translate, rotate).
**
** Verifies:
**   • zoom clamps to [0.1, 10.0]
**   • zoom multiplies scale correctly
**   • translate updates offset_x and offset_y
**   • rotate updates rot_x, rot_y, rot_z per axis
**   • rotate with unknown axis does nothing
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
    env.scale = 1.0;
    env.iso_x = ISO_ANGLE_X;
    env.iso_y = ISO_ANGLE_Y;

    /* ═══════════════════════════════════════════════════════════
    ** SUITE: zoom
    ** ═══════════════════════════════════════════════════════════ */
    TEST_SUITE("zoom in");

    env.scale = 1.0;
    zoom(&env, 0.9);
    TEST_ASSERT(fabs(env.scale - 0.9) < 0.001);

    zoom(&env, 0.9);
    TEST_ASSERT(fabs(env.scale - 0.81) < 0.001);

    TEST_SUITE("zoom out");

    env.scale = 1.0;
    zoom(&env, 1.1);
    TEST_ASSERT(fabs(env.scale - 1.1) < 0.001);

    zoom(&env, 1.1);
    TEST_ASSERT(fabs(env.scale - 1.21) < 0.01);

    TEST_SUITE("zoom clamp min");

    env.scale = 0.1;
    zoom(&env, 0.1);
    TEST_ASSERT(env.scale >= 0.1);
    TEST_ASSERT(env.scale < 0.15);

    TEST_SUITE("zoom clamp max");

    env.scale = 10.0;
    zoom(&env, 1.1);
    TEST_ASSERT(env.scale <= 10.0);
    TEST_ASSERT(env.scale > 9.5);

    TEST_SUITE("zoom extreme chain");

    /* 100 zoom-ins should not go below 0.1. */
    env.scale = 1.0;
    for (int i = 0; i < 100; i++)
        zoom(&env, 0.5);
    TEST_ASSERT(env.scale >= 0.1);
    TEST_ASSERT(env.scale < 0.15);

    /* 100 zoom-outs should not exceed 10.0. */
    env.scale = 1.0;
    for (int i = 0; i < 100; i++)
        zoom(&env, 1.5);
    TEST_ASSERT(env.scale <= 10.0);
    TEST_ASSERT(env.scale > 9.0);

    /* ═══════════════════════════════════════════════════════════
    ** SUITE: translate
    ** ═══════════════════════════════════════════════════════════ */
    TEST_SUITE("translate x");

    env.offset_x = 0.0;
    env.offset_y = 0.0;
    translate(&env, 10.0, 0.0);
    TEST_ASSERT(fabs(env.offset_x - 10.0) < 0.001);
    TEST_ASSERT(fabs(env.offset_y) < 0.001);

    TEST_SUITE("translate y");

    translate(&env, 0.0, 20.0);
    TEST_ASSERT(fabs(env.offset_x - 10.0) < 0.001);
    TEST_ASSERT(fabs(env.offset_y - 20.0) < 0.001);

    TEST_SUITE("translate negative");

    translate(&env, -5.0, -10.0);
    TEST_ASSERT(fabs(env.offset_x - 5.0) < 0.001);
    TEST_ASSERT(fabs(env.offset_y - 10.0) < 0.001);

    TEST_SUITE("translate chain");

    env.offset_x = 0.0;
    env.offset_y = 0.0;
    for (int i = 0; i < 10; i++)
        translate(&env, 1.0, 1.0);
    TEST_ASSERT(fabs(env.offset_x - 10.0) < 0.001);
    TEST_ASSERT(fabs(env.offset_y - 10.0) < 0.001);

    /* ═══════════════════════════════════════════════════════════
    ** SUITE: rotate
    ** ═══════════════════════════════════════════════════════════ */
    TEST_SUITE("rotate x");

    env.rot_x = 0.0;
    env.rot_y = 0.0;
    env.rot_z = 0.0;
    rotate(&env, 'x', 0.05);
    TEST_ASSERT(fabs(env.rot_x - 0.05) < 0.001);
    TEST_ASSERT(fabs(env.rot_y) < 0.001);
    TEST_ASSERT(fabs(env.rot_z) < 0.001);

    TEST_SUITE("rotate y");

    rotate(&env, 'y', 0.05);
    TEST_ASSERT(fabs(env.rot_x - 0.05) < 0.001);
    TEST_ASSERT(fabs(env.rot_y - 0.05) < 0.001);
    TEST_ASSERT(fabs(env.rot_z) < 0.001);

    TEST_SUITE("rotate z");

    rotate(&env, 'z', 0.05);
    TEST_ASSERT(fabs(env.rot_x - 0.05) < 0.001);
    TEST_ASSERT(fabs(env.rot_y - 0.05) < 0.001);
    TEST_ASSERT(fabs(env.rot_z - 0.05) < 0.001);

    TEST_SUITE("rotate negative");

    rotate(&env, 'x', -0.1);
    TEST_ASSERT(fabs(env.rot_x - (-0.05)) < 0.001);

    TEST_SUITE("rotate unknown axis");

    double old_x = env.rot_x;
    double old_y = env.rot_y;
    double old_z = env.rot_z;
    rotate(&env, 'q', 0.05);
    TEST_ASSERT(fabs(env.rot_x - old_x) < 0.001);
    TEST_ASSERT(fabs(env.rot_y - old_y) < 0.001);
    TEST_ASSERT(fabs(env.rot_z - old_z) < 0.001);

    TEST_SUITE("rotate chain");

    env.rot_x = 0.0;
    env.rot_y = 0.0;
    env.rot_z = 0.0;
    for (int i = 0; i < 20; i++)
        rotate(&env, 'y', 0.1);
    TEST_ASSERT(fabs(env.rot_x) < 0.001);
    TEST_ASSERT(fabs(env.rot_y - 2.0) < 0.01);
    TEST_ASSERT(fabs(env.rot_z) < 0.001);

    free(env.map);

    return TEST_END();
}
