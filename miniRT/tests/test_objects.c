/* test_objects.c — suite de tests des constructeurs d'objets. */
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
    t_vec3    p;

    TEST_BEGIN();
    TEST_SUITE("objects");

    /* sphère : rayon = diametre / 2, géométrie locale à l'origine */
    sp = obj_sphere(vec3_new(1.0, 2.0, 3.0), 2.0, color_new(1.0, 0.0, 0.0));
    TEST_ASSERT_EQ(sp->type, OBJ_SPHERE);
    TEST_ASSERT_NEAR(sp->sphere_radius, 1.0, 1e-9);
    TEST_ASSERT_NEAR(sp->sphere_center.x, 0.0, 1e-9);
    TEST_ASSERT_NEAR(sp->sphere_center.y, 0.0, 1e-9);
    TEST_ASSERT_NEAR(sp->sphere_center.z, 0.0, 1e-9);
    /* la sphère est translatée à (1,2,3) : le point local (0,0,0) devient (1,2,3) */
    p = mat4_transform_point(sp->transform, vec3_new(0.0, 0.0, 0.0));
    TEST_ASSERT_NEAR(p.x, 1.0, 1e-9);
    TEST_ASSERT_NEAR(p.y, 2.0, 1e-9);
    TEST_ASSERT_NEAR(p.z, 3.0, 1e-9);
    /* les propriétés matérielles par défaut */
    TEST_ASSERT_NEAR(sp->mat.specular, DEFAULT_SPECULAR, 1e-9);
    TEST_ASSERT_NEAR(sp->mat.reflection, DEFAULT_REFLECTION, 1e-9);
    TEST_ASSERT_NEAR(sp->mat.transparency, DEFAULT_TRANSP, 1e-9);
    TEST_ASSERT_NEAR(sp->mat.refr_index, DEFAULT_RI, 1e-9);
    free(sp);

    /* plan : normale alignée sur (0,1,0) en local, point à la position */
    pl = obj_plane(vec3_new(0.0, 1.0, 0.0), vec3_new(0.0, 1.0, 0.0),
        color_new(0.5, 0.5, 0.5));
    TEST_ASSERT_EQ(pl->type, OBJ_PLANE);
    TEST_ASSERT_NEAR(pl->plane_normal.x, 0.0, 1e-9);
    TEST_ASSERT_NEAR(pl->plane_normal.y, 1.0, 1e-9);
    TEST_ASSERT_NEAR(pl->plane_normal.z, 0.0, 1e-9);
    p = mat4_transform_point(pl->transform, vec3_new(0.0, 0.0, 0.0));
    TEST_ASSERT_NEAR(p.x, 0.0, 1e-9);
    TEST_ASSERT_NEAR(p.y, 1.0, 1e-9);
    TEST_ASSERT_NEAR(p.z, 0.0, 1e-9);
    free(pl);

    /* plan incliné : normale (0,0,1) */
    pl = obj_plane(vec3_new(0.0, 0.0, 0.0), vec3_new(0.0, 0.0, 1.0),
        color_new(1.0, 1.0, 1.0));
    /* en local la normale est (0,1,0) ; après rotation elle doit être (0,0,1) */
    p = mat4_transform_dir(pl->transform, vec3_new(0.0, 1.0, 0.0));
    TEST_ASSERT_NEAR(p.x, 0.0, 1e-9);
    TEST_ASSERT_NEAR(p.y, 0.0, 1e-9);
    TEST_ASSERT_NEAR(p.z, 1.0, 1e-9);
    free(pl);

    /* cylindre : rayon = diametre/2, hauteur, axe (0,1,0) en local */
    cy = obj_cylinder(vec3_new(0.0, 0.0, 0.0), vec3_new(0.0, 1.0, 0.0),
        2.0, 4.0, color_new(0.0, 1.0, 0.0));
    TEST_ASSERT_EQ(cy->type, OBJ_CYLINDER);
    TEST_ASSERT_NEAR(cy->cyl_radius, 1.0, 1e-9);
    TEST_ASSERT_NEAR(cy->cyl_height, 4.0, 1e-9);
    TEST_ASSERT_NEAR(cy->cyl_axis.y, 1.0, 1e-9);
    free(cy);

    /* cylindre incliné : axe (1,0,0) */
    cy = obj_cylinder(vec3_new(0.0, 0.0, 0.0), vec3_new(1.0, 0.0, 0.0),
        1.0, 2.0, color_new(0.0, 1.0, 0.0));
    p = mat4_transform_dir(cy->transform, vec3_new(0.0, 1.0, 0.0));
    TEST_ASSERT_NEAR(p.x, 1.0, 1e-9);
    TEST_ASSERT_NEAR(p.y, 0.0, 1e-9);
    TEST_ASSERT_NEAR(p.z, 0.0, 1e-9);
    free(cy);

    /* cône : half_angle en radians */
    co = obj_cone(vec3_new(0.0, 0.0, 0.0), vec3_new(0.0, 1.0, 0.0),
        30.0, 5.0, color_new(0.0, 0.0, 1.0));
    TEST_ASSERT_EQ(co->type, OBJ_CONE);
    TEST_ASSERT_NEAR(co->cone_half_angle, 30.0 * M_PI / 180.0, 1e-9);
    TEST_ASSERT_NEAR(co->cone_height, 5.0, 1e-9);
    free(co);

    /* cube : taille, centré à l'origine */
    cu = obj_cube(2.0, color_new(1.0, 1.0, 0.0));
    TEST_ASSERT_EQ(cu->type, OBJ_CUBE);
    TEST_ASSERT_NEAR(cu->cube_size, 2.0, 1e-9);
    free(cu);

    /* cube positionné + orienté */
    cu = obj_cube_at(vec3_new(1.0, 1.0, 1.0), vec3_new(0.0, 1.0, 0.0), 2.0,
        color_new(1.0, 1.0, 0.0));
    p = mat4_transform_point(cu->transform, vec3_new(0.0, 0.0, 0.0));
    TEST_ASSERT_NEAR(p.x, 1.0, 1e-9);
    TEST_ASSERT_NEAR(p.y, 1.0, 1e-9);
    TEST_ASSERT_NEAR(p.z, 1.0, 1e-9);
    free(cu);

    TEST_END();
}
