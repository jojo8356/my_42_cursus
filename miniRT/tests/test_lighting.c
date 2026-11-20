/* test_lighting.c — suite de tests d'éclairage (motifs, ombres, Phong). */
# include "test_framework.h"
# include "minrt.h"
# include <math.h>

static t_scene    make_scene(t_color amb, double ratio, t_vec3 light_dir)
{
    t_scene    scene;

    ft_bzero(&scene, sizeof(scene));
    scene.ambient = amb;
    scene.ambient_ratio = ratio;
    scene.light.color = color_new(1.0, 1.0, 1.0);
    scene.light.ratio = 1.0;
    scene.light.direction = vec3_normalize(light_dir);
    return (scene);
}

int    main(void)
{
    t_scene    scene;
    t_obj        *sp;
    t_ray    ray;
    t_color    c;
    t_color    base;
    t_material    mat;
    t_vec3    pt;
    t_vec3    p1;
    t_vec3    p2;
    t_vec3    ld;

    TEST_BEGIN();
    TEST_SUITE("lighting");

    /* ---- apply_material : aucun motif -> base ---- */
    ft_bzero(&mat, sizeof(mat));
    mat.pattern = PAT_NONE;
    mat.specular = DEFAULT_SPECULAR;
    mat.shininess = DEFAULT_SHININESS;
    mat.pattern_c1 = color_new(0.0, 0.0, 0.0);
    mat.pattern_c2 = color_new(1.0, 1.0, 1.0);
    base = color_new(0.5, 0.5, 0.5);
    pt = vec3_new(0.0, 0.0, 0.0);
    c = apply_material(&base, &mat, &pt);
    TEST_ASSERT_NEAR(c.r, 0.5, 1e-9);
    TEST_ASSERT_NEAR(c.g, 0.5, 1e-9);
    TEST_ASSERT_NEAR(c.b, 0.5, 1e-9);

    /* ---- motif checker ---- */
    mat.pattern = PAT_CHECKER;
    mat.pattern_a = 1.0;
    mat.pattern_c1 = color_new(0.0, 0.0, 0.0);
    mat.pattern_c2 = color_new(1.0, 1.0, 1.0);
    base = color_new(1.0, 1.0, 1.0);
    pt = vec3_new(0.0, 0.0, 0.0);
    c = apply_material(&base, &mat, &pt);
    TEST_ASSERT_NEAR(c.r, 0.0, 1e-9);
    pt = vec3_new(1.0, 0.0, 0.0);
    c = apply_material(&base, &mat, &pt);
    TEST_ASSERT_NEAR(c.r, 1.0, 1e-9);

    /* ---- motif stripe ---- */
    mat.pattern = PAT_STRIPE;
    mat.pattern_a = 1.0;
    mat.pattern_c1 = color_new(0.0, 0.0, 0.0);
    mat.pattern_c2 = color_new(1.0, 0.0, 0.0);
    base = color_new(1.0, 1.0, 1.0);
    pt = vec3_new(0.0, 0.0, 0.0);
    c = apply_material(&base, &mat, &pt);
    TEST_ASSERT_NEAR(c.r, 0.0, 1e-9);
    pt = vec3_new(1.5, 0.0, 0.0);
    c = apply_material(&base, &mat, &pt);
    TEST_ASSERT_NEAR(c.r, 1.0, 1e-9);

    /* ---- trace_ray : pas d'objet -> ambiant ---- */
    scene = make_scene(color_new(1.0, 1.0, 1.0), 0.2, vec3_new(0.0, 0.0, 1.0));
    ray.origin = vec3_new(0.0, 0.0, -5.0);
    ray.dir = vec3_normalize(vec3_new(0.0, 0.0, 1.0));
    c = trace_ray(&scene, &ray, 0);
    TEST_ASSERT_NEAR(c.r, 0.2, 1e-9);
    TEST_ASSERT_NEAR(c.g, 0.2, 1e-9);
    TEST_ASSERT_NEAR(c.b, 0.2, 1e-9);

    /* ---- trace_ray : sphère rouge éclairée de face ---- */
    /* lumière venant de -Z (direction (0,0,1)) : le avant de la sphère (normale
       (0,0,-1)) est éclairé, diff = 1. */
    scene = make_scene(color_new(1.0, 1.0, 1.0), 0.2, vec3_new(0.0, 0.0, 1.0));
    sp = obj_sphere(vec3_new(0.0, 0.0, 0.0), 2.0, color_new(1.0, 0.0, 0.0));
    sp->mat.specular = 0.0;
    scene_add_object(&scene, sp);
    ray.origin = vec3_new(0.0, 0.0, -5.0);
    ray.dir = vec3_normalize(vec3_new(0.0, 0.0, 1.0));
    c = trace_ray(&scene, &ray, 0);
    /* ambient*base = (0.2,0,0) ; diffuse = base*1*1 = (1,0,0) ; total ~ (1,0,0) */
    TEST_ASSERT_NEAR(c.r, 1.0, 1e-6);
    TEST_ASSERT_NEAR(c.g, 0.0, 1e-6);
    TEST_ASSERT_NEAR(c.b, 0.0, 1e-6);

    /* ---- trace_ray : côté non éclairé -> ambiant seul ---- */
    /* lumière venant de +Z : le avant (normale (0,0,-1)) a diff = -1 -> 0 */
    scene = make_scene(color_new(1.0, 1.0, 1.0), 0.2, vec3_new(0.0, 0.0, -1.0));
    sp = obj_sphere(vec3_new(0.0, 0.0, 0.0), 2.0, color_new(1.0, 0.0, 0.0));
    scene_add_object(&scene, sp);
    ray.origin = vec3_new(0.0, 0.0, -5.0);
    ray.dir = vec3_normalize(vec3_new(0.0, 0.0, 1.0));
    c = trace_ray(&scene, &ray, 0);
    TEST_ASSERT_NEAR(c.r, 0.2, 1e-6);
    TEST_ASSERT_NEAR(c.g, 0.0, 1e-6);
    TEST_ASSERT_NEAR(c.b, 0.0, 1e-6);

    /* ---- in_shadow ---- */
    scene = make_scene(color_new(1.0, 1.0, 1.0), 0.2, vec3_new(0.0, 0.0, 1.0));
    sp = obj_sphere(vec3_new(0.0, 0.0, 2.0), 1.0, color_new(1.0, 1.0, 1.0));
    scene_add_object(&scene, sp);
    /* point entre la lumière (+Z) et la sphère : rayon ombre vers +Z touche */
    p1 = vec3_new(0.0, 0.0, 3.0);
    ld = vec3_new(0.0, 0.0, -1.0);
    TEST_ASSERT(in_shadow(&scene, &p1, &ld) == 1);
    /* point à l'extérieur : pas d'ombre */
    p2 = vec3_new(0.0, 0.0, -5.0);
    TEST_ASSERT(in_shadow(&scene, &p2, &ld) == 0);

    /* ---- phong_shade : profondeur limite ---- */
    c = trace_ray(&scene, &ray, MAX_DEPTH + 1);
    TEST_ASSERT_NEAR(c.r, 0.0, 1e-9);
    TEST_ASSERT_NEAR(c.g, 0.0, 1e-9);
    TEST_ASSERT_NEAR(c.b, 0.0, 1e-9);

    scene_free(&scene);
    TEST_END();
}
