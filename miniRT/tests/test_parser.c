/* test_parser.c — suite de tests du lecteur .rt. */
# include "test_framework.h"
# include "minrt.h"
# include <stdlib.h>
# include <math.h>

static void    write_scene(const char *path)
{
    FILE    *f;

    f = fopen(path, "w");
    if (!f)
        return;
    fprintf(f, "# scene\n");
    fprintf(f, "A 0.3 255,255,255\n");
    fprintf(f, "C 0,0,-5 0,0,1 90\n");
    fprintf(f, "L 10,10,10 1.0 255,255,255\n");
    fprintf(f, "sp 0,0,0 2 255,0,0 re:0.5 tr:0.0 ri:1.0\n");
    fprintf(f, "pl 0,-1,0 0,1,0 100,100,100\n");
    fprintf(f, "cy 0,0,0 0,1,0 2 4 0,255,0\n");
    fprintf(f, "co 3,0,0 0,1,0 2 4 0,0,255\n");
    fprintf(f, "cu -3,0,0 0,1,0 2 255,255,0\n");
    fclose(f);
}

int    main(void)
{
    t_vec3    v;
    t_color    c;
    double    d;
    t_scene    scene;

    TEST_BEGIN();
    TEST_SUITE("parser");

    /* ---- helpers ---- */
    TEST_ASSERT_EQ(parse_double("3.5", &d), 0);
    TEST_ASSERT_NEAR(d, 3.5, 1e-9);
    TEST_ASSERT_EQ(parse_double("abc", &d), -1);

    TEST_ASSERT_EQ(parse_vec3("1,2,3", &v), 0);
    TEST_ASSERT_NEAR(v.x, 1.0, 1e-9);
    TEST_ASSERT_NEAR(v.y, 2.0, 1e-9);
    TEST_ASSERT_NEAR(v.z, 3.0, 1e-9);
    TEST_ASSERT_EQ(parse_vec3("1,2", &v), -1);

    TEST_ASSERT_EQ(parse_color("255,0,0", &c), 0);
    TEST_ASSERT_NEAR(c.r, 1.0, 1e-9);
    TEST_ASSERT_NEAR(c.g, 0.0, 1e-9);
    TEST_ASSERT_NEAR(c.b, 0.0, 1e-9);
    TEST_ASSERT_EQ(parse_color("0.5,0.5,0.5", &c), 0);
    TEST_ASSERT_NEAR(c.r, 0.5, 1e-9);

    /* ---- parsing complet ---- */
    write_scene("/tmp/minrt_test_scene.rt");
    ft_bzero(&scene, sizeof(scene));
    TEST_ASSERT_EQ(parse_rt("/tmp/minrt_test_scene.rt", &scene), 0);

    TEST_ASSERT_NEAR(scene.ambient_ratio, 0.3, 1e-9);
    TEST_ASSERT_NEAR(scene.ambient.r, 1.0, 1e-9);
    TEST_ASSERT_NEAR(scene.ambient.g, 1.0, 1e-9);
    TEST_ASSERT_NEAR(scene.ambient.b, 1.0, 1e-9);

    TEST_ASSERT_NEAR(scene.camera.position.x, 0.0, 1e-9);
    TEST_ASSERT_NEAR(scene.camera.position.z, -5.0, 1e-9);
    TEST_ASSERT_NEAR(scene.camera.fov, 90.0, 1e-9);

    TEST_ASSERT_NEAR(scene.light.ratio, 1.0, 1e-9);
    TEST_ASSERT_NEAR(scene.light.position.x, 10.0, 1e-9);
    /* direction normalisée vers l'origine */
    TEST_ASSERT_NEAR(vec3_length(scene.light.direction), 1.0, 1e-9);

    TEST_ASSERT_EQ(scene.obj_count, 5);

    /* sphère avec tags bonus */
    TEST_ASSERT_EQ(scene.objects[0].type, OBJ_SPHERE);
    TEST_ASSERT_NEAR(scene.objects[0].sphere_radius, 1.0, 1e-9);
    TEST_ASSERT_NEAR(scene.objects[0].mat.reflection, 0.5, 1e-9);
    TEST_ASSERT_NEAR(scene.objects[0].mat.refr_index, 1.0, 1e-9);

    TEST_ASSERT_EQ(scene.objects[1].type, OBJ_PLANE);
    TEST_ASSERT_EQ(scene.objects[2].type, OBJ_CYLINDER);
    TEST_ASSERT_EQ(scene.objects[3].type, OBJ_CONE);
    TEST_ASSERT_EQ(scene.objects[4].type, OBJ_CUBE);

    /* validation */
    TEST_ASSERT_EQ(scene_validate(&scene), 0);
    scene_free(&scene);

    /* ---- fichier invalide ---- */
    {
        FILE    *f;

        f = fopen("/tmp/minrt_bad.rt", "w");
        fprintf(f, "A 0.3 255,255,255\n");
        fprintf(f, "sp 0,0,0 notanumber 255,0,0\n");
        fclose(f);
        ft_bzero(&scene, sizeof(scene));
        TEST_ASSERT_EQ(parse_rt("/tmp/minrt_bad.rt", &scene), -1);
    }

    /* ---- fichier absent ---- */
    TEST_ASSERT_EQ(parse_rt("/tmp/minrt_nope.rt", &scene), -1);

    TEST_END();
}
