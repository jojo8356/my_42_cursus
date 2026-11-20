/* test_matrix.c — suite de tests des matrices 4x4. */
# include "test_framework.h"
# include "minrt.h"
# include <math.h>

static int    near(double a, double b)
{
    return (fabs(a - b) < 1e-9);
}

int    main(void)
{
    t_mat4    id;
    t_mat4    m;
    t_mat4    inv;
    t_mat4    t;
    t_mat4    r;
    t_mat4    s;
    t_vec3    p;
    t_vec3    p2;

    TEST_BEGIN();
    TEST_SUITE("matrix");

    id = mat4_identity();
    TEST_ASSERT(near(id.m[0][0], 1.0));
    TEST_ASSERT(near(id.m[0][1], 0.0));
    TEST_ASSERT(near(id.m[3][3], 1.0));

    /* A * I = A */
    m = mat4_translate(2.0, 3.0, 4.0);
    {
        t_mat4    prod = mat4_multiply(m, id);
        TEST_ASSERT(near(prod.m[0][3], 2.0));
        TEST_ASSERT(near(prod.m[1][3], 3.0));
        TEST_ASSERT(near(prod.m[2][3], 4.0));
    }

    /* translation d'un point */
    p = mat4_transform_point(mat4_translate(1.0, 2.0, 3.0),
        vec3_new(1.0, 1.0, 1.0));
    TEST_ASSERT_NEAR(p.x, 2.0, 1e-9);
    TEST_ASSERT_NEAR(p.y, 3.0, 1e-9);
    TEST_ASSERT_NEAR(p.z, 4.0, 1e-9);

    /* rotation Y de 90 deg : (1,0,0) -> (0,0,-1) */
    r = mat4_rotate_y(90.0);
    p = mat4_transform_dir(r, vec3_new(1.0, 0.0, 0.0));
    TEST_ASSERT_NEAR(p.x, 0.0, 1e-9);
    TEST_ASSERT_NEAR(p.y, 0.0, 1e-9);
    TEST_ASSERT_NEAR(p.z, -1.0, 1e-9);

    /* rotation X de 90 deg : (0,1,0) -> (0,0,1) */
    r = mat4_rotate_x(90.0);
    p = mat4_transform_dir(r, vec3_new(0.0, 1.0, 0.0));
    TEST_ASSERT_NEAR(p.x, 0.0, 1e-9);
    TEST_ASSERT_NEAR(p.y, 0.0, 1e-9);
    TEST_ASSERT_NEAR(p.z, 1.0, 1e-9);

    /* rotation Z de 90 deg : (1,0,0) -> (0,1,0) */
    r = mat4_rotate_z(90.0);
    p = mat4_transform_dir(r, vec3_new(1.0, 0.0, 0.0));
    TEST_ASSERT_NEAR(p.x, 0.0, 1e-9);
    TEST_ASSERT_NEAR(p.y, 1.0, 1e-9);
    TEST_ASSERT_NEAR(p.z, 0.0, 1e-9);

    /* Rodrigues : rotation de 180 deg autour de (0,1,0) : (1,0,0) -> (-1,0,0) */
    r = mat4_rotation_axis(vec3_new(0.0, 1.0, 0.0), 180.0);
    p = mat4_transform_dir(r, vec3_new(1.0, 0.0, 0.0));
    TEST_ASSERT_NEAR(p.x, -1.0, 1e-9);
    TEST_ASSERT_NEAR(p.y, 0.0, 1e-9);
    TEST_ASSERT_NEAR(p.z, 0.0, 1e-9);

    /* Rodrigues : 90 deg autour de (0,0,1) : (1,0,0) -> (0,1,0) */
    r = mat4_rotation_axis(vec3_new(0.0, 0.0, 1.0), 90.0);
    p = mat4_transform_dir(r, vec3_new(1.0, 0.0, 0.0));
    TEST_ASSERT_NEAR(p.x, 0.0, 1e-9);
    TEST_ASSERT_NEAR(p.y, 1.0, 1e-9);
    TEST_ASSERT_NEAR(p.z, 0.0, 1e-9);

    /* scale */
    s = mat4_scale(2.0, 3.0, 4.0);
    p = mat4_transform_point(s, vec3_new(1.0, 1.0, 1.0));
    TEST_ASSERT_NEAR(p.x, 2.0, 1e-9);
    TEST_ASSERT_NEAR(p.y, 3.0, 1e-9);
    TEST_ASSERT_NEAR(p.z, 4.0, 1e-9);

    /* inversion : T * T^-1 = I */
    t = mat4_translate(5.0, -2.0, 7.0);
    TEST_ASSERT(mat4_invert(t, &inv) == 1);
    {
        t_mat4    prod = mat4_multiply(t, inv);
        TEST_ASSERT(near(prod.m[0][0], 1.0));
        TEST_ASSERT(near(prod.m[1][1], 1.0));
        TEST_ASSERT(near(prod.m[2][2], 1.0));
        TEST_ASSERT(near(prod.m[3][3], 1.0));
        TEST_ASSERT(near(prod.m[0][3], 0.0));
        TEST_ASSERT(near(prod.m[1][3], 0.0));
    }

    /* inversion : transformer un point par T puis T^-1 revient au point */
    p = vec3_new(1.0, 2.0, 3.0);
    p2 = mat4_transform_point(inv, mat4_transform_point(t, p));
    TEST_ASSERT_NEAR(p2.x, p.x, 1e-9);
    TEST_ASSERT_NEAR(p2.y, p.y, 1e-9);
    TEST_ASSERT_NEAR(p2.z, p.z, 1e-9);

    /* inversion de la rotation : R * R^-1 = I */
    r = mat4_rotate_y(33.0);
    TEST_ASSERT(mat4_invert(r, &inv) == 1);
    {
        t_mat4    prod = mat4_multiply(r, inv);
        TEST_ASSERT(near(prod.m[0][0], 1.0));
        TEST_ASSERT(near(prod.m[1][1], 1.0));
        TEST_ASSERT(near(prod.m[2][2], 1.0));
    }

    /* matrice singulière -> inversion refusee */
    {
        t_mat4    singular = mat4_identity();
        singular.m[0][0] = 0.0;
        singular.m[1][0] = 0.0;
        singular.m[2][0] = 0.0;
        TEST_ASSERT(mat4_invert(singular, &inv) == 0);
    }

    /* transpose */
    m = mat4_translate(1.0, 2.0, 3.0);
    {
        t_mat4    tr = mat4_transpose(m);
        TEST_ASSERT(near(tr.m[0][3], m.m[3][0]));
        TEST_ASSERT(near(tr.m[3][0], m.m[0][3]));
    }

    TEST_END();
}
