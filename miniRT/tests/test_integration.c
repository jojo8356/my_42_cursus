/* test_integration.c — suite d'intégration (rendu complet, bonus, MLX). */
# include "test_framework.h"
# include "minrt.h"
# include <math.h>
# include <stdlib.h>

static void    write_scene(const char *path)
{
    FILE    *f;

    f = fopen(path, "w");
    fprintf(f, "A 0.2 255,255,255\n");
    fprintf(f, "C 0,0,-5 0,0,1 60\n");
    fprintf(f, "L 0,0,-10 1.0 255,255,255\n");
    fprintf(f, "sp 0,0,0 2 255,0,0\n");
    fprintf(f, "pl 0,-2,0 0,1,0 200,200,200\n");
    fclose(f);
}

int    main(void)
{
    t_scene    scene;
    t_obj        *sp;
    t_obj        *mirror;
    t_obj        *glass;
    t_color    *buf;
    t_game    game;
    t_ray    ray;
    t_color    c_mirror;
    t_color    c_plain;
    t_color    c_glass;

    TEST_BEGIN();
    TEST_SUITE("integration");

    /* ---- rendu complet d'une scène parsée ---- */
    write_scene("/tmp/minrt_integ.rt");
    ft_bzero(&scene, sizeof(scene));
    TEST_ASSERT_EQ(parse_rt("/tmp/minrt_integ.rt", &scene), 0);
    scene.camera.width = 80;
    scene.camera.height = 60;
    camera_init(&scene.camera, scene.camera.position, scene.camera.direction,
        scene.camera.fov, 80, 60);
    buf = (t_color *)xcalloc(80 * 60, sizeof(t_color));
    render_region(&scene, &scene.camera, 0, 60, 0, buf, 80);
    /* centre : sphère rouge */
    TEST_ASSERT(buf[30 * 80 + 40].r > 0.5);
    TEST_ASSERT(buf[30 * 80 + 40].g < 0.3);
    /* bas : plan gris */
    TEST_ASSERT(buf[55 * 80 + 40].r > 0.1);
    TEST_ASSERT(buf[55 * 80 + 40].g > 0.1);
    TEST_ASSERT(buf[55 * 80 + 40].b > 0.1);
    /* haut : ciel */
    TEST_ASSERT(buf[0 * 80 + 40].b > buf[0 * 80 + 40].r);
    scene_free(&scene);
    free(buf);

    /* ---- réflexion : miroir reflète le ciel (ambiant bleu) ---- */
    ft_bzero(&scene, sizeof(scene));
    scene.ambient = color_new(0.0, 0.0, 1.0);
    scene.ambient_ratio = 1.0;
    scene.light.color = color_new(0.0, 0.0, 0.0);
    scene.light.ratio = 0.0;
    scene.light.direction = vec3_normalize(vec3_new(0.0, 0.0, 1.0));
    camera_init(&scene.camera, vec3_new(0.0, 0.0, -5.0),
        vec3_new(0.0, 0.0, 1.0), 60.0, 80, 60);

    /* objet non-réfléchissant rouge : ambiant*base = (0,0,0) */
    sp = obj_sphere(vec3_new(0.0, 0.0, 0.0), 2.0, color_new(1.0, 0.0, 0.0));
    scene_add_object(&scene, sp);
    ray.origin = vec3_new(0.0, 0.0, -5.0);
    ray.dir = vec3_normalize(vec3_new(0.0, 0.0, 1.0));
    c_plain = trace_ray(&scene, &ray, 0);
    TEST_ASSERT_NEAR(c_plain.b, 0.0, 1e-6);

    /* miroir blanc (re:1.0) : reflète le ciel bleu */
    scene_free(&scene);
    mirror = obj_sphere(vec3_new(0.0, 0.0, 0.0), 2.0, color_new(1.0, 1.0, 1.0));
    mirror->mat.reflection = 1.0;
    scene_add_object(&scene, mirror);
    c_mirror = trace_ray(&scene, &ray, 0);
    TEST_ASSERT(c_mirror.b > c_plain.b);
    scene_free(&scene);

    /* ---- transparence / réfraction : verre laisse passer la lumière ---- */
    ft_bzero(&scene, sizeof(scene));
    scene.ambient = color_new(1.0, 1.0, 1.0);
    scene.ambient_ratio = 0.2;
    scene.light.color = color_new(1.0, 1.0, 1.0);
    scene.light.ratio = 1.0;
    scene.light.direction = vec3_normalize(vec3_new(0.0, 0.0, 1.0));
    camera_init(&scene.camera, vec3_new(0.0, 0.0, -5.0),
        vec3_new(0.0, 0.0, 1.0), 60.0, 80, 60);
    glass = obj_sphere(vec3_new(0.0, 0.0, 0.0), 2.0, color_new(1.0, 1.0, 1.0));
    glass->mat.transparency = 1.0;
    glass->mat.refr_index = 1.5;
    scene_add_object(&scene, glass);
    /* derrière la sphère de verre : un plan rouge */
    sp = obj_sphere(vec3_new(0.0, 0.0, 5.0), 2.0, color_new(1.0, 0.0, 0.0));
    scene_add_object(&scene, sp);
    c_glass = trace_ray(&scene, &ray, 0);
    /* le rayon traverse le verre et voit la sphère rouge (rouge dominant) */
    TEST_ASSERT(c_glass.r > c_glass.g);
    TEST_ASSERT(c_glass.r > c_glass.b);
    scene_free(&scene);

    /* ---- game_init / game_destroy via le shim MLX ---- */
    write_scene("/tmp/minrt_game.rt");
    ft_bzero(&game, sizeof(game));
    TEST_ASSERT_EQ(game_init(&game, 1, 80, 60, 1, 0, "/tmp/minrt_game.rt"), 0);
    TEST_ASSERT_EQ(game.scene.obj_count, 2);
    TEST_ASSERT(game.mlx.img_data != NULL);
    render_frame(&game);
    game_destroy(&game);

    TEST_END();
}
