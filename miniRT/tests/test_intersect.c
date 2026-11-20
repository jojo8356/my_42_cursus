/* test_intersect.c — suite de tests des intersections rayon-objet. */
# include "test_framework.h"
# include "minrt.h"
# include <math.h>

int    main(void)
{
    t_obj    *sp;
    t_obj    *pl;
    t_obj    *cy;
    t_obj    *co;
    t_obj    *cu;
    t_obj    *tr;
    t_hit    hit;
    t_ray    ray;

    TEST_BEGIN();
    TEST_SUITE("intersect");

    /* ---- sphère ---- */
    sp = obj_sphere(vec3_new(0.0, 0.0, 0.0), 2.0, color_new(1.0, 0.0, 0.0));
    /* rayon vers le centre depuis (0,0,-5) */
    ray.origin = vec3_new(0.0, 0.0, -5.0);
    ray.dir = vec3_normalize(vec3_new(0.0, 0.0, 1.0));
    hit_reset(&hit);
    TEST_ASSERT(intersect_sphere(sp, &ray, &hit) == 1);
    TEST_ASSERT_NEAR(hit.t, 4.0, 1e-6);
    TEST_ASSERT_NEAR(hit.point.z, -1.0, 1e-6);
    /* normale pointe vers le rayon (sortant) */
    TEST_ASSERT_NEAR(hit.normal.z, -1.0, 1e-6);
    /* rayon qui rate */
    ray.origin = vec3_new(10.0, 10.0, -5.0);
    hit_reset(&hit);
    TEST_ASSERT(intersect_sphere(sp, &ray, &hit) == 0);
    free(sp);

    /* ---- plan ---- */
    pl = obj_plane(vec3_new(0.0, 0.0, 0.0), vec3_new(0.0, 1.0, 0.0),
        color_new(1.0, 1.0, 1.0));
    ray.origin = vec3_new(0.0, 5.0, 0.0);
    ray.dir = vec3_normalize(vec3_new(0.0, -1.0, 0.0));
    hit_reset(&hit);
    TEST_ASSERT(intersect_plane(pl, &ray, &hit) == 1);
    TEST_ASSERT_NEAR(hit.t, 5.0, 1e-6);
    TEST_ASSERT_NEAR(hit.point.y, 0.0, 1e-6);
    TEST_ASSERT_NEAR(hit.normal.y, 1.0, 1e-6);
    /* rayon parallèle au plan */
    ray.origin = vec3_new(0.0, 5.0, 0.0);
    ray.dir = vec3_normalize(vec3_new(1.0, 0.0, 0.0));
    hit_reset(&hit);
    TEST_ASSERT(intersect_plane(pl, &ray, &hit) == 0);
    free(pl);

    /* ---- cylindre (vertical, rayon 1, hauteur 4 centree) ---- */
    cy = obj_cylinder(vec3_new(0.0, 0.0, 0.0), vec3_new(0.0, 1.0, 0.0),
        2.0, 4.0, color_new(0.0, 1.0, 0.0));
    /* rayon horizontal vers le cylindre */
    ray.origin = vec3_new(-5.0, 0.0, 0.0);
    ray.dir = vec3_normalize(vec3_new(1.0, 0.0, 0.0));
    hit_reset(&hit);
    TEST_ASSERT(intersect_cylinder(cy, &ray, &hit) == 1);
    TEST_ASSERT_NEAR(hit.t, 4.0, 1e-6);
    /* rayon qui passe loin */
    ray.origin = vec3_new(-5.0, 0.0, 10.0);
    hit_reset(&hit);
    TEST_ASSERT(intersect_cylinder(cy, &ray, &hit) == 0);
    free(cy);

    /* ---- cône (apex à l'origine, ouvre vers +Y, hauteur 5, half 30 deg) ---- */
    co = obj_cone(vec3_new(0.0, 0.0, 0.0), vec3_new(0.0, 1.0, 0.0),
        30.0, 5.0, color_new(0.0, 0.0, 1.0));
    /* rayon vers l'apex depuis le bas */
    ray.origin = vec3_new(0.0, -5.0, 0.0);
    ray.dir = vec3_normalize(vec3_new(0.0, 1.0, 0.0));
    hit_reset(&hit);
    TEST_ASSERT(intersect_cone(co, &ray, &hit) == 1);
    free(co);

    /* ---- cube (côté 2, centré à l'origine) ---- */
    cu = obj_cube(2.0, color_new(1.0, 1.0, 0.0));
    ray.origin = vec3_new(-5.0, 0.0, 0.0);
    ray.dir = vec3_normalize(vec3_new(1.0, 0.0, 0.0));
    hit_reset(&hit);
    TEST_ASSERT(intersect_cube(cu, &ray, &hit) == 1);
    TEST_ASSERT_NEAR(hit.t, 4.0, 1e-6);
    TEST_ASSERT_NEAR(hit.point.x, -1.0, 1e-6);
    /* rayon qui rate */
    ray.origin = vec3_new(-5.0, 5.0, 0.0);
    ray.dir = vec3_normalize(vec3_new(1.0, 0.0, 0.0));
    hit_reset(&hit);
    TEST_ASSERT(intersect_cube(cu, &ray, &hit) == 0);
    free(cu);

    /* ---- triangle ---- */
    tr = obj_triangle(vec3_new(-1.0, 0.0, 0.0), vec3_new(1.0, 0.0, 0.0),
        vec3_new(0.0, 2.0, 0.0), color_new(1.0, 0.0, 1.0));
    ray.origin = vec3_new(0.0, 1.0, -5.0);
    ray.dir = vec3_normalize(vec3_new(0.0, 0.0, 1.0));
    hit_reset(&hit);
    TEST_ASSERT(intersect_triangle(tr, &ray, &hit) == 1);
    /* rayon qui rate (décalé en x) */
    ray.origin = vec3_new(5.0, 1.0, -5.0);
    ray.dir = vec3_normalize(vec3_new(0.0, 0.0, 1.0));
    hit_reset(&hit);
    TEST_ASSERT(intersect_triangle(tr, &ray, &hit) == 0);
    free(tr);

    /* ---- obj_intersect dispatch + scene_intersect ---- */
    sp = obj_sphere(vec3_new(0.0, 0.0, 3.0), 1.0, color_new(1.0, 0.0, 0.0));
    pl = obj_plane(vec3_new(0.0, -2.0, 0.0), vec3_new(0.0, 1.0, 0.0),
        color_new(0.0, 1.0, 0.0));
    {
        t_scene    scene;

        ft_bzero(&scene, sizeof(scene));
        scene_add_object(&scene, sp);
        scene_add_object(&scene, pl);
        scene.light.direction = vec3_normalize(vec3_new(-1.0, -1.0, 0.0));

        ray.origin = vec3_new(0.0, 0.0, -5.0);
        ray.dir = vec3_normalize(vec3_new(0.0, 0.0, 1.0));
        hit_reset(&hit);
        TEST_ASSERT(obj_intersect(&scene.objects[0], &ray, &hit) == 1);
        hit_reset(&hit);
        TEST_ASSERT(scene_intersect(&scene, &ray, &hit) == 1);
        TEST_ASSERT(hit.obj == &scene.objects[0]);
        scene_free(&scene);
    }

    TEST_END();
}
