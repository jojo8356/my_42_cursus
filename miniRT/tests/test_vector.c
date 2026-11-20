/* test_vector.c — suite de tests des opérations vecteurs 3D. */
# include "test_framework.h"
# include "minrt.h"
# include <math.h>

int    main(void)
{
    t_vec3    a, b, c;
    t_vec3    r;

    TEST_BEGIN();
    TEST_SUITE("vector");

    a = vec3_new(1.0, 2.0, 3.0);
    b = vec3_new(4.0, 5.0, 6.0);

    r = vec3_add(a, b);
    TEST_ASSERT_NEAR(r.x, 5.0, 1e-9);
    TEST_ASSERT_NEAR(r.y, 7.0, 1e-9);
    TEST_ASSERT_NEAR(r.z, 9.0, 1e-9);

    r = vec3_sub(b, a);
    TEST_ASSERT_NEAR(r.x, 3.0, 1e-9);
    TEST_ASSERT_NEAR(r.y, 3.0, 1e-9);
    TEST_ASSERT_NEAR(r.z, 3.0, 1e-9);

    r = vec3_scale(a, 2.0);
    TEST_ASSERT_NEAR(r.x, 2.0, 1e-9);
    TEST_ASSERT_NEAR(r.y, 4.0, 1e-9);
    TEST_ASSERT_NEAR(r.z, 6.0, 1e-9);

    r = vec3_neg(a);
    TEST_ASSERT_NEAR(r.x, -1.0, 1e-9);
    TEST_ASSERT_NEAR(r.y, -2.0, 1e-9);
    TEST_ASSERT_NEAR(r.z, -3.0, 1e-9);

    TEST_ASSERT_NEAR(vec3_dot(a, b), 32.0, 1e-9);
    TEST_ASSERT_NEAR(vec3_dot(a, a), 14.0, 1e-9);

    c = vec3_cross(a, b);
    TEST_ASSERT_NEAR(c.x, 2.0 * 6.0 - 3.0 * 5.0, 1e-9);
    TEST_ASSERT_NEAR(c.y, 3.0 * 4.0 - 1.0 * 6.0, 1e-9);
    TEST_ASSERT_NEAR(c.z, 1.0 * 5.0 - 2.0 * 4.0, 1e-9);

    TEST_ASSERT_NEAR(vec3_length_sq(a), 14.0, 1e-9);
    TEST_ASSERT_NEAR(vec3_length(a), sqrt(14.0), 1e-9);

    r = vec3_normalize(vec3_new(3.0, 4.0, 0.0));
    TEST_ASSERT_NEAR(r.x, 0.6, 1e-9);
    TEST_ASSERT_NEAR(r.y, 0.8, 1e-9);
    TEST_ASSERT_NEAR(r.z, 0.0, 1e-9);

    r = vec3_normalize(vec3_new(0.0, 0.0, 0.0));
    TEST_ASSERT_NEAR(r.x, 0.0, 1e-9);
    TEST_ASSERT_NEAR(r.y, 0.0, 1e-9);
    TEST_ASSERT_NEAR(r.z, 0.0, 1e-9);

    /* reflect : i=(0,-1,0) contre n=(0,1,0) -> (0,1,0) */
    r = vec3_reflect(vec3_new(0.0, -1.0, 0.0), vec3_new(0.0, 1.0, 0.0));
    TEST_ASSERT_NEAR(r.x, 0.0, 1e-9);
    TEST_ASSERT_NEAR(r.y, 1.0, 1e-9);
    TEST_ASSERT_NEAR(r.z, 0.0, 1e-9);

    /* refract : entrant perpendiculairement, le rayon continue (0,-1,0) */
    r = vec3_refract(vec3_new(0.0, -1.0, 0.0), vec3_new(0.0, 1.0, 0.0), 1.0 / 1.5);
    TEST_ASSERT(r.x == 0.0);
    TEST_ASSERT(r.z == 0.0);
    TEST_ASSERT_NEAR(r.y, -1.0, 1e-9);

    TEST_ASSERT_NEAR(vec3_dist(vec3_new(0.0, 0.0, 0.0), vec3_new(3.0, 4.0, 0.0)),
        5.0, 1e-9);

    r = vec3_lerp(vec3_new(0.0, 0.0, 0.0), vec3_new(10.0, 10.0, 10.0), 0.5);
    TEST_ASSERT_NEAR(r.x, 5.0, 1e-9);
    TEST_ASSERT_NEAR(r.y, 5.0, 1e-9);
    TEST_ASSERT_NEAR(r.z, 5.0, 1e-9);

    r = vec3_min(vec3_new(1.0, 5.0, 3.0), vec3_new(2.0, 4.0, 6.0));
    TEST_ASSERT_NEAR(r.x, 1.0, 1e-9);
    TEST_ASSERT_NEAR(r.y, 4.0, 1e-9);
    TEST_ASSERT_NEAR(r.z, 3.0, 1e-9);

    r = vec3_max(vec3_new(1.0, 5.0, 3.0), vec3_new(2.0, 4.0, 6.0));
    TEST_ASSERT_NEAR(r.x, 2.0, 1e-9);
    TEST_ASSERT_NEAR(r.y, 5.0, 1e-9);
    TEST_ASSERT_NEAR(r.z, 6.0, 1e-9);

    TEST_END();
}
