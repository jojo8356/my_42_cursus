/* test_camera.c — suite de tests de la caméra. */
# include "test_framework.h"
# include "minrt.h"
# include <math.h>

int    main(void)
{
    t_camera    cam;
    t_ray    ray;

    TEST_BEGIN();
    TEST_SUITE("camera");

    camera_init(&cam, vec3_new(0.0, 0.0, -5.0), vec3_new(0.0, 0.0, 1.0),
        60.0, 800, 600);

    /* la base est orthonormée */
    TEST_ASSERT_NEAR(vec3_length(cam.forward), 1.0, 1e-9);
    TEST_ASSERT_NEAR(vec3_length(cam.right), 1.0, 1e-9);
    TEST_ASSERT_NEAR(vec3_length(cam.up), 1.0, 1e-9);
    TEST_ASSERT_NEAR(vec3_dot(cam.forward, cam.right), 0.0, 1e-9);
    TEST_ASSERT_NEAR(vec3_dot(cam.forward, cam.up), 0.0, 1e-9);
    TEST_ASSERT_NEAR(vec3_dot(cam.right, cam.up), 0.0, 1e-9);

    /* le rayon central pointe selon forward */
    ray = camera_ray(&cam, 400, 300);
    TEST_ASSERT_NEAR(ray.origin.x, 0.0, 1e-9);
    TEST_ASSERT_NEAR(ray.origin.y, 0.0, 1e-9);
    TEST_ASSERT_NEAR(ray.origin.z, -5.0, 1e-9);
    TEST_ASSERT_NEAR(ray.dir.x, 0.0, 0.002);
    TEST_ASSERT_NEAR(ray.dir.y, 0.0, 0.002);
    TEST_ASSERT_NEAR(ray.dir.z, 1.0, 1e-6);

    /* le rayon est normalisé */
    TEST_ASSERT_NEAR(vec3_length(ray.dir), 1.0, 1e-9);

    /* le coin supérieur-gauche pointe vers le haut et la gauche (+x) */
    ray = camera_ray(&cam, 0, 0);
    TEST_ASSERT(ray.dir.x > 0.0);
    TEST_ASSERT(ray.dir.y > 0.0);
    TEST_ASSERT(ray.dir.z > 0.0);

    /* le coin inférieur-droit pointe vers le bas et la droite (-x) */
    ray = camera_ray(&cam, 799, 599);
    TEST_ASSERT(ray.dir.x < 0.0);
    TEST_ASSERT(ray.dir.y < 0.0);
    TEST_ASSERT(ray.dir.z > 0.0);

    /* fov plus grand -> rayon plus évasé (angle plus grand) */
    camera_init(&cam, vec3_new(0.0, 0.0, -5.0), vec3_new(0.0, 0.0, 1.0),
        120.0, 800, 600);
    ray = camera_ray(&cam, 0, 300);
    {
        t_camera    cam2;
        t_ray    ray2;

        camera_init(&cam2, vec3_new(0.0, 0.0, -5.0), vec3_new(0.0, 0.0, 1.0),
            30.0, 800, 600);
        ray2 = camera_ray(&cam2, 0, 300);
        /* angle horizontal plus grand pour fov 120 */
        TEST_ASSERT(fabs(atan2(ray.dir.x, ray.dir.z))
            > fabs(atan2(ray2.dir.x, ray2.dir.z)));
    }

    TEST_END();
}
